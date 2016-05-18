/*
Przetwornik AC
*/
#include <avr/io.h>

#define ADC_ENABLE 					ADCSRA |= (1<<ADEN)
#define ADC_DISABLE 				ADCSRA &= 0x7F
#define ADC_START_CONVERSION		ADCSRA |= (1<<ADSC)
#define CHANNEL_X                     PORT7 // z którego pinu czytaæ napiêcie
#define CHANNEL_Y                     PORT6 // z którego pinu czytaæ napiêcie

int read_X(void);
int read_Y(void);
void ADC_init(void);
int ADC_Read(void);
