IMPLEMENTATION MODULE HASCSSprite;
				
SpriteType aSprite;
SpriteType SpriteBuffer[9];
String60Typ SpriteName;
String60Typ SpriteDatei[10];
unsigned Index;
int Changes;

void PaintSprite(SpriteType Sprite)
/* malt Sprite vergrößert links oben */
{
	unsigned x,y,c;
	for (y = 0; y <= 15; y++)
		for (x = 0; x <= 15; x++)
			if ((1 << (15-x)) & Sprite[y])
				SetSprite(x, y, SystemSprite[1]);
			else
				SetSprite(x, y, SystemSprite[4]);
}

void PaintDemoSprites(void)
/* malt 9 mal das aktuelle Sprite */
{
	unsigned x, y;
	for (x = 0; x <= 2; x++)
		for (y = 0; y <= 2; y++)
			SetSprite(x+17, y+11, aSprite);
}

void PaintBufferSprites(void)
{
	unsigned x, y;

	for (x = 0; x <= 2; x++)
		for (y = 0; y <= 2; y++)
			SetSprite(x+22,y+11,SpriteBuffer[3*x+y]);
}

void PrintAllSprites(SpriteArrayType *s)
/* malt alle Sprites eines Feldes in die untersten 4 Zeilen */
{
	unsigned x;
	for (x = 0; x <= MaxSprites - 1; x++)
		SetSprite(x * 40, 17 + x / 40, s[x]);
}


void MakeScreen(unsigned SpriteArray)
{
/* Erzeugt Arbeitsbildschirm */

	void PrintMenue(void)
	{
		unsigned i;
		PrintAt(54,0,"    Oben");   OutlineBar(27, 0, 32, 0);
		PrintAt(54,1,"   Unten ");  OutlineBar(27, 1, 32, 1);
		PrintAt(54,2,"   Links");   OutlineBar(27, 2, 32, 2);
		PrintAt(54,3,"   Rechts");  OutlineBar(27, 3, 32, 3);
		PrintAt(54,5,"  Spiegeln"); OutlineBar(27, 5, 32, 5);
		PrintAt(54,6,"  Rotieren"); OutlineBar(27, 6, 32, 6);
		PrintAt(54,7,"   Füllen");  OutlineBar(27, 7, 32, 7);
		PrintAt(54,8,"   Farbe");   OutlineBar(27, 8, 32, 8);

		FillRectangle(34, SpriteArray+5, 39, SpriteArray+5, SystemSprite[0]);
		PrintAt(68,0,"    ENDE");   OutlineBar(34, 0, 39, 0);
		PrintAt(68,2,"   Laden");   OutlineBar(34, 2, 39, 2);
		PrintAt(68,3," Speichern"); OutlineBar(34, 3, 39, 3);
		PrintAt(68,5,"   Felder");  OutlineBar(34, 5, 39, 5);
		PrintAt(68,6,"   Monster"); OutlineBar(34, 6, 39, 6);
		PrintAt(68,7,"   System");  OutlineBar(34, 7, 39, 7);
		PrintAt(68,8,"   Zeichen"); OutlineBar(34, 8, 39, 8);
		for (i = 34; i <= 39; i++)
			InvertFeld(i, SpriteArray+5);

		PrintAt(55,10,"Dateiname:");
		FillRectangle(27, 11, 38, 11, SystemSprite[0]);
		PrintAt(55,11,SpriteDatei[Index]);
		OutlineBar(27, 10, 38, 11);

		for (i = 0; i <= 9; i++) {
			FillRectangle(17, i, 24, i, SystemSprite[0]);
			PrintAt(36, i, SpriteDatei[i]);
			if (i == Index && Changes)
				PrintAt(35, i, "*");
			OutlineBar(17, i, 24, i);
		}
		for (i = 0; i <= 7; i++)
			InvertFeld(17+i, Index);
	}

	void CopyRight(void)
	{
		PrintAt(15,22,"      HASCS III - Sprite Editor Version 1.00");
		PrintAt(15,23,"     Copyright © 1987-1993 Alexander Kirchner");
	}


	CopyRight();
	PaintSprite(aSprite);
	PaintDemoSprites();
	PaintBufferSprites();
	if (SpriteArray == 0)
		PrintAllSprites(FelderSprite);
	else if (SpriteArray == 1)
		PrintAllSprites(MonsterSprite);
	else if (SpriteArray == 2)
		PrintAllSprites(SystemSprite);
	else
		PrintAllSprites(GegenSprite);
	PrintMenue();

}

