/*
Silnik: 1 pe³ny krok 1.8 stopnia
Ko³o zêbate: 12mm
Liczba zêbów:20
Pasek zêbaty: --^--^--^--^--^--...^--^2mm

P1 - Góra
P2 - Dó³
P3 - +
P4 - -
P5 - ENTER
P1+P2 - help
Potencjometr- regulacja d³ugoœci kroku silniczka

*******Menu******
+0 START (Start nagrywania)
+1 D³ugoœæ kroku[ms]: 0.1-200
2 Iloœæ kroków na cykl: 1-10000
Il kr na cykl***
K:10000 I=******
3 Odstêp czasowy: x_h:x_min:x_sek:x_ms help:(Odstêp czasowy miêdzy zdjêciami)
+4 Tryb: Timelapse/Manual
5 AF po ka¿dym wyzw: TAK/NIE
+6 Iloœæ kroków na obrót: 100 200 400 800
+7 Kierunek obrotów
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include "src\motor.h"
#include "src\HD44780.h"
#include "src\button.h"
#include "src\ADC.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define KROK_DYST 0.2513
#define KROK_FULL_DIST 4058
#define KROK_FULL_DIST_D 4058.0
#define FULL_DIST 1020.0

#define CANON_PORT PORTD
#define CANON_AF_PIN          PORT7
#define CANON_SHUTTER_PIN     PORT6
#define CANON_SH_HOLD_DOWN  CANON_PORT |= ( 1<<CANON_SHUTTER_PIN)
#define CANON_SH_RELEASE    CANON_PORT &= ~(1<<CANON_SHUTTER_PIN)
#define CANON_AF_HOLD_DOWN  CANON_PORT |= ( 1<<CANON_AF_PIN)
#define CANON_AF_RELEASE    CANON_PORT &= ~(1<<CANON_AF_PIN)

//zmienne w pamiêci EEPROM
uint16_t EEMEM canon_af_delay_EE=4;
uint16_t EEMEM canon_sh_delay_EE=30;

//Zmienne globalne
extern unsigned char praca; //rodzaj pracy 100 200 400 800
extern bool kier; //kierunek obrotów
//flagi przycisków
extern bool btn_plus;
extern bool btn_minus;
extern bool btn_enter;
extern bool btn_up;
extern bool btn_down;
extern bool check_menu;

char tab[16]; // tablica to przechowywania tekstów
char p_menu=0; //okreœla pozycje w menu na której stoi kursor
bool st =0; //1-start nagrywania/ruchu
double t_kroku=1.9; //czas miêdzy krokami silnika
double tmp_t_kroku=19.0; //pomocnicza

//czasy
unsigned int tim; //w przerwanut timera1
char minuta=0;
char sekunda=0;
char godzina=0;
char sel=2; //wybór nastawy h m s
int czas_s=0; //wyliczony czas posuniêcia w sekundach

int iknc=50; //iloœæ kroków na cykl
uint32_t lz; //liczba zrobionych zdjêæ na 1 przejazd
uint32_t lz_temp=0; //tymczasowa do u¿ytku w pêtli g³ównej

bool tryb = 0; //0Timelapse/1manual
unsigned char ikno=2; // iloœæ kroków na obrót/ odczyt nastawy

int i; //zmienna pomocnicza do for
bool escape=0;// na przerwanie escape siê uaktywnia(œrodkowy klawisz pada)

long int canon_tim=0;// timer migawki
const double canon_t_1nasw=0.016; //czas kroku naswietlania
long int canon_tim_set=65;
bool flaga0=0;
bool flaga1=0;
bool af = 1;
bool submenu_exit=0;

uint16_t canon_af_delay; //pobieranie czasów z eeprom
uint16_t canon_sh_delay;

uint16_t canon_delay=0; //opoznienie po dojechaniu wozka

void Timer0_start(void)
{
    TIMSK |= (1<<TOIE0);
    TCNT0 =5; //przepe³nienie co 0.016s
    canon_tim=0;
}

void Timer0_stop(void)
{
    TIMSK &= ~(1<<TOIE0); //stój
    TCNT0 =5; //przepe³nienie co 0.016s
    canon_tim=0;
    CANON_SH_RELEASE;
}

ISR(TIMER1_OVF_vect) //przerwanie na przepe³nienie timera 1 (co  1 sekunde)
{
    tim++;
    TCNT1=49910;
}

ISR(INT0_vect)
{
    escape=1;
}

ISR(TIMER0_OVF_vect) //czas trzymania spustu migawki(bulb)
{
    canon_delay++;//opóŸnienie na ustabilizowanie siê aparatu

    if(canon_delay>canon_af_delay && af==1) //jeœli autofokus jest aktywowany...
        CANON_AF_HOLD_DOWN;

    if (canon_delay>canon_sh_delay)//1=16ms
    {
    canon_tim++;
    if(canon_tim<=canon_tim_set)
    {
        CANON_SH_HOLD_DOWN;
    }
    else
    {
        CANON_SH_RELEASE;
        CANON_AF_RELEASE;
        Timer0_stop();
        canon_delay=0;
    }
    }
}

ISR(TIMER2_OVF_vect)
{
    //Check_Button();     //SprawdŸ czy zosta³ naciœniêty któryœ przycisk
}

void Port_init(void)
{
    DDRB = 0xff;
    PORTB = 0x00;

    DDRA = 0x00;
    PORTA = 0x00;

    PORTD = 0xff;
    DDRD = 0b11000000; //bity 7-AF 6-SH 2-PD; reszta wolne
}

void canon_init(void)
{
    //eeprom_update_word(&canon_af_delay_EE, 11);// nadpisywanie/aktualizacja zmiennej wygl¹da tak! proste
    canon_af_delay=eeprom_read_word(&canon_af_delay_EE);
    canon_sh_delay=eeprom_read_word(&canon_sh_delay_EE);
    CANON_AF_RELEASE;
    CANON_SH_RELEASE;
}

void int0init(void)
{
    MCUCR |= (1<<ISC00)|(1<<ISC01);
    GICR |= (1<<INT0);
}
void Timer_init(void)
{
    TCCR1B |= (1<<CS10) | (1<<CS12); //Timer 1 prescaler 1024
    //TIMSK |= (1<<TOIE1); //overflow
    TCNT1 = 49910; //Wartoœæ pocz¹tkowa
    tim=0;
    TCCR0 |= (1<<CS00) | (1<<CS02); // Timer 0 prescaler 1024
    TCCR2 |= (1<<CS20) | (1<<CS21) | (1<<CS22);// Timer 2 prescale 1024
}

void Timer_start(void)
{
    TCNT1 = 49910; //Wartoœæ pocz¹tkowa
    TIMSK |= (1<<TOIE1);
    tim=0;
}

void Timer_stop(void)
{
    TIMSK &= ~(1<<TOIE1);
    TCNT1 = 49910; //Wartoœæ pocz¹tkowa
    tim=0;
}


void Timer2_start(void)
{
    TIMSK |= (1<<TOIE2);
    TCNT2 = 0;
}
int set_Time(char h, char min, char sek)
{
    // Przelicza czas z hms na sekundy
    int time;
    time = sek + min*60 + h*3600;
    return (int)time;
}

void canon_AF(void)
{

}

void canon_Shutter(void)
{

}

void submenu_Advaced(void)
{
    double canon_sh_delay_temp, canon_af_delay_temp;

    Check_Button();
    if(btn_up)
    {
        p_menu++;
        if(p_menu>6)    //zawiñ do pocz¹tku
            p_menu=0;
        Btn_flags_erase();
    }

    if(btn_down)
    {
    p_menu--;
    if(p_menu<0)    //zawiñ do koñca
    p_menu=6;
    Btn_flags_erase();
    }

     if(check_menu==1)
    {
    switch(p_menu)
    {
    case 0:
//********************Autofocus auto/manual***********************************/
            LCD_Clear();
            LCD_WriteText("AF");
            LCD_GoTo(0,1);
            if (btn_plus||btn_minus)
                af = !af;
            if (af == 0)
                LCD_WriteText("Manual");
            else
                LCD_WriteText("Auto");
            check_menu=0;
            Btn_flags_erase();
        break;
    case 1:
        //*******************Czas na stabilizacje statywu************************/
            LCD_Clear();
            LCD_WriteText("ShutterDelay[s]");
            LCD_GoTo(0,1);
            if(btn_plus)
                canon_sh_delay++;
            if(btn_minus)
                canon_sh_delay--;
            if(canon_sh_delay<=1)
                canon_sh_delay=1;
            canon_sh_delay_temp=(double)canon_sh_delay*0.016;
            sprintf(tab,"%.3f",canon_sh_delay_temp);
            LCD_WriteText(tab);
            check_menu=0;
            Btn_flags_erase();
        break;
    case 2:
        //*******************OpóŸnienie AF************************/
            LCD_Clear();
            LCD_WriteText("AFDelay[s]");
            LCD_GoTo(0,1);
            if(btn_plus)
                canon_af_delay++;
            if(btn_minus)
                canon_af_delay--;
            if(canon_af_delay<=1)
                canon_af_delay=1;
            canon_af_delay_temp=(double)canon_af_delay*0.016;
            sprintf(tab,"%.3f",canon_af_delay_temp);
            LCD_WriteText(tab);
            check_menu=0;
            Btn_flags_erase();
        break;
    case 3:
