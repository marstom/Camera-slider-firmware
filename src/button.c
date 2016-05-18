/*
Obs�uga przycisk�w i joysticka
*/

#include "button.h"

double btn_delay=200; //czas op�nienia dla przycisk�w
bool button_f=0; //flaga dla przycisk�w
int button_count=0; //licznik przeskocze� dla przycisk�w
bool check_menu = 1; //je�eli 1 to znaczy �e nast�pi�a aktualizacja menu

//flagi przycisk�w
bool btn_plus=0;
bool btn_minus=0;
bool btn_enter=0;
bool btn_up=0;
bool btn_down=0;

void Btn_flags_erase(void)
{
    btn_enter=0;
    btn_plus=0;
    btn_minus=0;
    btn_up=0;
    btn_down=0;
}

void Button_Turbo(void)
{
        button_f=1;     //tryb turbo
        button_count++;
        switch(button_count)
        {
            case 0:
                btn_delay=200;
            break;
            case 5:
                btn_delay=100;
                break;
            case 10:
                btn_delay=50;
                break;
            case 30:
                btn_delay=20;
                break;
            case 100:
                btn_delay=1;
        }
        _delay_ms(btn_delay);
}

void Check_Button(void)
{
        if(UP){     //G�rny
            btn_up=1;
            check_menu=1;   //sygna� �e czas na od�wie�anie menu
            Button_Turbo();
        }
        if(DOWN){     //Dolny
            btn_down=1;
            check_menu=1;   //sygna� �e czas na od�wie�anie menu
            Button_Turbo();
        }

        if(RIGHT) //prawo
        {
            btn_plus=1;
            check_menu=1;
            Button_Turbo();
        }
        if(LEFT) //lewo
        {
            btn_minus=1;
            check_menu=1;
            Button_Turbo();
        }
        if(P5) // �rodkowy na padzie
        {
            btn_enter=1;
            check_menu=1;
            Button_Turbo();
        }

        if(button_f==1){ //do turbo. Resetowanie stanu turbo
         if(ALL_RELEASE){
            btn_delay=200;
            button_f=0;
            button_count=0;
         }
        }
}
