/*Przetwornik AC*/

#include "ADC.h"

//zmienne globalne
int c;
char c_l, c_h;
unsigned char read; // opcja

void ADC_init(void)
{
	ADCSRA=0x00;//disable ADC
	ADMUX  = 0x40;  //select adc input 0, ref:AVCC
	ADCSRA = 0x82; //prescaler:4, single conversion mode
	ADC_ENABLE; //adc enable
}

/************************************
Odczytuje wartoœæ z przetwornika AC
***********************************/
int ADC_Read(void)
{
	char i;
    int ADC_temp, ADCH_temp;
    int ADC_var = 0;


    for(i=0;i<8;i++)             // do the ADC conversion 8 times for better accuracy
    {
	 	ADC_START_CONVERSION;
        while(!(ADCSRA & 0x10)); // wait for conversion done, ADIF flag active
        ADCSRA|=(1<<ADIF);

        ADC_temp = ADCL;         // read out ADCL register
        ADCH_temp = ADCH;        // read out ADCH register
		ADC_temp +=(ADCH_temp << 8);
        ADC_var += ADC_temp;      // accumulate result (8 samples) for later averaging
    }

    ADC_var = ADC_var >> 3;       // average the 8 samples

	if(ADC_var > 1000) ADC_var = 1000;

    return ADC_var;
}

int read_X(void) //kontroler 0..-500*500..0
{
    ADMUX = 0x40 | CHANNEL_X;
    c=ADC_Read();
    c=c-500;


    if(c>=0)
        c=500-c+1; //+1 k³opot z zerem
    if (c<0)
        c=(500+c+1)*(-1);

    if (c>=0 && c<11) //limity górne i dolne, coby siê silnik nie rozpêdzi³ za mocno:P
        c=10;
    if (c<=0 && c>-11)
        c=-10;
    return c;
}

int read_Y(void) //kontroler 0..-500*500..0
{
    ADMUX = 0x40 | CHANNEL_Y;
    c=ADC_Read();
    c=c-500;


    if(c>=0)
        c=500-c+1;
    if (c<0)
        c=(500+c+1)*(-1);

    if (c>=0 && c<11) //limity górne i dolne, coby siê silnik nie rozpêdzi³ za mocno:P
        c=10;
    if (c<=0 && c>-11)
        c=-10;
    return c;
}