//********************************Zapis*****************************************/
        LCD_Clear();
        LCD_WriteText("ZAPISZ");
        if(btn_enter)
        {
            LCD_Clear();
            LCD_WriteText("Z A P I S A N O ");
            _delay_ms(100);
            eeprom_update_word(&canon_af_delay_EE, canon_af_delay);
            eeprom_update_word(&canon_sh_delay_EE, canon_sh_delay);
            _delay_ms(1000);
        }
        LCD_Clear();
        LCD_WriteText("ZAPISZ");
        check_menu=0;
        Btn_flags_erase();
        break;
    case 4:
//******************************************Info*********************************/
    case 5:
        LCD_Clear();
        LCD_WriteText("Info");
        check_menu=0;
        Btn_flags_erase();
        break;
//*************************************Wyjscie********************************/
        case 6:
        LCD_Clear();
        LCD_WriteText("WYJSCIE");
        if(btn_enter)
        {
            submenu_exit=1;
        }
        check_menu=0;
        Btn_flags_erase();
        break;
    }
    }
}

void przelicz(void)
{
    lz = FULL_DIST*(uint32_t)ikno/(0.2*(float)iknc); //liczba zdjêæ na przejazd ikno mno¿nik zale¿ny od podzia³ki kroków silnika

}
///********************************************
///*******************MENU G£ÓWNE**************
///********************************************
void GoTo_menu(void)
{
    bool flag0=1; //case 3
    uint16_t nfo_tleft; //case 8
    double iknc_cm; //ile centymetrów na przejazd miêdzy zdjêciami
    double canon_t_double;

    if(btn_up)
    {
        p_menu++;
        if(p_menu>10)    //zawiñ do pocz¹tku
            p_menu=0;
        Btn_flags_erase();
    }

    if(btn_down)
    {
    p_menu--;
    if(p_menu<0)    //zawiñ do koñca
    p_menu=10;
    Btn_flags_erase();
    }
        switch(p_menu) //menu g³ówne wyœwietlanie
        {
//***************START*******************
        case 0:
            LCD_Clear();
            //LCD_GoTo(0,0)
            LCD_WriteText("START");
            if(btn_enter){
                st=!st;
            }
            LCD_GoTo(0,1);
            if(st==0){
                LCD_WriteText("Nie");
                Timer_stop();
            }
                else{
                LCD_WriteText("Tak");
                Timer_start();
            }
            check_menu=0;
            Btn_flags_erase();
            break;
        case 1:
//*************T Kroku*****************
            LCD_Clear();
            LCD_WriteText("T kroku [ms]");
            if(btn_plus)
                tmp_t_kroku++;
            if(btn_minus)
                tmp_t_kroku--;
            if(tmp_t_kroku>8000)
                tmp_t_kroku = 8000;
            if(tmp_t_kroku<=0)
                tmp_t_kroku=0;
            t_kroku=tmp_t_kroku/10;
            LCD_GoTo(0,1);
            //itoa(tmp_t_kroku,tab,10); //zapisuje liczbe w dowolnym systemie jako tekst
            sprintf(tab,"%.3f",t_kroku);
            LCD_WriteText(tab);
            check_menu=0;
            Btn_flags_erase();
            break;
        case 2:
//***************Iloœæ kroków na 1 cykl************************** +Zrobiæ przeliczanie na mm
            LCD_Clear();
            LCD_WriteText("Il. kr. na. cykl");

            if(btn_plus)
                iknc++;
            if(btn_minus)
                iknc--;
            if(iknc<=1)
                iknc=1;
            if(iknc>=4000)
                iknc=4000;
            LCD_GoTo(0,1);
            itoa(iknc,tab,10);
            LCD_WriteText(tab);
            LCD_GoTo(6,1); //przeliczanie na il zdjec
            //LCD_WriteText("Lz:");
            //przelicz();
            iknc_cm=iknc*0.02/ikno;
            //lz = FULL_DIST*(uint32_t)ikno/(0.188*(float)iknc); //liczba zdjêæ na przejazd ikno mno¿nik zale¿ny od podzia³ki kroków silnika
            //ltoa(lz,tab,10);
            sprintf(tab,"D=%.3f",iknc_cm);
            LCD_WriteText(tab);

            przelicz();
            check_menu=0;
            Btn_flags_erase();
            break;
        case 3:
//******************************Step********************************//
            LCD_Clear();
            LCD_WriteText("Pojedynczy krok");
            if(btn_enter && flag0==1)
            {
                Krok();
                flag0=0;
            }
            else
            {
                flag0 =1;
            }
            check_menu=0;
            Btn_flags_erase();
            break;
        case 4:
//*************** TRYB *********************/
            LCD_Clear();
            LCD_WriteText("Tryb");
            LCD_GoTo(0,1);
            if(btn_enter)
            {
                tryb = !tryb;
                //tryb = 0;
            }
            if(tryb==0)
            {
                LCD_WriteText("Timelapse");
            }
            else
            {
                LCD_WriteText("Manual");
            }
            check_menu=0;
            Btn_flags_erase();
            break;
//************Iloœæ kroków na obrót silnika***************
        case 5:
            LCD_Clear();
            LCD_WriteText("Tryb pracy siln");
            LCD_GoTo(0,1);
            if(btn_plus)
                Nast_tryb();
            if(btn_minus)
                Poprz_tryb();
            //itoa(praca,tab,10);
            //LCD_WriteText(tab);
            switch(praca)
            {
            case 1:
                LCD_WriteText("1 :krok");
                ikno = 1;
                break;
            case 2:
                LCD_WriteText("1/2 :kroku");
                ikno = 2;
                break;
            case 3:
                LCD_WriteText("1/4 :kroku");
                ikno = 4;
                break;
            case 4:
                LCD_WriteText("1/8 :kroku");
                ikno = 8;
                break;
            case 5:
                LCD_WriteText("1/16 :kroku");
                ikno = 16;
                break;
            }
            przelicz();
            check_menu=0;
            Btn_flags_erase();
            break;
        case 6:
//******************Kierunek obrotów*********************************/
            LCD_Clear();
            LCD_WriteText("Kierunek obrotow");
            LCD_GoTo(0,1);
            if(btn_plus || btn_minus)
                Zmien_kierunek();
            if(kier)
                    LCD_WriteText("--->");
                    else
                    LCD_WriteText("<---");
            check_menu=0;
            Btn_flags_erase();
            break;
        case 7:
//****************************Czas jedniego cyklu************************************************************//
            LCD_Clear();
            LCD_WriteText("t. cyklu:");
            LCD_GoTo(0,1);
            LCD_WriteText("h:   m:   s:  ");
            if(btn_enter)
            {
                sel++;
                if(sel>2)
                    sel=0;
                _delay_ms(200);
            }
            switch(sel)
            {
            case 2:
                LCD_GoTo(10,1);
                LCD_WriteText("S:");

                if(btn_plus)
                    sekunda++;
                if(btn_minus)
                    sekunda--;
                if(sekunda>=60)
                    sekunda=59;
                if(sekunda<=0)
                    sekunda=0;
                break;
            case 1:
                LCD_GoTo(5,1);
                LCD_WriteText("M:");

                if(btn_plus)
                    minuta++;
                if(btn_minus)
                    minuta--;
                if(minuta>=60)
                    minuta=59;
                if(minuta<=0)
                    minuta=0;
                break;
            case 0:
                LCD_GoTo(0,1);
                LCD_WriteText("H:");

                if(btn_plus)
                    godzina++;
                if(btn_minus)
                    godzina--;
                if(godzina>=99)
                    godzina=99;
                if(godzina<=0)
                    godzina=0;
                break;
            }
//Wype³nij drugi wiersz
            LCD_GoTo(12,1); ///S
            itoa(sekunda,tab,10);
            LCD_WriteText(tab);

            LCD_GoTo(7,1);///M
            itoa(minuta,tab,10);
            LCD_WriteText(tab);

            LCD_GoTo(2,1);///H
            itoa(godzina,tab,10);
            LCD_WriteText(tab);

            czas_s = set_Time(godzina,minuta,sekunda);

            check_menu=0;
            Btn_flags_erase();
            break;
            case 8:
//*****************************************Info*************************************
            LCD_Clear();
            LCD_WriteText("Pozostalo:   min");
            nfo_tleft=lz*czas_s;
            nfo_tleft=nfo_tleft + (uint16_t)(t_kroku*KROK_FULL_DIST_D*(double)ikno);
            nfo_tleft=nfo_tleft/60;
            itoa(nfo_tleft,tab,10);
            LCD_GoTo(10,0);
            LCD_WriteText(tab);
            LCD_GoTo(0,1);
            przelicz();
            itoa(lz,tab,10);
            LCD_WriteText("L.Zdjec: ");
            LCD_WriteText(tab);
            check_menu=0;
            Btn_flags_erase();
                break;
//***********************************Czas naswietlania*****************************************
            case 9:
            LCD_Clear();
            LCD_WriteText("T. ekspozycji:");
            if (btn_plus) //nastawa
                canon_tim_set++;
            if (btn_minus)
                canon_tim_set--;
            if (canon_tim_set<=1)
                canon_tim_set=1;
            canon_t_double = canon_t_1nasw*(double)canon_tim_set;
            LCD_GoTo(0,1);
            sprintf(tab,"%f.3",canon_t_double);
            LCD_WriteText(tab);
            check_menu=0;
            Btn_flags_erase();
                break;
            case 10:
//********************************************ADVACED**********************
            if(btn_enter)
            {
				submenu_exit=0;
				p_menu=0; //zacznij od pozycji 0 nast menu
                while(submenu_exit==0) //teraz jesteœ w submenu
                    submenu_Advaced();
                p_menu=10; //wróæ na poprzedni¹ pozycje
            }
            LCD_Clear();
			LCD_WriteText("Zaawansowane");
            check_menu=0;
            Btn_flags_erase();
                break;
        default:
            //do nothing
            break;
        }
}
///*************************************************
///*******************MENU G£ÓWNE KONIEC************
///*************************************************


