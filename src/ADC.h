/*
Przetwornik AC
*/
#include <avr/io.h>

#define ADC_ENABLE 					ADCSRA |= (1<<ADEN)
#define ADC_DISABLE 				ADCSRA &= 0x7F
#define ADC_START_CONVERSION		ADCSRA |= (1<<ADSC)
#define CHANNEL_X                     PORT7 // z kt�rego pinu czyta� napi�cie
#define CHANNEL_Y                     PORT6 // z kt�rego pinu czyta� napi�cie

int read_X(void);
int read_Y(void);
void ADC_init(void);
int ADC_Read(void);
