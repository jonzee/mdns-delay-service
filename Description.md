Zadanie zaliczeniowe -- rozproszony pomiar opóźnień
===================================================

Zadanie polega na napisaniu programu `opoznienia`, służącego do pomiaru
opóźnień między komputerami. Program powinien samodzielnie odnajdywać inne
komputery dające możliwość pomiaru i w dających się ustawić (opcja -t,
domyślnie 1 sekunda) odstępach czasu mierzyć opóźnienia do wszystkich tych
komputerów.

Program też ma umożliwiać zgodne z podanym standardem (mDNS) wykrywanie
innych serwerów umożliwiających pomiar opóźnień na komputerach w sieci
lokalnej.

Dodatkowo program powinien udostępniać interfejs użytkownika dostępny
po połączeniu programem telnet.

Podsumowując, program `opoznienia` pełni role:
 * serwera pomiaru opóźnień metodą UDP
 * klienta pomiaru opóźnień trzema metodami
 * serwera mDNS
 * klienta mDNS
 * serwera interfejsu użytkownika


Metody pomiaru
--------------

Należy zaimplementować wszystkie metody pomiaru z zadania 1, a ponadto
zaimplementować pomiar na podstawie czasu odpowiedzi na pakiety ICMP Echo Request.
W pakietach ICMP należy wpisać w polu identyfikator wartość 0x13. W części data
należy w pierwszych 24 bitach umieścić swój numer indeksu (zakodowany w BCD,
w kolejności bajtów bigendian), a w ostatnich 8 - numer grupy. Należy zadbać
o sensowną obsługę numerów sekwencyjnych. Program `opoznienia` nie powinien
odpowiadać na zapytania ICMP Echo Request. Zakładamy, że to potrafi system
operacyjny.

Program `opoznienia` powinien pełnić rolę serwera pomiaru czasu UDP, zgodnie
z protokołem z pierwszego zadania zaliczeniowego. Domyślnie wykorzystywany
powinien być port 3382, co jednak powinno dać się zmienić opcją -u.


Odkrywanie sieci
----------------

W celu automatycznego odnajdywania innych komputerów w sieci będziemy
korzystali z protokołów Multicast DNS (RFC 6762) i DNS-SD (RFC 6763).

Te protokoły nie były przedstawiane na wykładach ani laboratoriach i należy
zapoznać się z nimi samemu. Warto dobrać się w zespoły, żeby wspólnie szybciej
udało się zrozumieć ścisły język standardów i wydobyć z niego to, co jest
potrzebne do zaimplementowania niezbędnej funkcjonalności.  Implementacje
oczywiście muszą być samodzielne.

Dla uproszczenia wystarczy obsługiwać tylko część tych protokołów:
 * Multicast DNS w takim zakresie, żeby móc skutecznie odpowiadać na
   pojedyncze pytania o wpisy typu PTR _USLUGA._PROTOKOL.local i wpisy typu A
   NAZWA._USLUGA._PROTOKOL.local
 * aby dobrze wybierać nazwy DNS-SD: RFC 6763 Appendix D
 * nie implementujemy rozdziału 7 RFC 6762, dotyczącego optymalizacji mających
   na celu zmniejszenie ruchu w sieci. Zamiast tego wysyłamy zapytania
   o odpowiednie usługi co 10 sekund (z możliwością zmiany opcją -T)
   i aktualizujemy odpowiednio listę komputerów.
 * w szczególności nie implementujemy rozdziałów 8, 9, 10, 11 RFC 6762 itp.

Program `opoznienia` powinien być serwerem i odpowiadać na pytania o usługę
_opoznienia._udp, i jednocześnie być klientem pytającym o odpowiednią usługę.
Zakładamy, że komputery udostępniające pomiar za pomocą protokołu UDP chcemy
też wykorzystać do pomiarów za pomocą pakietów ICMP.

