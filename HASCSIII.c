/* HASCSIII module */

#include "compat.h"

#include "HASCSSystem.h"
#include "Start.h"

int __argc; char **__argv;

int main(int argc, char *argv[])
{
	__argc = argc; __argv = argv;

	InitWorkstation(" HASCS III ");
	if (ReadConfig())
		StartGame();
	ExitWorkstation(0);
	return 0;
}

