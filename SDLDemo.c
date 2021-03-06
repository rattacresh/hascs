#include <stdio.h>
#include "compat.h"

#include "HASCSSystem.h"
#include "HASCSGraphics.h"

static void TestPaint()
{
	DOutlineBar(27, 10, 38, 11);
#if 0
	/* private Funktionen von HASCSGraphics, */
	HorzLine(40, 200, 590);
	VertLine(140, 140, 100);
#endif
	InvertFeld(8,12);
	InvertFeld(10,12);
	InvertFeld(12,12);
	InvertFeld(14,12);
}

static void TestAllocate()
{
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

static void TestWaitInput()
{
	unsigned x;
	unsigned y; 
	BITSET s; 
	char ch;
	WaitInput(&x, &y, &s, &ch, -1);

	printf("\nHASCS-Maustaste: %u\n", s);
	printf("(0 ist links und 1 ist rechts)\n\n");
	printf("HASCS-Taste: <%c> (ASCII dezimal %u)\n", ch, ch);
}

static void TestError()
{
	printf("ShowError: %i\n", ShowError);
	printf("Folgende Dummy-Fehlermeldung soll nur bei ShowError==1 angezeigt werden:\n");
	Error("***Dummy-Fehlermeldung*** (die zu keinem Programmabbruch f�hrt, da mode 1)", 1);
}

static void TestBuffer()
{
	char *myBuf = GetBuffer(100);
	int i;
	for (i=0; i<7; ++i) 
		myBuf[i] = 'G';
	myBuf[7] = '\0';
	char *otherBufPtr = GetBuffer(10);
	printf("Die BufferAdr sollten gleich sein.\n");
	printf("Hier sollten 7 Gs zu sehen sein: %s\n", otherBufPtr);
	printf("(Wenn ein kleinerer Puffer geholt wird, dann wird der alte nicht gel�scht.)\n");
}

static void TestCaches() {
	printf("R�ckgabe NewCache(7, 1024): %u\n\n", NewCache(7, 1024));
	printf("R�ckgabe NewCache(0, 2666): %u\n\n", NewCache(0, 2666));
	printf("R�ckgabe GetCache(7): %u\n\n", GetCache(7));
	printf("R�ckgabe GetCache(0): %u\n\n", GetCache(0));
	printf("R�ckgabe GetCache(3): %u\n\n", GetCache(3));
	printf("L�sche Cache mit Index 1 (Cache mit id 7):\nFreeCache(1)\n\n"); FreeCache(1);
	printf("R�ckgabe GetCache(7): %u\n", GetCache(7));
}

static void TestFileFunc()
{
	printf("Gr��e HASCSMonster.c: %lu\n", FileLength("HASCSMonster.c"));
	printf("Gr��e NOTAFILE:       %lu\n", FileLength("NOTAFILE"));
	
	int dummyfile = CreateFile("einDummyFile");
	if (!dummyfile)
		printf("FEHLER: Habe keinen Filehandler bekommen!\n");
	else
		printf("Habe die Datei einDummyFile angelegt:\n");
	LoadAndRun("ls -l einDummyFile", "");
	DeleteFile("einDummyFile");
	printf("Habe einDummyFile gel�scht:\n");
	LoadAndRun("ls -l einDummyFile", "");
}

int __argc; char **__argv;

int main(int argc, char *argv[])
{
	__argc = argc; __argv = argv;

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

	printf("Test: GetBuffer...\n");
	TestBuffer();
	printf("OK\n\n");

	printf("Test: Caches...\n");
	TestCaches();
	printf("OK\n\n");

	printf("Test: File-Funktionen...\n");
	TestFileFunc();
	printf("OK\n\n");

	ExitWorkstation(0);

	return 0;
}