/* Spritebearbeitung */

void RollUp(SpriteType *Sprite)
{
	unsigned x, y;
	unsigned c[16];

	for (x = 0; x <= 15; x++)
		c[x] = GetSprite(Sprite, x, 0);
	for (y = 0; y <= 14; y++)
		for (x = 0; x <= 15; x++)
			EditSprite(Sprite, x, y, GetSprite(Sprite, x, y+1));

	for (x = 0; x <= 15; x++)
		EditSprite(Sprite, x, 15, c[x]);
}


void RollDown(SpriteType *Sprite)
{
	unsigned x, y;
	unsigned c[16];

	for (x = 0; x <= 15; x++)
		c[x] = GetSprite(Sprite, x, 15);
	for (y = 15;  y >= 1; y--)
		for (x = 0; x <= 15; x++)
			EditSprite(Sprite, x, y, GetSprite(Sprite, x, y-1));
	for (x = 0; x <= 15; x++)
		EditSprite(Sprite, x, 0, c[x]);
}


void RollRight(SpriteType *Sprite)
{

	unsigned x, y;
	unsigned c[16];

	for (y = 0; y <= 15; y++)
		c[y] = GetSprite(Sprite, 15, y);
	for (x = 15; x >= 1; x--)
		for (y = 0; y <= 15; y++)
			EditSprite(Sprite, x, y, GetSprite(Sprite, x-1, y));
	for (y = 0; y <= 15; y++)
		EditSprite(Sprite, 0, y, c[y]);
}


void RollLeft(SpriteType *Sprite)
{
	unsigned x, y;
	unsigned c[16];

	for (y = 0; y <= 15; y++)
		c[y] = GetSprite(Sprite, 0, y);
	for (x = 0; x <= 14; x++)
		for (y = 0; y <= 15; y++)
			EditSprite(Sprite, x, y, GetSprite(Sprite, x+1, y));
	for (y = 0; y <= 15; y++)
		EditSprite(Sprite, 15, y, c[y]);
}

void Horizontal(SpriteType *Sprite)
{
	unsigned x, y, c1, c2;
	for (y = 0; y <= 15; y++)
		for (x = 0; x <= 7; x++) {
			c1 = GetSprite(Sprite, x, y);
			c2 = GetSprite(Sprite, 15-x, y);
			EditSprite(Sprite, x, y, c2);
			EditSprite(Sprite, 15-x, y, c1);
		}
}

void Vertikal(SpriteType *Sprite)
{
	unsigned x, y, c1, c2;

	for (y = 0; y <= 7; y++)
		for (x = 0; x <= 15; x++) {
			c1 = GetSprite(Sprite, x, y);
			c2 = GetSprite(Sprite, x, 15-y);
			EditSprite(Sprite, x, y, c2);
			EditSprite(Sprite, x, 15-y, c1);
		}
}

void RotateRight(SpriteType *Sprite)
{
	unsigned x, y; SpriteType h;
	for (x = 0; x <= 15; x++)
		for (y = 0; y <= 15; y++)
			EditSprite(h, 15-y, x, GetSprite(Sprite, x, y));
	Sprite = h;
}

void Fill(SpriteType *Sprite, unsigned Farbe)
{
	/* Füllt Sprite mit Farbe */

	unsigned x, y, a;

	for (y = 0; y <= 15; y++)
		for (x = 0; x <= 15; x++)
			EditSprite(Sprite, x, y, Farbe);
}

