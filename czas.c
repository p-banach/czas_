#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define dlugosc_etykiet 2400
#define ilosc_etykiet 1000
#define ilosc_instrukcji 3500
#define dlugosc_stosu 4000
#define pamiec_zapis 10000

/*obiekty ogólne:
 * pamiec to obszar do ktorego mozna zapisywac dane*/
int stos[dlugosc_stosu];
int miejsce_stos=-1;
int pamiec[pamiec_zapis];

struct etykieta{
    /* nazwy etykiet są przechowywane we wspólnej tablicy,
     * adres to miejsce w ktorym rozpoczyna sie nazwa danej etykiety
     * len to dlugos jej nazwy
     * numer instrukcji wskazuje do ktorego miesca w programie nalezy skoczyc*/
    int adres;
    int len;
    int numer_instrukcji;
};
/* Obiekty związane z etykietami. Ich nazwy zapisywane są w tablicy nazwy_etykiet,
 * struktura zawiera informacje na ktorym miejscu w tej tablicy
 * znajduje sie nazwa odpowiadajacej etykiety
 * pozycja w tablicy to wspolny licznik
 * max dlugosc to najdluzsza dotychcszas spotkana etykieta
 * */
struct etykieta etykiety[ilosc_etykiet];
char nazwy_etykiet[dlugosc_etykiet];
int liczba_etykiet = 0;
int pozycja_w_tablicy = 0;
int max_dlugosc=0;

struct instrukcja{
    /* typ instrukcji od 1 do 6
     * 1 - odejmowanie
     * 2 - skok
     * 3 - etykieta | argument wskazuje na odpowiednia etykiete do wywolania
     * 4 - powrót
     * 5 - czytanie |
     * 6 - pisanie */
    int typ;
    int arg1;
    int arg2;
};
/* obiekty związane z instrukcjami. */
struct instrukcja instrukcje[ilosc_instrukcji];
int liczba_instrukcji = 0;


int PobierzLiczbe(char *c){
    /*podczas parsowania programu, po trafieniu na znak cyfry lub '-'
     * ta funkcja wczytuje resztę liczby*/
    char s[10] = "0";

    int x=1;
    if(*c==(int)'-'){
        x = -1;
        *c=getchar();
    }

    s[0] = *c;
    int l = 1;
    *c = getchar();

    while (isdigit(*c) && *c != EOF && *c != '\n' && *c != 32) {
        s[l] = *c;
        *c = getchar();
        l++;
    }
    return x*atoi(s);
}

bool JestSeparatorem(char c){
    /*
     * 32 space
     * 10 new line
     * 11 tab
     * 9 horizontal tab*/
    return (c==EOF || c=='\n' || c=='|' || c== 32 || c==11 || c==10 || c==9);
}

bool JestLiczba(char c){
    /*'-' rowniez wskazuje że nastepna będzie cyfra */
    return (isdigit(c) || c=='-');
}

bool PobierzPoczatekEtykiety(char nazwa[], char *c, int stlen, int *len){
    /* przekazuje 1 jezeli nowa etykieta jest krotsza od najdluzszej dotychczasowej
     * W innym wypadku nie trzeba juz sprawdzac czy wczesniej sie pojawila */
    nazwa[0]=*c;
    int i = 1;
    while(i<stlen && !JestSeparatorem(*c)){
        *c=getchar();
        if(!JestSeparatorem(*c)){
            nazwa[i]=*c;
            i++;
        }
    }
    /* przekazuje dlugosc pobranego fragmentu za pomocą *len */
    *len = i;

    return i<stlen;
}

void PobierzReszteEtykiety(char *c){
    /*Używana do pobrania pozostalosci etykiety dluzszej niz dotychczasowe */
    while(!JestSeparatorem(*c)){
        nazwy_etykiet[pozycja_w_tablicy]=*c;
        *c=getchar();
        pozycja_w_tablicy++;
    }
}

void Przepisz(char from[], char to[],  int ile ){
    /*Tak jak w nazwie */
    for(int i = 0; i<ile; i++){
        to[i] = from[i];
    }
}

bool CzyTakieSame(char jeden[], char dwa[], int n){
    /*Tak jak w nazwie */
    for(int i = 0; i<n; i++){
        if(jeden[i]!=dwa[i]){
            return false;
        }
    }
    return true;
}

