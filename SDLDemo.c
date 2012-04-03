#include <stdio.h>
#include "HASCSSystem.h"

int main(void)
{
	InitWorkstation("HASCSSystem-Test");

	WaitKey();

	ExitWorkstation(0);

	return 0;
}
