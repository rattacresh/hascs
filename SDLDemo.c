#include <stdio.h>
#include "HASCSSystem.h"
#include "HASCSGraphics.h"

int main(void)
{
	InitWorkstation("HASCSSystem-Test");

	DOutlineBar(27, 10, 38, 11);
	HorzLine(40, 200, 590);
	VertLine(140, 140, 100);
	InvertFeld(8,12);
	InvertFeld(10,12);
	InvertFeld(12,12);
	InvertFeld(14,12);

	unsigned x;
	unsigned y; 
	BITSET s; 
	char ch;
	WaitInput(&x, &y, &s, &ch, -1);

	printf("\nHASCS-Maustaste: %u\n", s);
	printf("(0 ist links und 1 ist rechts)\n\n");
	printf("HASCS-Taste: <%c> (ASCII dezimal %u)\n", ch, ch);


	ExitWorkstation(0);

	return 0;
}