bool SzukajEtykiety(char nazwa[], int len, int *ktora_ety){
    /*Sprawdzając kolejne nazwy etykiet porownuje pobrany fragment */
    *ktora_ety=-1;
    int i = 0;
    bool x = 0;
    while(i<liczba_etykiet && x!=true){
        /*Jezeli ilosc znakow jest rozna to na pewno nie będą takie same */
        if(etykiety[i].len==len) {
            /*Przekopiowuję fragment tablicy nazwy_etykiet, aby użyć funkcji porownującej */
            char *str = (char *) malloc(etykiety[i].len * sizeof(char));
            Przepisz(nazwy_etykiet + etykiety[i].adres, str, etykiety[i].len+1);
            x = CzyTakieSame(nazwa,str,len);
            free(str);
        }
        i++;
    }
    /*Jeżeli się znalazła, to przez ktora_ety przekazujemy jej numer
     * w innym wypadku przekazujemy obecna ilosc etykiet  */
    if(x){
        *ktora_ety=i-1;
    }
    else{
        *ktora_ety=liczba_etykiet;
    }
    return x;
}

void PrzygotujPamiec(){
    /* Według specyfikacji początkowy stan pamięci komórki a to (-1-a) */
    for(int i =(-1)*(pamiec_zapis/2); i<pamiec_zapis/2; i++){
        pamiec[i]=-1-i;
    }
}

void PominSeparatory(char *c){
    /* Tak jak w nazwie */
    while (*c != EOF && (*c == '\n' || *c == 32 || *c == '|' || *c==11)) {
        *c = getchar();
    }
}

void SzukajNajdluzszej(void){
    /* ustala długość najdłuższej dotychczas nazwy etykiety
    * przedatne do sprawdzania czy dana etykieta juz byla sprawdzona*/
    if(etykiety[liczba_etykiet-1].len>max_dlugosc){
        max_dlugosc=etykiety[liczba_etykiet-1].len;
    }
}

void DodajInstrukcje(int typ, int arg1, int arg2){
    /* Dodaję nową instrukcję w tablicy
     * -1 uznaje za argument pusty */
    instrukcje[liczba_instrukcji].typ=typ;
    instrukcje[liczba_instrukcji].arg1=arg1;
    instrukcje[liczba_instrukcji].arg2=arg2;
    liczba_instrukcji++;
}

void DodajEtykiete(int adres, int len, int numer){
    /* Tworzy nową etykietę w tablicy
     * wywoluje funkcje sprawdzajaca czy jest najdluzsza */
    etykiety[liczba_etykiet].adres=adres;
    etykiety[liczba_etykiet].len=len;
    etykiety[liczba_etykiet].numer_instrukcji=numer;
    liczba_etykiet++;
    SzukajNajdluzszej();
}

void ZmienEtykiete(int adres, int len, int numer,int ktora){
    /*Używana do zmiany w przypadku gdy najpierw
     * pojawia sie skok lub wywolanie,
     * a dopiero potem definicja etykiety*/
    if(adres!=-1){
        etykiety[ktora].adres=adres;
    }
    if(len!=-1) {
        etykiety[ktora].len = len;
    }
    if(numer!=-1) {
        etykiety[ktora].numer_instrukcji = numer;
    }

}

void DefinicjaEtykiety(char *c, char nazwa[], int len, int *ktora_ety){
    /* Po naptkaniu definicji etykiety, ta funkcja sprawda czy zostala juz zapisana w tablicy nazw
     * i przypisuje jej odpowiednia instrukcję jeżeli tak,
     * w innym wypadku tworzy nową etykietę o poprawnych własnościach*/
    if(PobierzPoczatekEtykiety(nazwa, c, 1 + max_dlugosc, &len)){
        int znalazlem = SzukajEtykiety(nazwa, len, ktora_ety);
        if (znalazlem==0){
            Przepisz(nazwa, nazwy_etykiet + pozycja_w_tablicy, len);
            DodajEtykiete(pozycja_w_tablicy, len, liczba_instrukcji);
            /* przesuwam pozycję w tablicy nazw o długość nazwy znalezionej etykiety */
            pozycja_w_tablicy += len;
        }
        else
        {
            ZmienEtykiete(-1, -1, liczba_instrukcji, *ktora_ety);
        }
    }
    else{
        /*wejscie do tego fragmentu kodu oznacza ze nowa etykieta
         * na pewno nie zostala jeszcze zdefiniowana, więc od razu tworzę nową*/
        int adres = pozycja_w_tablicy;
        Przepisz(nazwa, nazwy_etykiet + adres, len);
        /* przesuwam pozycję w tablicy nazw o długość nazwy znalezionej etykiety */
        pozycja_w_tablicy += len;
        *c = getchar();
        PobierzReszteEtykiety(c);
        DodajEtykiete(adres, pozycja_w_tablicy - adres, liczba_instrukcji );
    }
}

