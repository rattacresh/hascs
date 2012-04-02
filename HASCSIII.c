/* HASCSIII module */

#include "HASCSSystem.h"
#include "Start.h"

int main(void)
{
	InitWorkstation(" HASCS III ");
	if (ReadConfig())
		StartGame();
	ExitWorkstation(0);
}

