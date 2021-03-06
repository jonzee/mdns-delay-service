#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "linkedlist.h"
#include "opoznienia.h"
#include "err.h"

int stack_len() {
    if (pthread_rwlock_rdlock(&lock) != 0) syserr("pthread_rwlock_rdlock error");

    Node *p = head;
    int len = 0;     
    while(p) {
        ++len;
        p = p->next;
    }

    if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
    return len;
}

void init_host(struct host_data * h) {
	(*h).u = (*h).t = (*h).i = 0;
	memset((*h).udp, 0, sizeof((*h).udp));
	memset((*h).tcp, 0, sizeof((*h).tcp));
	memset((*h).icm, 0, sizeof((*h).icm));
    (*h).tcp_time = 0;
    (*h).tcp_numb = 0;
    (*h).icm_time = 0;
    (*h).icm_seq = 0;
    (*h).is_tcp = 0;
    (*h).is_udp = 0;
    (*h).is_icm = 0;
} 

void stack_check() {
  if (pthread_rwlock_wrlock(&lock) != 0) syserr("pthread_rwlock_rdlock error");

  Node *tmp;
  Node *p = head;
  while(p && p->next) {
      if (!p->next->host.check) {
        tmp = p->next;
        p->next = tmp->next;
        free(tmp);
      } else
        p = p->next;
  }
  if (head && !head->host.check) {
    tmp = head;
    head = head->next;
    free(tmp);
  }

  p = head;
  while(p) {
    p->host.check = 0;
    p = p->next;
  }  
  if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");

}

void create_or_add(uint32_t ip, char *type) {
  struct sockaddr_in a;
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = ip;

  // printf("create_or_add: %lu\n", ip);
  if (pthread_rwlock_wrlock(&lock) != 0) syserr("pthread_rwlock_rdlock error");

  Node *p = head;
  while(p) {
    if(ip != p->host.ip)
      p = p->next;
    else
      break;
  }

  Node *edit;
  if (p == NULL) {
    edit = malloc(sizeof(Node));
    init_host(&(edit->host));
    edit->host.ip = ip;
    edit->next = head;
    head = edit;
    // printf("ADD NEW: %s, IPv4 address : %s\n", type, inet_ntoa(a.sin_addr));
  } else
    edit = p;

  // add icmp host:
  if (!edit->host.is_icm) {
    a.sin_port = htons(0);
    edit->host.is_icm = 1;
    edit->host.addr_icm = *(struct sockaddr *) &a;
    // printf("add icm: ");
    // print_ip_port(*(struct sockaddr *) &a);
  }
  if (!strcmp("udp", type) && !edit->host.is_udp) {
    a.sin_port = htons(udp_port);
    edit->host.addr_udp = *(struct sockaddr *) &a;
    edit->host.is_udp = 1;
    // printf("add udp: ");
    // print_ip_port(*(struct sockaddr *) &a);
  } else if (!strcmp("tcp", type) && !edit->host.is_tcp) {
    a.sin_port = htons(tcp_port);
    edit->host.addr_tcp = *(struct sockaddr *) &a;
    edit->host.is_tcp = 1;
    // printf("add tcp: ");
    // print_ip_port(*(struct sockaddr *) &a);
  }
  edit->host.check = 1;

  if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
}

void stack_push(struct sockaddr addr) {
  Node *new = malloc(sizeof(Node));
//  init_host(&(new->host), addr);
	if (pthread_rwlock_wrlock(&lock) != 0) syserr("pthread_rwlock_wrlock error");
  new -> next = head;
  head = new;
  if (pthread_rwlock_unlock(&lock) != 0) syserr("pthread_rwlock_unlock error");
}
 
stack_data stack_pop(Node **node_head) {
    Node *node_togo = *node_head;
    stack_data d;
     
    if(node_head) {
        d = node_togo -> host;
        *node_head = node_togo -> next;
        free(node_togo);
    }
    return d;
}

void stack_print() {
    if (pthread_rwlock_rdlock(&lock) != 0) syserr("pthread_rwlock_rdlock error");

    Node *p = head;
    if(!p) printf("the stack is empty");
    else {
        while(p) {
            (void) printf("%s", inet_ntoa(((struct sockaddr_in *) &p->host.addr_udp)->sin_addr));
			(void) printf(":");
			(void) printf("%d -> ", ntohs(((struct sockaddr_in *) &p->host.addr_udp)->sin_port));
            p = p -> next;
        }
    }
    printf("\n");

    if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
}
 
void stack_clear(Node **node_head) {
    while(*node_head)
        stack_pop(node_head);
}
int stack_elem(struct sockaddr *sa) {
    if (pthread_rwlock_rdlock(&lock) != 0) syserr("pthread_rwlock_rdlock error");
 
    Node *p = head;
     
    while(p) {
        if(strcmp(sa->sa_data, p->host.addr_udp.sa_data)) //set for numbers, modifiable
            p = p->next;
        else {
            return 1;
        }
    }
    return 0;

   if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
}

void add_udp_measurement(struct sockaddr *sa, uint64_t result) {

	if (pthread_rwlock_wrlock(&lock) != 0) syserr("pthread_rwlock_wrlock error");

    Node *p = head;    
    while(p) {
        if(strcmp(sa->sa_data, p->host.addr_udp.sa_data)) p = p->next;
        else {
  				// printf("udp pomiar: %d, wynik: %lu\n", p->host.u, result); 
  				p->host.udp[ p->host.u ] = result;
  				p->host.u = (p->host.u+1)%10;						
    			break;
        }
    }
    
	if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
}

void add_icm_measurement(struct sockaddr *sa, uint64_t end) {

  if (pthread_rwlock_wrlock(&lock) != 0) syserr("pthread_rwlock_wrlock error");

    Node *p = head;    
    while(p) {
        if(strcmp(sa->sa_data, p->host.addr_icm.sa_data)) p = p->next;
        else {
          p->host.icm[ p->host.i ] = end - p->host.icm_time;
          // printf("icm pomiar: %d, wynik %lu\n", p->host.i, p->host.icm[ p->host.i ]);
          p->host.i = (p->host.i+1)%10;
          break;
        }
    }
    
  if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
}

void add_tcp_measurement(int numb, uint64_t end) {

  if (pthread_rwlock_wrlock(&lock) != 0) syserr("pthread_rwlock_wrlock error");

  Node *p = head;
  while(p) {
    if (p->host.tcp_numb != numb) p = p->next;
    else {
      if (end > 0) {
        p->host.tcp[ p->host.t ] = end - p->host.tcp_time;
        // printf("tcp pomiar: %d, wynik %lu\n", p->host.t, p->host.tcp[ p->host.t ]);
        p->host.t = (p->host.t + 1) % 10;
      }
      p->host.tcp_time = 0;
      break;
    }
  }
  
  if (pthread_rwlock_unlock(&lock) != 0) syserr("pthred_rwlock_unlock error");
}