void SkokLubWywolanie(char *c, char nazwa[], int len, int *ktora_ety, int arg1, int typ){
    /*Funkcja podobna do DefinicjaEtykiety, ale na przypadek znalezienia skoku lub wywołania */
    if(PobierzPoczatekEtykiety(nazwa,c,1 + max_dlugosc,&len)) {
        int znalazlem = SzukajEtykiety(nazwa, len, ktora_ety);

        if(znalazlem==0){
            Przepisz(nazwa,nazwy_etykiet+pozycja_w_tablicy,len);
            DodajInstrukcje(typ,arg1,liczba_etykiet);
            DodajEtykiete(pozycja_w_tablicy,len,-1);
            pozycja_w_tablicy+=len;
        }
        else{
            ZmienEtykiete(-1,-1,-1,*ktora_ety);
            DodajInstrukcje(typ,arg1,*ktora_ety);
        }
    }
    else{
        int adres = pozycja_w_tablicy;
        Przepisz(nazwa,nazwy_etykiet+adres,len);
        pozycja_w_tablicy+=len;
        *c=getchar();
        PobierzReszteEtykiety(c);
        DodajEtykiete(adres,pozycja_w_tablicy-adres,-1);
        /*W tym wypadku dodajemy również instrukcje ze wszystkimi elementami */
        DodajInstrukcje(typ,arg1,liczba_etykiet-1);
    }
}

void Parsuj(void){
    /* Funkcja służąca do rozpoznawania instrukcji oraz etykiet, i zapisywania ich
     * w odpowiednich strukturach*/
    char c;
    c=getchar();
    /*Napotkanie konca pliku lub początku danych kończy wczytywanie */
    while(c!=EOF && c!='&') {
        /* tablica nazwa jest o jeden dluzsza,
         * niz najdluzsza dotychczas spotkana etykieta
         * Używam jej do porownywania nowych etykiet ze starymi, aby uniknac duplikatów*/
        char *nazwa = (char *)malloc(1+(max_dlugosc*sizeof(char)));
        nazwa[0]='\0';
        /*len to dlugosc nowej etykiety w tablicy nazwa,
         * ktora ety to zmienna sluzaca do przekazywania do funkcji szukajacych*/
        int ktora_ety;
        int len=0;

        PominSeparatory(&c);
        /* Pierwsze trzy przypadki są oczywiste
         * ; to pełna instrukcja
         * po ^ zawsze następuję cyfra
         * po : zawsze następuję definicja*/
        if(c==';'){
        DodajInstrukcje(4,-1,-1);
        }
        else if(c=='^'){          /* Po spotkaniu znaku ^, pobieramy kolejny i pomijamy (ewentualne) separatory, */
            c=getchar();          /* Ponieważ jest to pierwszy znak w tej instrukcji, to nastepny na pewno bedzie adres */
            PominSeparatory(&c);  /* czyli po pominieciu separatorow spotkamy cyfre*/
            DodajInstrukcje(5, PobierzLiczbe(&c),-1);
        }
        else if(c==':'){
            c=getchar();
            DefinicjaEtykiety(&c ,nazwa,len,&ktora_ety);
        }
        else if(JestLiczba(c)) {
            /*Liczba może oznaczać
             * 1 instrukcję pisania
             * 2 instrukcję odejmowania
             * 3 instrukcję skoku warunkowego*/
            int arg1= PobierzLiczbe(&c);
            PominSeparatory(&c);
            if(c=='^'){
                DodajInstrukcje(6,arg1,-1);
            }
            else if(JestLiczba(c)){
                DodajInstrukcje(1,arg1, PobierzLiczbe(&c));
            }
            else if(isalpha(c)){
                SkokLubWywolanie(&c,nazwa,len,&ktora_ety, arg1, 2);
            }
        }
        else if(isalpha(c)) {
            /* litera pojawiająca się jako pierwsza oznacza wywołanie etykiety */
            SkokLubWywolanie(&c,nazwa,len,&ktora_ety,-1,3);
        }
        if(c!=EOF && c!='&'){
            c=getchar();
        }
        free(nazwa);
    }

}