Do pomiarów czasu połączenia przy wykorzystaniu protokołu TCP będziemy
wykorzystywać wszystkie komputery, które oferują usługę SSH (_ssh._tcp), łącząc
się do tej usługi (port 22). Należy umożliwić włączenie w programie
`opoznienia` trybu, w którym usługa _ssh._tcp jest ogłaszana przez DNS-SD
(opcja -s).


UI
--

Interfejs programu powinien być dostępny na porcie 3637 (do zmiany opcją -U)
i powinno dać się do niego podłączyć programem telnet. Rysujemy następującą
tabelkę, zakładając, że mamy do dyspozycji 80 znaków szerokości:

IP1               15 15 15
IP2          10 10 10
IP3     5 5 5

przy czym w poszczególnych kolumnach znajdują się: adres IP komputera,
opóźnienie zmierzone przez UDP, opóźnienie zmierzone przez TCP, opóźnienie
zmierzone przez ICMP. Jako opóźnienie pokazujemy średnią z ostatnich 10
pomiarów. Liczba spacji między kolumną 1 a 2 jest proporcjonalna do średniego
opóźnienia dla danego komputera (średnia z 3 metod). Wiersze powinny być
posortowane malejąco po opóźnieniu.

Należy założyć, że terminal odbiorcy ma rozmiar 80x24. Na terminalu powinien
być pokazywany fragment tabelki, przy czym za pomocą klawiszy Q-góra, A-dół
można przesuwać wyświetlaną część.

Wyświetlane dane należy aktualizować co 1 sekundę (do zmiany opcją -v
z możliwością podawania liczb zmiennoprzecinkowych z kropką).

Jeżeli zostanie otwartych kilka połączeń, to każdy ma swój widok tabelki.

Do czyszczenia ekranu należy użyć odpowiedniego kodu sterującego terminalu
(patrz 'man console_codes'). Zakładamy, że terminal użytkownika jest zgodny
z VT100.


Podsumowanie opcji wiersza poleceń
----------------------------------

Należy pamiętać o następujących opcjach i ich domyślnych wartościach:
 * port serwera do pomiaru opóźnień przez UDP: 3382 (-u)
 * port serwera do połączeń z interfejsem użytkownika: 3637 (-U)
 * czas pomiędzy pomiarami opóźnień: 1 sekunda (-t)
 * czas pomiędzy wykrywaniem komputerów: 10 sekund (-T)
 * czas pomiędzy aktualizacjami interfejsu użytkownika: 1 sekunda (-v)
 * rozgłaszanie dostępu do usługi _ssh._tcp: domyślnie wyłączone (-s)


Skalowalność
------------

Należy zadbać o prawidłowe i efektywne działanie programu w różnych sieciach
wielkości do kilku tysięcy komputerów i dla opóźnień do 10 sekund. Należy
spodziewać się, że będzie testowana współpraca z programami innych studentów
lub z innymi implementacjami mDNS zgodnymi z podanymi standardami.


Rozwiązanie
-----------

Pliki źródłowe opoznienia.c i ewentualne pozostałe oraz Makefile należy umieścić
na serwerze SVN

https://svn.mimuw.edu.pl/repos/SIK/

w katalogu

students/ab123456/zadanie2

gdzie ab123456 to standardowy login wg schematu: inicjały, nr indeksu, używany
na maszynach wydziału. Można też umieścić tam inne pliki potrzebne do
skompilowania i uruchomienia programu, jeśli to jest konieczne.

Przy oddawaniu zadania należy sprawdzającemu najpierw zademonstrować
rozwiązanie działające na trzech przygotowanych maszynach wirtualnych.


Termin
------

Termin włożenia zadania do repozytorium to 8.06.2015 godz. 22:00. Termin ten
nie podlega przesunięciu, poza indywidualnymi, uzasadnionymi przypadkami
ustalonymi odpowiednio wcześnie z koordynatorem przedmiotu.

Każde rozpoczęte 48 godz. spóźnienia będzie kosztowało jeden punkt, jednak
w sumie za spóźnienia nie będzie odjętych więcej niż 2p.
