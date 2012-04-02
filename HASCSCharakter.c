/* HASCSCharakter module */

/* Erschaffung eines neuen Charakters */


int CreateCharakter(SpielerTyp *Spieler)
	unsinged mx, my, i, VP, TY;
	BITSET mb;
	unsigned B[6]; /* Basiswerte */
	char ch;
	int genus;

	void Erhoehe(unsigned i)
	{
		if (B[i] < 14 && VP > 0) {
			B[i]++; VP--;
			GotoXY(42, 9 + i); PrintCard(B[i], 2);
			PrintAt(22, 7, "#200#"); PrintCard(VP, 2);
		}
	}

	void Vermindere(unsigned i)
	{
		if (B[i] > 4) {
			B[i]--; VP++;
			GotoXY(42, 9 + i); PrintCard(B[i],2);
			PrintAt(22, 7, "#200#"); PrintCard(VP,2);
		}
	}

	for (i = 0; i <= 5; i++) { B[i] = 4; } VP = 30;

	ReserveScreen(8 ,2, 30, 18);

	TextMode = 1;
	PrintAt(11,  4, Spieler.Name);
	TextMode = 0;
	PrintAt(22,  7, "#200#"); PrintCard(VP, 2);
	PrintAt(22,  9, "#10#"); /* Basiseigenschaften */
	PrintAt(22, 10, "#11#");
	PrintAt(22, 11, "#12#");
	PrintAt(22, 12, "#13#");
	PrintAt(22, 13, "#14#");
	PrintAt(22, 14, "#15#");
	PrintAt(22, 16, "#201#"); DOutlineBar(11, 16, 27, 16);

	for (i = 0; i <= 5; i++) {
		PrintAt(42, 9 + i, " 4");
		SetSprite(25, 9 + i, SystemSprite[2]); /* + */
		SetSprite(27, 9 + i, SystemSprite[3]); /* - */
	}

	do {
		WaitInput(mx ,my , mb , ch, -1);
		mx = mx / 16; my = my / 16;
		if (my >= 9 && my <= 14 && (1<<0) & mb) {
			if (mx == 27) Vermindere(my - 9);
			else if (mx == 25) Erhoehe(my - 9);
		}
	} while (my != 16 || mx < 11 || mx > 27); /* Weiter... */

	RestoreScreen;
	ReserveScreen(8, 2, 30, 18);

	TextMode = 1;
		PrintAt( 11, 4, Spieler.Name);
	TextMode = 0;
	PrintAt( 22, 6, "#210#");
	PrintAt(25, 8, "#0#"); SetSprite(25, 8, SystemSprite[10]);
	PrintAt(25, 9, "#1#"); SetSprite(27, 9, SystemSprite[11]);
	PrintAt(25,10, "#2#"); SetSprite(25,10, SystemSprite[12]);
	PrintAt(25,11, "#3#"); SetSprite(27,11, SystemSprite[13]);
	PrintAt(25,12, "#4#"); SetSprite(25,12, SystemSprite[14]);
	PrintAt(25,14, "#212#"); PrintAt(41,14, "#213#");
	PrintAt(25,16, "#211#"); DOutlineBar(11, 16, 27, 16);

	TY = 0;
	SetSprite(11, 8, SystemSprite[7]); /* Pfeil */
	genus = TRUE;
	SetSprite(11,14, SystemSprite[7]);

	do {
		WaitInput(mx, my, mb, ch, -1);
		mx = mx / 16; my = my / 16;
		if (mx >= 11 && mx <= 27 && (1<<0) IN mb)
			if ((my >= 8 && my <= 12)) {
				SetSprite(11, 8 + TY, SystemSprite[0]);
				TY = my - 8;
				SetSprite(11, 8 + TY, SystemSprite[7]);
			}
		if (my == 14) {
			if (mx < 19) {
				SetSprite(19,14, SystemSprite[0]);
				SetSprite(11,14, SystemSprite[7]);
				genus = TRUE;
			} else {
				SetSprite(19,14, SystemSprite[7]);
				SetSprite(11,14, SystemSprite[0]);
				genus = FALSE;
			}
		}
	} while (my != 16 || mx < 11 || mx > 27); /* Weiter... */

	RestoreScreen;

	for (i = 1; i <= MaxRuck; i++) { Spieler.Rucksack[i].KennNummer = 0; }

	Spieler.St = B[0]; Spieler.Ge = B[1]; Spieler.Ko = B[2]; Spieler.In = B[3]; Spieler.Zt = B[4]; Spieler.Ch = B[5];
	Spieler.TPMax = Zufall(6) + Spieler.Ko;
	Spieler.TP = Spieler.TPMax;
	Spieler.Status = 0; Spieler.Permanent = STot|SAusruhen|SReitet|SMann;
	if (Spieler.genus) { Status |= SMann; }
	Spieler.Gold = 0;
	Spieler.Nahrung = 0;
	Spieler.AnzGegenstaende = 0;
	Spieler.LevelNumber = 1;
	Spieler.Lernen = 0;
	Spieler.Typ = 0; Spieler.Typ |= TY;
	Spieler.Sprite = 10+TY;
	Spieler.EP = 0; Spieler.Grad = 0; Spieler.EPnext = 100; Spieler.Moves = 0;
	Spieler.Ruestung.KennNummer = 0;
	Spieler.Ring.KennNummer = 0;
	Spieler.linkeHand.KennNummer = 0;
	Spieler.rechteHand.KennNummer = 0;
	for (i = 1; i <= MaxFlags; i++)
		Flags[i] = 0;
	OldLevels = 0;
	return TRUE;
}