int OdczytajWartosc(int arg){
    /*Sluzy do upewnienia się że nie wyidziemy poza zakres pamieci
     * maszyny, sluzacej do zapisywania
     * przekazuje wartosc zapisana pod przekazanym adresem, lub wartosc domyslna*/
    int wartosc;
    if(arg>= (-1)*(pamiec_zapis/2) && arg < pamiec_zapis/2){
        wartosc=pamiec[arg];
    }
    else wartosc=-1-arg;
    return wartosc;
}

void WykonajOdejmowanie(struct instrukcja inst){
    /* tak jak w nazwie, wykonuje instrukcję odejmowania adresów */
    int arg1 = inst.arg1; //3
    int arg2 = inst.arg2; //1

    int p_arg1=OdczytajWartosc(arg1); //30
    int p_arg2=OdczytajWartosc(arg2); //10

    int pp_arg1=OdczytajWartosc(p_arg1); //300
    int pp_arg2=OdczytajWartosc(p_arg2); //100

    pamiec[p_arg1]=pp_arg1-pp_arg2;
}

void WykonajSkok(struct instrukcja inst, int *i){
    /* tak jak w nazwie, sprawdza warunek i wykonuje przejscie do podanej etykiety */
    int arg1 =inst.arg1;
    int p_arg1 = OdczytajWartosc(arg1);
    if(OdczytajWartosc(p_arg1)>0){
        *i=etykiety[inst.arg2].numer_instrukcji-1;
    }
}

void WywolajEtykiete(struct instrukcja inst, int *i){
    /*przesuwa miejsce na stosie o 1, zapisuje tam następującą pozycję
     * i przechodzi do podanego miejsca kodu */
    miejsce_stos++;
    stos[miejsce_stos]=*i+1;
    *i=etykiety[inst.arg2].numer_instrukcji-1;
}

void WykonajPowrot(int *i){
    /* zdejmuje jeden rekord ze stosu powrotu i przechodzi do
     * odpowiedniego miejsca programu*/
    if(miejsce_stos>-1){
        *i=stos[miejsce_stos]-1;
        miejsce_stos--;
    }
    else{
        /*to oznacza koniec programu */
        *i=liczba_instrukcji+1;
    }
}

void Czytaj(struct instrukcja inst){
    /* wykonuje instrukcję czytania do adresu */
    int arg1 =inst.arg1 ;
    int p_arg1=OdczytajWartosc(arg1);

    char c = getchar();
    if(c!=EOF){
        pamiec[p_arg1]=c;
    }
    else{
        pamiec[arg1]=-1;
    }
}

void Pisz(struct instrukcja inst){
    /* wykonuje instrukcję pisania z adresu */
    int arg1 = inst.arg1;
    int p_arg1 = OdczytajWartosc(arg1);
    int pp_arg1 =OdczytajWartosc(p_arg1);
    putchar(pp_arg1);
}

void WykonajInstrukcje(struct instrukcja inst, int typ, int *i){
    /* Wywołuje odpowiednie funkcje zależnie od typu instrucji */
    switch(typ){
        case 1:
            WykonajOdejmowanie(inst);
            break;
        case 2:
            WykonajSkok(inst, i);
            break;
        case 3:
            WywolajEtykiete(inst, i);
            break;
        case 4:
            WykonajPowrot(i);
            break;
        case 5:
            Czytaj(inst);
            break;
        case 6:
            Pisz(inst);
            break;
        default:
            break;
    }
}

void WykonajProgram(){
    /* Wywołuje program do końca instrukcji */
    for(int i = 0; i<liczba_instrukcji; i++){
        WykonajInstrukcje(instrukcje[i], instrukcje[i].typ, &i);
    }
}

int main() {
    PrzygotujPamiec();
    Parsuj();
    WykonajProgram();
    return 0;
}