///Funkcja g³ówna//////////////////////////////////////////
int main(void)
{
    double analog; //pozycja potencjometru
    uint32_t lz_powrot;

    Port_init();
    Motor_init();
    Rodzaj_pracy(praca);
    Kierunek(kier);
    LCD_Initalize();
    LCD_Clear();
    ADC_init();
    Timer_init();
    int0init();
    przelicz();
    canon_init();
    sei();

    while(1)
    {
        if(tryb==0)
        {        //je¿eli tryb jest automatyczny
        Check_Button();     //SprawdŸ czy zosta³ naciœniêty któryœ przycisk
        if(check_menu==1)   //Jeœli tak przejdŸ do menu.
        GoTo_menu();
        }

///Tutaj praca krokowa w odstepach czasowych
        if(st==1 && ALL_RELEASE && tryb==0){  //Je¿eli start jest w³¹czony i przyciski s¹ zwolnione uruchom silnik i tryb jest timelapse
            if(flaga1==1){//jeœli flaga startu ruchu wózka =1
            tim=0; // zacznij liczyæ czas od pocz¹tku
            flaga1=0;
            }
          /* if(flaga0==1){//je¿eli czas jest wiêkszy ni¿ 1/4 czasu odstêpu md zdjêciami
           //if(tim>=czas_s/4 && flaga0==0){//je¿eli czas jest wiêkszy ni¿ 1/4 czasu odstêpu md zdjêciami
                Timer0_start(); //zacznij robiæ zdjêcie
                     LCD_Clear();///debug
                     LCD_WriteText("Tak!");///debug
                     flaga0 = 0; //oznacza ¿e wykonano
            }*/
            if(tim>=czas_s){ //jesli licznik jest wiêkszy ni¿ czas nastawiony
                flaga1=1;
                for(i = iknc; i>0; i--){ //zrób iloœæ kroków okreœlon¹ w cyklu
                    _delay_ms(t_kroku);
                    Krok();
                    }
                lz_temp++;
                flaga0=0; //czyœæ flage wykonania zdjêcia
                Timer0_start();// zacznij robiæ zdjêcie
                if (lz_temp>=lz){ //jeœli osi¹gniêto limit zdjêæ zatrzymaj maszyne
                    lz_temp=0;
                    st=0;
                    Rodzaj_pracy(2);//1/2
                    Zmien_kierunek();
                    lz_powrot = FULL_DIST*2/0.2; //droga powrotu w trybie II jest sta³a
                    for (i=lz_powrot; i>=0 && escape!=1; i--) //i cofnij do pocz¹tki *2bo ikno jest 2
                    {
                        _delay_ms(1.7);
                        Krok();
                    }
                    if(escape==1) //zastopuj
                        st=0;
                    Zmien_kierunek(); //wróæ do poprzedniego kierunku jazdy
                    Rodzaj_pracy(praca);//wróæ do akrualnego trybu pracy
                    GoTo_menu(); //odœwie¿
                }
            }
        }
        if(st==0) //zeruj licznik kroków
        {
            lz_temp=0;
            Timer0_stop();
        }

///Obs³uga manualnego Trybu
        while(tryb==1) //jeœli jesteœ na manualu
        {
            analog=(double)read_Y();
            analog=(double)analog*100.0;
            /*//debug
            LCD_Clear();
            //itoa(analog,tab,10);
            sprintf(tab, "zm=%f",analog);
            LCD_WriteText(tab);
            ///Debug**/
            if (analog>0.0)
                {
                Kierunek(1);
                _delay_us(analog);
                if(analog<=48000.0 && analog>=-48000.0)
                    Krok();
                }
            if (analog<=0.0)
            {
                analog = analog*(-1);
                Kierunek(0);
                _delay_us(analog);
                if(analog<=48000.0 && analog>=-48000.0)
                    Krok();
            }
            if(UP) //joystick w górê wychodzi z tego trybu
            {
                tryb=0;
                check_menu=1;
                _delay_ms(500);
            }
            if(P5)
            {
                CANON_SH_HOLD_DOWN;
                _delay_ms(1); //drgania styków
            }
            else
            {
                CANON_SH_RELEASE;
            }

            if(DOWN)
            {
                CANON_AF_HOLD_DOWN;
            }
            else
            {
                CANON_AF_RELEASE;
            }

        }
        escape=0;
    }

    return 0;
}
