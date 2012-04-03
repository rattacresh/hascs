#include <stdio.h>
#include "HASCSSystem.h"
#include "HASCSGraphics.h"

int main(void)
{
	InitWorkstation("HASCSSystem-Test");
	GraphicsInit();

	//OutlineBar(27, 10, 38, 11);
	//HorzLine(0, 200, 600);
	//WaitTime(0);

	WaitKey();

	ExitWorkstation(0);

	return 0;
}
