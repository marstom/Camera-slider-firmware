/*

*/
#include "motor.h"


unsigned char praca=2; //tryb pracy silnika krokowego
bool kier=0;

void Rodzaj_pracy(unsigned char tryb) //tryb pracy sterownika 1-5
{
    //PORT_SILNIK ^= 0b00000111;
    switch(tryb)
    {
    case 1: //  pe³ny krok
        PORT_SILNIK &= ~((1<<MS1)|(1<<MS2)|(1<<MS3));
        break;
    case 2: //  1/2kroku
        PORT_SILNIK |= (1<<MS1);
        PORT_SILNIK &= ~((1<<MS2)|(1<<MS3));
        break;
    case 3: //1/4 kroku
        PORT_SILNIK |= (1<<MS2);
        PORT_SILNIK &= ~((1<<MS1)|(1<<MS3));
        break;
    case 4: //1/8 kroku
        PORT_SILNIK |= (1<<MS1)|(1<<MS2);
        PORT_SILNIK &= ~(1<<MS3);
        break;
    case 5: //1/16 kroku
        PORT_SILNIK |= (1<<MS1)|(1<<MS2)|(1<<MS3);
        break;
    default:
        break;
    }
}

void Kierunek(unsigned char dir) // 0 - przód, 1-ty³
{
    if (dir == 0)
    PORT_SILNIK |= (1<<DIR); //ustaw
        else
        PORT_SILNIK &= ~(1<<DIR); //zeruj
}

void Krok(void) //1 kork
{
    PORT_SILNIK |= (1<<STEP);
    _delay_us(10);
    //_delay_ms(10); ///cele testowe na LED TYLKO!
    PORT_SILNIK &= ~(1<<STEP);
}


void Nast_tryb(void) // Na guziczek po pó³ sekundy prze³¹cz tryb
{
    praca++;
    if(praca>=5)
        praca=5;
    Rodzaj_pracy(praca);
}

void Poprz_tryb(void) // Na guziczek po pó³ sekundy prze³¹cz tryb
{
    praca--;
    if(praca<=1)
        praca=1;
    Rodzaj_pracy(praca);
}

void Zmien_kierunek(void) // na guziczek zmieñ kierunek obrotów
{
    //zaneguj tylko pierwszy bit
    //kier ^= (1<<0);
    kier = !kier;
    Kierunek(kier);
}

void Motor_init(void)
{
    PORT_SILNIK |= (1<<RESET)|(1<<SLEEP);//ustaw porty na 1
    PORT_SILNIK &= ~(1<<ENABLE);
}
