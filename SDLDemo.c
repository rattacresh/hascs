#include <stdio.h>
#include "HASCSSystem.h"
#include "HASCSGraphics.h"

int main(void)
{
	InitWorkstation("HASCSSystem-Test");
	GraphicsInit();

	OutlineBar(27, 10, 38, 11);
	HorzLine(40, 200, 600);
	VertLine(140, 140, 100);
	InvertFeld(10,10);
	
	WaitTime(0); // Redraw screen
	WaitKey();

	ExitWorkstation(0);

	return 0;
}
