/*
Obs³uga przycisków
*/
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include "ADC.h"

//#define P1 !(PIND & _BV(0)) //gora
//#define P2 !(PIND & _BV(1)) //dol
//#define P3 !(PIND & _BV(2)) //>
//#define P4 !(PIND & _BV(3)) //<
#define P5 !(PIND & _BV(2)) //ENTER
#define ALL_RELEASE !((read_X()<20 && read_X()>0)||(read_X()>-20 && read_X()<0)||(read_Y()<20 && read_Y()>0)||(read_Y()>-20 && read_Y()<0))
#define LEFT read_Y()<20 && read_Y()>0
#define RIGHT read_Y()>-20 && read_Y()<0
#define UP read_X()>-20 && read_X()<0
#define DOWN read_X()<20 && read_X()>0

void Btn_flags_erase(void);
//void Button_Turbo(void);
void Check_Button(void);
