/* HASCSIII module */

int main(void)
{
	InitWorkstation(" HASCS III ");
	if (ReadConfig())
		StartGame();
	ExitWorkstation(0);
}

