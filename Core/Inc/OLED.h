#ifndef OLED_H
#define OLED_H
#include "main.h"

typedef struct MenuElement {
	const char *title;
	struct MenuElement *parent;
	struct MenuElement *child;
	struct MenuElement *up;
	struct MenuElement *down;
	void (*render)(void);
} MenuElement_t;

extern MenuElement_t Display_main, Display_1, Display_2, Display_3, Display_4,
		Display_5;
extern MenuElement_t Data_main, Data_1, Data_2, Data_3;
extern MenuElement_t Diagnostics_main, Diagnostics_1, Diagnostics_2, Diagnostics_3,
		Diagnostics_4;

extern MenuElement_t Warn_UnsafeDriving, Warn_Proximity, Warn_Light, Warn_Temp,
		Warn_Impact;

void Menu_Init();

void init_OLED();

void testOLED();


void str_toggleS3(uint8_t editing);


#endif
