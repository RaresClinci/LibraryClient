# README Tema 4

## Implementare
Baza acestei teme a fost laboratorul 9, de unde am preluat functiile de creare de mesaj http si de trimitere de mesaj.

 Le-am portat apoi in C++ pentru a beneficia de functii de lucru cu stringuri, clase si de biblioteci pentru parsarea JSON-urilor

## Cererile
Clientul poate creea cereri de tipul: GET, POST si DELETE.

Pentru o implementare mai usoara, pentru GET si POST am doua tipuri de functii de creare a mesajelor: una cu operatii pe client, care permite transmiterea cookie-urilor, si una pe biblioteca, permitand transimiterea autorizatiei JWT.

Clientul interpreteaza raspunsul serverului si da un mesaj sugestiv dupa status si payload.

Dupa trimiterea unui mesaj, socketul pe care s-a trimis este inchis.

## Comenzi
Intr-o bucla infinita, clientul asteapta comenzi de la utilizator, pentru comanda exit, programul incheindu-se. Fiecare comanda apeleaza functia specifica.

Se respecta flowul din cerinta, fiind trimise cereri corespunzatoare.

O functie utila in acest sens a fost `get_status` care verifica statusul raspunsului si intoarce un boolean si de asemenea afiseaza mesaje custom de output/ eroare.

Functile care primesc input despre user verifica sa nu aiba spatii sau sa nu fie sir vid. Iar cele de carti verifica sa nu aiba campurile sir vid.

Functia de login ii da unei variabile globale `loginCookie` cookie-ul primit, iar functia de enter_library depoziteaza in variabila globala `libraryToken` JWT-ul de acces la biblioteca. Fiecare functie care necesita unul dintre aceste coduri va da un mesaj de eroare si se va opri inainte de input daca variabila globala corespunzatoare este nula. 

Functia de logout elibereaza cele 2 variabile globale.

Functia add_book foloseste o clasa `Book` pentru instantierea cartii si parsarea JSONului(are o metoda speciala). Aceasta clasa nu era strict necesara, dar am dorit sa incerc OOP in C++.

Functia de add_book, conform cerintei, esueaza si daca numarul de pagini nu este un numar.

## Biblioteca de JSON
Deoarece am utilizat C++ si serverul folosea ca payloa date de tip json, am utilizat biblioteca `nlohmann/json`

## Concluzie
Am invatat cum se folosesc diverse cereri HTTP intr-un context practic si am avut ocazia sa lucrez cu concepte noi in C++(biblioteci externe si putin OOP).

