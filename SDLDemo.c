#include <stdio.h>
#include <SDL/SDL.h>
#include "HASCSSystem.h"
#include "HASCSGraphics.h"

int main(void)
{
	InitWorkstation("HASCSSystem-Test");
	GraphicsInit();

	DOutlineBar(27, 10, 38, 11);
	HorzLine(40, 200, 590);
	VertLine(140, 140, 100);
	InvertFeld(10,10);

	HorzLine(12, 202, 28);
	//	HorzLine(2, 12, 28);

	
	WaitTime(0); // Redraw screen

	unsigned x;
	unsigned y; 
	BITSET s; 
	char ch;
	WaitInput(&x, &y, &s, &ch, -1);


	ExitWorkstation(0);

	return 0;
}
