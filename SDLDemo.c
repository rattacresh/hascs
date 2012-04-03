#include <stdio.h>
#include "HASCSSystem.h"

unsigned char feld[100];

void ffe(unsigned char *manip) {
	manip[22] = 122;
}

int main(void)
{
	InitWorkstation("HASCSSystem-Test");
	//int i; scanf ("%d",&i);
	ExitWorkstation(0);
	feld[22] = 55;
	ffe(feld);
	printf("Hier: %i\n",feld[22]);
	return 0;
}