void ChangeColors(SpriteType *Sprite, unsigned OldColor, unsigned NewColor);
{
	/* Wechselt Farbe aus */

	unsigned x, y;

	for (x = 0; x <= 15; x++)
		for (y = 0; y <= 15; y++)
			if (GetSprite(Sprite, x, y) == OldColor)
				EditSprite(Sprite, x, y, NewColor);
			else if (GetSprite(Sprite, x, y) == NewColor)
				EditSprite(Sprite, x, y, OldColor);
}

/* editieren Sie jetzt... */

void DoEdit(void)
{
	unsigned Farbe, Farbe2, MausX, MausY, SpriteArray, SpriteNummer;
	BITSET MausButton;
	int Weiter;
	char Taste;
				
	void MenueWahl(unsigned Spalte, unsigned Zeile)
	{
		unsigned i;
		if (Spalte == 1) {
			switch (Zeile) {
			case 0 : Weiter = FALSE;
			case 2 : if (SpriteDatei[Index, 0] != '\0') {
					LoadOrSaveSprites(TRUE, SpriteDatei[Index]);
					Changes = FALSE;
					MakeScreen(SpriteArray);
				}
				break;
			case 3 : if (SpriteDatei[Index,0] != '\0') {
					LoadOrSaveSprites(FALSE, SpriteDatei[Index]);
					Changes = FALSE;
					MakeScreen(SpriteArray);
				}
				break;
			case 5...8 : for (i = 34; i <= 39; i++) {
					InvertFeld(i, SpriteArray+5);
					InvertFeld(i, Zeile);
				}
				SpriteArray = Zeile - 5;
				if (SpriteArray == 0) {
					PrintAllSprites(FelderSprite);
				} else if (SpriteArray == 1) {
					PrintAllSprites(MonsterSprite);
				} else if (SpriteArray == 2) {
					PrintAllSprites(SystemSprite);
				} else {
					PrintAllSprites(GegenSprite);
				}
				break;
			case 10... 11 : GotoXY(60,11);
				InputString(SpriteDatei[Index], 18);
				MakeScreen(SpriteArray);
			}
		} else if (Spalte == 0) {
			switch (Zeile) {
			case 0 : RollUp(aSprite); break;
			case 1 : RollDown(aSprite); break;
			case 2 : RollLeft(aSprite); break;
			case 3 : RollRight(aSprite); break;
			case 5 : if (MausLinks & MausButton) {
					Horizontal(aSprite);
				} else {
					Vertikal(aSprite);
				}
				break;
			case 6 : RotateRight(aSprite);
			case 7 : if (MausLinks & MausButton) {
					Fill(aSprite, Farbe);
				} else if (MausRechts & MausButton) {
					Fill(aSprite, Farbe2);
				}
				break;
			case 8 : ChangeColors(aSprite, Farbe, Farbe2); break;
			case 10,11 : GotoXY(55,11);
				InputString(SpriteDatei[Index],14);
				MakeScreen(SpriteArray);
				break;
			}
			PaintSprite(aSprite);
			PaintDemoSprites;
		}
	}

	void DateiWahl(unsigned i)
	{
		unsigned j;
		if (Changes && SpriteDatei[Index][0] != '\0')
			LoadOrSaveSprites(FALSE, SpriteDatei[Index]);
		Changes = FALSE;
		Index = i;
		if (SpriteDatei[Index][0] != '\0')
			LoadOrSaveSprites(TRUE, SpriteDatei[Index]);
		MakeScreen(SpriteArray);
	}


	void SelectSprite(unsigned n)

	{
		switch (SpriteArray) {
		case 0 : aSprite = FelderSprite[n]; break;
		case 1 : aSprite = MonsterSprite[n]; break;
		case 2 : aSprite = SystemSprite[n]; break;
		case 3 : aSprite = GegenSprite[n]; break;
		}
		PaintSprite(aSprite);
		PaintDemoSprites;
		PrintAt(34, 15, "Nummer: "); PrintCard(n, 3);
	}

	void CopyBack(unsigned n)

	{
		switch (SpriteArray) {
		case 0 : FelderSprite[n] = aSprite; break;
		case 1 : MonsterSprite[n] = aSprite; break;
		case 2 : SystemSprite[n] = aSprite; break;
		case 3 : GegenSprite[n] = aSprite; break;
		}
		SetSprite(n * 40, 17 + n / 40, aSprite);
		if (!Changes) {
			Changes = TRUE;
			MakeScreen(SpriteArray);
		}
		PrintAt(34, 15, "Nummer: "); PrintCard(n, 3);
	}

	Farbe = 1; Farbe2 = 0;
	SpriteNummer = 0;
	SpriteArray = 0;
	Weiter = TRUE;

	while (Weiter) {
		WaitInput(MausX, MausY, MausButton, Taste, -1);
		MausX = MausX / 16; MausY = MausY / 16;
		if (MausX >= 27 && MausX <= 32 && MausY <= 8)
			MenueWahl(0, MausY);
		else if (MausX >= 34 && MausY <= 39 && MausY <= 8)
			MenueWahl(1, MausY);
		else if (MausX >= 18 && MausX <= 25 && MausY <= 9)
			DateiWahl(MausY);
		else if (MausX > 27 && MausX <= 38
		      && MausY >= 10 && MausY <= 13)
			MenueWahl(0, MausY);
		if (MausLinks & MausButton) {
			if ((MausX < 16 && MausY < 16)) {
				SetSprite(MausX, MausY, SystemSprite[Farbe]);
				EditSprite(aSprite, MausX, MausY, Farbe);
				PaintDemoSprites();
			} else if ((MausY >= 17 && MausY <= 20))
				SelectSprite(MausX + (MausY - 17) * 40);
			else if (MausX >= 22 && MausX <= 24
			      && MausY >= 11 && MausY <= 13)
			{
				aSprite = SpriteBuffer[(MausX - 22) * 3 + (MausY - 11)];
				PaintSprite(aSprite);
				PaintDemoSprites();
			}
		} else if (MausRechts & MausButton) {
			if ((MausY >= 17 && MausY <= 20)) {
				CopyBack(MausX + (MausY - 17) * 40);
			} else if ((MausX < 16 && MausY < 16)) {
				SetSprite(MausX, MausY, SystemSprite[4]);
				EditSprite(aSprite, MausX, MausY, Farbe2);
				PaintDemoSprites();
			} else if (MausX >= 22 && MausX <= 24
			        && MausY >= 11 && MausY <= 13)
			{
				SpriteBuffer[(MausX - 22) * 3 + (MausY - 11)] = aSprite;
				PaintBufferSprites();
			}
		}
	} /* while (Weiter) { */

}


void SpriteEdit(char *s)
{
	unsigned y, LoadedSprites;
	y = 0;
	if (FileName("DUMMY.DUM", SpriteName)) { } /* Dummy Aufruf */
	Concat(SpriteName, PrgPath, "*.SPR");
	while (y < 10 && FileName(SpriteName, SpriteDatei[y]))
		y++;

	while (y < 10) {
		SpriteDatei[y] = "";
		y++;
	}

	Concat(SpriteName, s, ".SPR");

	Index = 0;
	for (y = 0; y <= 9; y++)
		if (Compare(SpriteName, SpriteDatei[y])
		 && !Compare(SpriteName, ""))
			Index = y;

	LoadedSprites = Index;
	Changes = FALSE;

	for (y = 0; y <= 15; y++)
		aSprite [y] = {};

	for (y = 0; y <= 8; y++)
		SpriteBuffer[y] = SystemSprite[4];

	FillRectangle(0, 0, 39, 24, SystemSprite[0]);
	MakeScreen(0);

	DoEdit();

	if (Index != LoadedSprites)
		LoadOrSaveSprites(TRUE, SpriteDatei[LoadedSprites]);
}
