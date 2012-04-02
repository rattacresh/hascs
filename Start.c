//IMPLEMENTATION MODULE Start;
#include "Start.h"


unsigned Auswahl, SpriteAnzahl, d, s;
int Ok, OnceMore;


int ReadConfig()
	char Path[129];

	void MakeAbsolute(char *p)
	{
		if (p[1] == ':') return;
		Concat(p, ActPath, p);
	}

	ShowError = FALSE;
	do {
		if (Length(Command) == 0)
			Assign(Command, "HASCS.CFG");
		DiaPath = "DIALOG\\";
		MapPath = "MAP\\";
		PlaPath = "SAVE\\";
		PrgPath = "SYSTEM\\";
		SoundPath = "SOUND\\";
		TextEditor = "EDITOR.PRG";
		IniFile = "HASCS.INI";
		if (LoadDialog(ConfigDialog, FALSE)) {
			ExecuteDialog();
			MakeAbsolute(DiaPath);
			MakeAbsolute(MapPath);
			MakeAbsolute(PlaPath);
			MakeAbsolute(PrgPath);
			MakeAbsolute(SoundPath);
		}
		LoadLevel(1);
		if (FileError) {
			if (!SelectFile("Welches Abenteuer...", "*.CFG", Command))
				return FALSE;
		}
	} while (FileError);
	ShowError = TRUE;
	return TRUE;
}


unsigned Anfangsmaske(void)
{
	unsigned d, x, wahl;
	NewScreen(8, 3, 24, 17, " HASCS III ");
	d = AddObject(2, 2, 20, 1, "#100#", BigText);
	d = AddObject(2, 3, 20, 1, "#101#", BigText);
	d = AddObject(2, 4, 20, 1, "#102#", BigText);
	d = AddObject(2, 2, 20, 1, "#103#", 0);
	d = AddObject(2, 3, 20, 1, "#104#", 0);
	d = AddObject(2, 4, 20, 1, "#105#", 0);
	d = AddObject(2, 6, 20, 1, "353", SpriteFill);
	x = AddObject(2, 8, 20, 1, "#111#", BigText|Outlined|Exit);
	d = AddObject(2,10, 20, 1, "#112#", BigText|Outlined|Exit);
	d = AddObject(2,12, 20, 1, "#113#", BigText|Outlined|Exit);
	d = AddObject(2,14, 20, 1, "#114#", BigText|Outlined|Exit);

	DrawScreen();
	wahl = HandleScreen();
	RestoreScreen();
	
	return wahl + 1 - x;
}


void PlayTheGame(unsigned Wie)
{
	unsigned i;
	int Ende;

	for (i = 1; i <= 7; i++)
		OutputText(" "); /* Textzeilen löschen */
	OutputText("#109#");

	LoadLevel(Spieler.LevelNumber);
	if (Wie == 1 || Wie == 3) {
		Sichtweite = SetLightRange();
		Spieler.x = Spieler.LevelBreite / 2;
		Spieler.y = Spieler.LevelHoehe / 2;
		DoDialog(0); /* Anfangsdialog */
		LoadOrSavePlayer(FALSE); /* Anfangsabspeichern */
	}
	PrintCharakter(0);

	do {
		Ende = TRUE;
		switch (DoGame()) {
		case 1 : /* draufgegangen */
			DoDialog(999); /* Todesdialog */
			Ende = STot & Spieler.Status;
			break;
		case 3 : /* geschafft ! */
			DoDialog(998);
			DeleteLevels;
			LoadOrSavePlayer(FALSE);
			break;
		}
	} while (!Ende);
}


void StartGame();
{

	Editor = FALSE;
	DruckerAusgabe = FALSE;
	DebugMode = FALSE;
	SoundAusgabe = TRUE;
	MaxX = 78; /* Ausgabebreite */

	DoDialog(IniDialog); /* Texte, Anfangsbild etc.*/

	Spieler.x = LevelBreite / 2;
	Spieler.y = LevelHoehe / 2;
	Spieler.Sichtweite = SetLightRange();
	Spieler.Typ = 1<<0; Spieler.Sprite = 10;

	MakeSichtBereich(TRUE);
	RestoreScreen();

	do {
		OutputText("                            ");
		OutputText("                            ");
		OutputText("          HASCS III         ");
		OutputText("         Version 1.43       ");
		OutputText("                            ");
		OutputText("    Copyright © 1987-1995   ");
		OutputText("      Alexander Kirchner    ");
		OutputText("                            ");
		
		Auswahl = Anfangsmaske();

		if (Auswahl <= 3) {
			NewScreen(8, 7, 23, 7, "");
			d = AddObject(2, 2, 19, 1, "#106#", BigText);
			s = AddObject(2, 4, 19, 1, "", {Editable, Outlined, BigText});
			SetInputString(s, Spieler.Name);
			DrawScreen(); d = HandleScreen(); RestoreScreen();
			GetInputString(s, Spieler.Name);
			Ok = FALSE;
			if (Compare(Spieler.Name, ""))
				Auswahl = 0;

			if (Auswahl == 1) { 
				Ok = CreateCharakter(Spieler);
				if (Ok)
					DeleteLevels();
			}

			if (Auswahl == 2) {
				LoadOrSavePlayer(TRUE);
				Ok = !FileError;
				if (Spieler.LevelNumber == 0 || Spieler.LevelNumber > 999)
					Auswahl = 3;
			}

			if (Auswahl == 3) {
				LoadOldPlayer();
				Ok = !FileError;
				if (Ok)
					DeleteLevels();
			}

			if (Ok) {
				DebugMode = BenutzerNummer == StringToCard(Spieler.Name);
				PlayTheGame(Auswahl);
			}

		} /* if (Auswahl... */

	} while (Auswahl != 4);

}
