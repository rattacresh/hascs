/* HASCSIII module */

#include "HASCSSystem.h"
#include "Start.h"

int main(int argc, char *argv[])
{
	extern int __argc; extern char **__argv; __argc = argc; __argv = argv;

	InitWorkstation(" HASCS III ");
	if (ReadConfig())
		StartGame();
	ExitWorkstation(0);
	return 0;
}

