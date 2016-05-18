/*

*/
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define PORT_SILNIK PORTB
//def wejœæ sterownika
#define ENABLE PORT0
#define MS1 PORT1
#define MS2 PORT2
#define MS3 PORT3
#define RESET PORT4
#define SLEEP PORT5
#define STEP PORT6
#define DIR PORT7

void Motor_init(void);
void Rodzaj_pracy(unsigned char tryb);
void Kierunek(unsigned char dir);
void Krok(void);
void Nast_tryb(void); // Na guziczek po pó³ sekundy prze³¹cz tryb
void Poprz_tryb(void); // Na guziczek po pó³ sekundy prze³¹cz tryb
void Zmien_kierunek(void); // na guziczek zmieñ kierunek obrotów
