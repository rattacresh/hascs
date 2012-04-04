#include <stdio.h>
#include "HASCSSystem.h"
#include "HASCSGraphics.h"

void TestPaint() {
	DOutlineBar(27, 10, 38, 11);
	HorzLine(40, 200, 590);
	VertLine(140, 140, 100);
	InvertFeld(8,12);
	InvertFeld(10,12);
	InvertFeld(12,12);
	InvertFeld(14,12);
}

void TestAllocate() {
	int GAmount = 10;
	char *meinRAM = Allocate(GAmount + 1);
	if (!meinRAM)
		printf("Fehler: allozieren hat nicht geklappt!\n");
	int i;
	for (i=0; i<GAmount; ++i) 
		meinRAM[i] = 'G';
	meinRAM[GAmount] = '\0';
	printf("String im allozierten RAM: <%s>\n", meinRAM);
	printf("(Es sollten %i Buchstaben G zu sehen sein.)\n", GAmount);

	printf("meinRAM-adr vor dealloc: %p\n", meinRAM);
	Deallocate((void**)&meinRAM);
	printf("meinRAM-adr nach dealloc (soll nil sein): %p\n", meinRAM);
	if (meinRAM)
		printf("FEHLER: meinRAM wurde nicht dealloziert!\n");
}

void TestWaitInput() {
	unsigned x;
	unsigned y; 
	BITSET s; 
	char ch;
	WaitInput(&x, &y, &s, &ch, -1);

	printf("\nHASCS-Maustaste: %u\n", s);
	printf("(0 ist links und 1 ist rechts)\n\n");
	printf("HASCS-Taste: <%c> (ASCII dezimal %u)\n", ch, ch);
}

void TestError() {
	printf("ShowError: %i\n", ShowError);
	printf("Folgende Dummy-Fehlermeldung soll nur bei ShowError==1 angezeigt werden:\n");
	Error("***Dummy-Fehlermeldung*** (die zu keinem Programmabbruch führt, da mode 1)", 1);
}


int main(void)
{
	printf("Test: InitWorkstation...\n");
	InitWorkstation("HASCSSystem-Test");
	printf("OK\n\n");

	printf("Test: Zeichenfunktionen...\n");
	TestPaint();
	printf("OK\n\n");
	
	printf("Test: LoadAndRun...\n");
	LoadAndRun("df -h /", "");
	printf("OK\n\n");

	printf("Test: Allocate und Deallocate...\n");
	TestAllocate();
	printf("OK\n\n");

	printf("Test: WaitInput...\n");
	TestWaitInput();
	printf("OK\n\n");

	printf("Test: Error...\n");
	TestError();
	printf("OK\n\n");


	ExitWorkstation(0);

	return 0;
}
