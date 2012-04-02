#include "HASCSEditor.h" /* HASCSEditor module */

/* HASCS EDITOR III

   written by Alexander Kirchner

   Version  1.00  30.08.89
            1.01  25.10.89
            1.02  05.12.89
            1.10  24.01.90
            1.11  03.02.90
            1.12  08.02.90
            1.13  11.02.90 Monstervorgaben
            1.14  14.03.90 Ausschnitt f�llen
            1.15  27.03.90 Spritenummer
            1.20  23.04.90 Gegenstandsvorgaben
            1.25  14.05.90 Dialognummer bei Gegenst�nden
            1.30  26.05.90 Block f�llen
            1.40  11.06.90 Benutzernummer, Levelformat, Levelsprites
            1.42  30.06.90 �nderbare Benutzernummer
            1.43  20.09.90 Monsterstatus Parameterfeld
            1.50  04.01.91 Ladbare Leveldatei, Level 1 Automatikladen
            1.60  13.09.91 Designverbesserungen
            1.65  22.07.92 Konfig Datei
            1.70  15.10.92 Merker f�r �nderungen an Level / Feldern
            1.80  12.01.93 Level als Grafik sichern
            2.03  06.06.93 Dialog entschl�sseln, Koordinateneingabe

            3.00  14.09.93 Neuprogrammierung f�r HASCS III
            3.10  05.11.93 Oberfl�chenumstellung
            3.20  26.06.94 Koordinateneingabe
            4.00  02.07.94 Generator
            4.50  12.08.94 Monster-/Gegenstandsklassen
            4.60  11.10.94 Spielprogramm integriert
*/


unsigned User;
unsigned XOff, YOff, Modus;
unsigned SpriteNummer, Hintergrund, LevelNummer,
	Groesse, Anzahl, GZufall;
int LevelChanged, FelderChanged;

unsigned CopyBuffer[24][24];
unsigned CopyWidth, CopyHeight;

unsigned DialogNummer;

char SampleFile[21];
SoundType Sample;

char LevelDatei[21], LevelImage[21];


void LoescheLevel(unsigned f, int All);


unsigned Min (unsigned x, unsigned y)
{
	return x < y ? x : y;
}

unsigned Max(unsigned x, unsigned y)
{
	return x > y ? x : y;
}

/* Bildschirmausgaben ***************************************************/

void PrintMenue(void);
{
	unsigned i;

	void PrintMenueEntry(unsigned x, unsigned y, char *s)
	{
		unsigned tx;
		tx = 2 * x + 7 - Length(s) / 2;
		PrintAt(tx, y, s);
		OutlineBar(x, y, x + 6, y);
	}

	FillRectangle(25, 0, 39, 13, SystemSprite[0]);
	OutlineBar(25, 0, 39, 13);
	PrintMenueEntry(25, 0, "Felder");
	PrintMenueEntry(25, 1, "Monster");
	PrintMenueEntry(25, 2, "Gegenst�nde");
	PrintMenueEntry(25, 3, "Parameter");
	for (i = 0; i <= 6; i++)
		InvertFeld(25+i, Modus);

	PrintMenueEntry(25, 5, "Block f�llen");
	PrintMenueEntry(25, 6, "... kopieren");
	PrintMenueEntry(25, 7, "... einf�gen");

	PrintMenueEntry(25, 9, "Generator");

	PrintMenueEntry(33, 0, "Level laden");
	PrintMenueEntry(33, 1, "...speichern");
	PrintMenueEntry(33, 2, "...Parameter");
	PrintMenueEntry(33, 3, "...anzeigen");

	PrintMenueEntry(33, 5, "Felder");
	PrintMenueEntry(33, 6, "Dialoge");
	PrintMenueEntry(33, 7, "Sounds");

	PrintMenueEntry(33, 9, "MUSTER");
	PrintMenueEntry(33, 10, "EDITOR");
	PrintMenueEntry(33, 11, "HASCS III");
	PrintMenueEntry(33, 12, "ENDE");
}

void PrintSprites()
{
	unsigned x;
	for (x = 0; x <= MaxSprites-1; x++)
		if (Modus = 0)
			SetSprite(x % 15 + 25, x / 15 + 14, FelderSprite[x]);
		else if (Modus = 1)
			SetSprite(x % 15 + 25, x / 15 + 14, MonsterSprite[x]);
		else if (Modus = 2)
			SetSprite(x % 15 + 25, x / 15 + 14, SystemSprite[x]);
}

void PrintFeld(unsigned i, j)
{
	unsigned x, y, n;

	NormalKoords(i - XOff, j - YOff, x, y);
	if (x > 24 || y > 24) return;
	SetSprite(x, y, FelderSprite[Feld]);
	if (LevelMonster IN Level[i][j].Spezial) {
		n = Max(FindMonster(i, j), 1);
		SetSprite(x, y, MonsterSprite[Monster[n].Typ]);
	} else if (LevelGegenstand & Level[i][j].Spezial) {
		n = Max(FindGegenstand(i, j), 1);
		SetSprite(x, y, SystemSprite[Gegenstand[n].Sprite]);
	}
	if ((Level[i][j].Modus == 1 && LevelMonster & Level[i][j].Spezial) 
       	 || (Level[i][j].Modus == 2 && LevelGegenstand & Level[i][j].Spezial)
	 || (Level[i][j].Modus == 3 && LevelParameter & Level[i][j].Spezial))
		InvertFeld(x, y);
}

void PrintLevelPart(void);
{
	unsigned x, y, i, j;
	
	for (i = 0; i <= 23; i++)
		for (j = 0; j <= 23; j++)
			NormalKoords(i + XOff, j + YOff, x, y);
			PrintFeld(x, y);
}

void PrintInfo(unsigned x, unsigned y)
{
	if (LevelChanged)
		PrintAt(0, 24, "*");
	else
		PrintAt(0, 24, " ");

	PrintAt(2, 24, "x="); PrintCard(x, 3);
	PrintAt(10, 24, "y="); PrintCard(y, 3);
	PrintAt(18, 24, "l="); PrintCard(LevelNummer, 3);
	PrintAt(26, 24, "m="); PrintCard(SpriteNummer, 3);
	switch (Modus) {
	case 0: SetSprite (16, 24, FelderSprite[SpriteNummer]);
		PrintAt(36, 24, "    "); break;
	case 1: SetSprite(16, 24, MonsterSprite[SpriteNummer]);
		PrintAt(36, 24, "�"); PrintCard(AnzahlMonster, 3); break;
	case 2: SetSprite (16, 24, SystemSprite[SpriteNummer]);
		PrintAt(36, 24, "�"); PrintCard(AnzahlGegen, 3); break;
	case 3: SetSprite (16, 24, SystemSprite[SpriteNummer]);
		PrintAt(36, 24, "�"); PrintCard(AnzahlParameter, 3); break;
	}

	SetSprite(21, 24, SystemSprite[8]);
	SetSprite(23, 24, SystemSprite[7]);
	SetSprite(24, 21, SystemSprite[5]);
	SetSprite(24, 23, SystemSprite[6]);
	OutlineBar(0,24,24,24); OutlineBar(24,0,24,24);
}

void MakeScreen(unsigned x, unsigned y);
{
	FillRectangle(0, 0, 39, 24, SystemSprite[0]);
	PrintLevelPart();
	PrintMenue();
	PrintSprites();
	PrintInfo(x, y);
}

int Wirklich(char *msg)
{
	unsigned d, ab; int ende;
	if (LevelChanged || FelderChanged) {
		NewScreen(12, 7, 20, 9, "");
		d = AddObject(2, 2, 16, 1, msg, Centered);
		if (LevelChanged)
			d = AddObject(2, 3, 16, 1, "Level nicht gespeichert!", Centered);
		if (FelderChanged)
			d = AddObject (2, 4, 16, 1, "Felder nicht gespeichert!", Centered);
		d = AddObject(2, 6, 6, 1, "OK", Exit|Outlined|Centered);
		ab = AddObject(12, 6, 6, 1, "Abbruch", Exit|Outlined|Centered);

		DrawScreen();
		ende = HandleScreen() != ab;
		MakeScreen(XOff, YOff);
		return ende;
	}
	return TRUE;
}

(************************************************************************)

unsigned FeldAuswahl(void)
{
	unsigned mx, my, x, y, i;
	BITSET mb;
	char ch;

	x = 9; y = 6;
	ReserveScreen(x, y, x + 21, y + 9);
	for (i = 0; i <= MaxSprites - 1; i++)
		SetSprite(x + 1 + i % 20, y + 1 + i / 20, FelderSprite[i]);

	WaitInput(mx, my, mb, ch, -1); mx = mx / 16; my = my / 16;
	if (mx > x && my > y && mx < x + 21 && my < y + 9)
		return mx - x - 1 + 20 * (my - y - 1);
	else
		return 0;
}

unsigned Nachbar(unsigned x, unsigned y, unsigned f)
{
	unsigned i, j, n;
	n = 0;
	for (i = 0; i <= 2; i++)
		for (j = 0; j <= 2; j++)
			if (Level[(x+i+LevelBreite) % (LevelBreite+1),
			         (y+j+LevelHoehe) % (LevelHoehe+1)].Feld = f)
				n++;
	return n;
}

void Insel(unsigned x, unsigned y, unsigned f, 
	unsigned o, unsigned c, unsigned a)
{
	unsigned i, j, n;
	n = 0;
	while (n < c) {
		i = (x + LevelBreite + Zufall(3) - 1) % (LevelBreite + 1);
		j = (y + LevelHoehe + Zufall(3) - 1) % (LevelHoehe + 1);
		if ((Level[i][j].Feld == o && Nachbar(i,j,f) == 0 
		 && Nachbar(i,j,999) <= a)
			|| Level[i,j].Feld == 999)
		{
			x = i; y = j;
			Level[x][y].Feld = 999;
		}
		n++;
	}
	for (i = 0; i <= LevelBreite; i++)
		for (j = 0; j <= LevelHoehe; j++)
			if (Level[i][j].Feld == 999)
				Level[i][j].Feld = f;
}

int FindFeld(unsigned i, unsigned j, unsigned f, unsigned o)
{
	/* Zufallsauswahl eines Feldes f im Level ohne Nachbarn o */
	unsigned n, z, x, y;

	x = Zufall(LevelBreite+1) - 1; y = Zufall(LevelBreite+1) - 1;
	if (Level[x][y].Feld == f && Nachbar(x,y,o) == 0) {
		i = x; j = y; return TRUE;
	}
	n = 0;
	for (x = 0; x <= LevelBreite; x++)
		for (y = 0; y <= LevelHoehe; y++)
			if (Level[x][y].Feld == f && Nachbar(x,y,o) == 0)
				n++;
	if (n = 0) return FALSE;
	z = Zufall(n);
	n = 0;
	for (x = 0; x <= LevelBreite; x++)
		for (y = 0; y <= LevelHoehe; y++)
			if (Level[x][y].Feld == f && Nachbar(x,y,o) == 0) {
				n++;
				if (z <= n) {
				  	i = x; j = y; return TRUE;
				}
			}
	return FALSE;
}

void FeldAendern (unsigned f, unsigned o)
{
	unsigned x, y;

	for (x = 0; x <= LevelBreite; x++)
		for (y = 0; y <= LevelHoehe; y++)
			if (Level[x][y].Feld == f)
				Level[x][y].Feld = o;
}

void Generator(void)
{
	unsigned fa, ll, zu, af, nf, an, ok, ab, gr, x, y, i, Auswahl;
	int found;

	unsigned DialogBox()
	{
		char s[21]; unsigned d;
		NewScreen(10, 5, 21, 13, " GENERATOR ");
		af = AddObject(2, 2, 6, 1, "Untergrund: ", Exit|Outlined);
		CardToString(Hintergrund, 1, s);
		d = AddObject(9, 2, 1, 1, s, SpriteFill);
		nf = AddObject(11, 2, 6, 1, "Feld: ", Exit|Outlined);
		CardToString(SpriteNummer, 1, s);
		d = AddObject(18, 2, 1, 1, s, SpriteFill);
		gr = AddObject(2, 4, 8, 1, "Gr��e: ", Editable);
		SetInputCard(gr, Groesse);
		an = AddObject(11, 4, 8, 1, "Nachbarn: ", Editable);
		SetInputCard(an, Anzahl);
		zu = AddObject(2, 6, 8, 1, "Zufall Koord.", Outlined|Selectable|Centered);
		SetFlagSelected(zu, GZufall);

		fa = AddObject(2, 8, 8, 1, "Feld �ndern", Outlined|Centered|Exit);
		ll = AddObject(11, 8, 8, 1, "Level l�schen", Outlined|Centered|Exit);
		ok = AddObject(2, 10, 8, 1, "Generieren", Outlined|Centered|Exit);
		ab = AddObject(11, 10, 8, 1, "Abbruch", Outlined|Centered|Exit);
		DrawScreen();
		return HandleScreen();
	}

	do {
		Auswahl = DialogBox();
		if (Auswahl != ab) {
			Groesse = GetInputCard(gr);
			Anzahl = Min(GetInputCard(an), 9);
			GZufall = GetFlagSelected(zu);
		}
		if (Auswahl == af)
			Hintergrund = FeldAuswahl();
		else if (Auswahl == nf)
			SpriteNummer = FeldAuswahl();
		else if (Auswahl == ll)
			LoescheLevel(Hintergrund, FALSE);
		else if (Auswahl == fa)
			FeldAendern(Hintergrund, SpriteNummer);
		else if (Auswahl == ok) {
			if (GZufall == 1)
				found = FindFeld(x, y, Hintergrund, SpriteNummer);
			else
				x = XOff; y = YOff;
				found = Level[x][y].Feld = Hintergrund;
			
			if (found)
				Insel(x, y, SpriteNummer, Hintergrund, Groesse, Anzahl);
			
			XOff = x; YOff = y;
		}
		MakeScreen(XOff, YOff); WaitTime(0);
	while (Auswahl != ab);
}

/************************************************************************/

void InputMonster(unsigned x, unsigned y, unsigned neu, unsigned *t)
{
	MonsterTyp m; unsigned i, a, s, n, d;

	if (neu = 0 || neu = 2)
		m = MonsterKlasse[*t];
	else if (neu == 1 && LevelMonster IN Level[x][y].Spezial)
		m = Monster[Max(FindMonster(x,y),1)];
		*t = m.Typ;
	else
		return

	NewScreen(7, 4, 25, 15, " Monster ");
	n = AddObject(2, 2, 14, 1, "Name: ", Editable|Outlined);
	d = AddObject(2, 4, 7, 1, "Treffer: ", Editable);
	d = AddObject(10, 4, 6, 1, "TP: ", Editable);
	d = AddObject(2, 6, 7, 1, "Schaden: ", Editable);
	d = AddObject(10, 6, 6, 1, "Bonus: ", Editable);
	d = AddObject(2, 8, 7, 1, "Status: ", Editable);
	d = AddObject(10, 8, 6, 1, "Muster: ", Editable);
	d = AddObject(2, 10, 7, 1, "Dialog: ", Editable);
	d = AddObject(2, 12, 6, 1, "OK", Outlined|Exit|Centered);
	a = AddObject(9, 12, 6, 1, "Abbruch", Outlined|Exit|Centered);

	s = AddObject(17, 2, 6, 1, "Magisch", Outlined|Selectable|Centered);
	d = AddObject(17, 3, 6, 1, "Schnell", Outlined|Selectable|Centered);
	d = AddObject(17, 4, 6, 1, "Fernkampf", Outlined|Selectable|Centered);
	d = AddObject(17, 5, 6, 1, "T�r �ffnen", Outlined|Selectable|Centered);
	d = AddObject(17, 6, 6, 1, "Fliegt", Outlined|Selectable|Centered);
	d = AddObject(17, 7, 6, 1, "Schwimmt", Outlined|Selectable|Centered);
	d = AddObject(17, 8, 6, 1, "Geist", Outlined|Selectable|Centered);
	d = AddObject(17, 9, 6, 1, "Feuer", Outlined|Selectable|Centered);
	d = AddObject(17,10, 6, 1, "Reiten", Outlined|Selectable|Centered);
	d = AddObject(17,11, 6, 1, "Immun", Outlined|Selectable|Centered);
	d = AddObject(17,12, 6, 1, "Pariert", Outlined|Selectable|Centered);

	SetInputString(n, m.Name);
	SetInputCard(n+1, m.Trefferwurf);
	SetInputCard(n+2, m.TP);
	SetInputCard(n+3, m.Schaden);
	SetInputCard(n+4, m.Bonus);
	SetInputCard(n+5, m.Status);
	SetInputCard(n+6, m.Typ);
	SetInputCard(n+7, m.Sprich);
	for (i = 0; i <= 10; i++)
		if (i IN m.Spezial)
			SetFlagSelected(s + i, 1);

	DrawScreen();
	if (a == HandleScreen()) MakeScreen (x, y); return;

	GetInputString(n, m.Name);
	m.Trefferwurf = GetInputCard(n+1);
	m.TP = GetInputCard(n+2);
	m.Schaden = GetInputCard(n+3);
	m.Bonus = GetInputCard(n+4);
	m.Status = GetInputCard(n+5);
	m.Typ = Min(GetInputCard(n+6), MaxSprites - 1);
	m.Sprich = GetInputCard(n+7);
	m.Spezial = {};
	for (i = 0; i <=  10; i++)
		if (GetFlagSelected(s + i))
			INCL (m.Spezial, i);

	if (neu == 0 || neu == 1) { /* ins Level kopieren */
		DeleteMonster(x, y);
		NewMonster(x, y, m);
		LevelChanged = TRUE;
	} else if (neu == 2) {/* Klasse �ndern */
		MonsterKlasse[m.Typ] = m;
		FelderChanged = TRUE;
	}
	MakeScreen(x, y);
}


void InputGegenstand(unsigned x, unsigned y, unsigned neu, unsigned *s)
{
	GegenstandTyp g; unsigned i, d, na, fl, ab;

	if (neu == 0 || neu == 2)
		g = GegenKlasse[*s];
	else if (neu == 1 && LevelGegenstand IN Level[x][y].Spezial)
		g = Gegenstand[Max(FindGegenstand(x,y),1)];
		*s = g.KennNummer; 
	else
		return

	NewScreen(11, 4, 17, 14, " GEGENSTAND ");
	na = AddObject(2, 2, 13, 1, "Name: ", Editable|Outlined);
	d = AddObject(2, 4, 6, 1, "Typ: ", Editable);
	d = AddObject(9, 4, 6, 1, "Muster: ", Editable);
	d = AddObject(2, 6, 6, 1, "Spz.: ", Editable);
	d = AddObject(2, 7, 6, 1, "Par1: ", Editable);
	d = AddObject(2, 8, 6, 1, "Par2: ", Editable);
	d = AddObject(2, 9, 6, 1, "Par3: ", Editable);
	fl = AddObject(9, 6, 6, 1, "Magisch", Centered|Outlined|Selectable);
	d = AddObject(9, 7, 6, 1, "Verflucht", Centered|Outlined|Selectable);
	d = AddObject(9, 8, 6, 1, "Erkannt", Centered|Outlined|Selectable);
	d = AddObject(9, 9, 6, 1, "Chance", Centered|Outlined|Selectable);
	d = AddObject(2, 11, 6, 1, "OK", Centered|Outlined|Exit);
	ab = AddObject(9, 11, 6, 1, "Abbruch", Centered|Outlined|Exit);

	SetInputString (na, g.Name);
	SetInputCard(na + 1, g.KennNummer);
	SetInputCard(na + 2, g.Sprite);
	SetInputCard(na + 3, CARDINAL(g.Spezial));
	SetInputCard(na + 4, g.Ring);
	SetInputCard(na + 5, g.RingWirkung);
	SetInputCard(na + 6, g.RingDauer);

	for (i = 0; i <= 3; i++)
		if (i IN g.Flags) SetFlagSelected (fl+i, 1)
	
	DrawScreen();
	if (ab = HandleScreen()) MakeScreen(x, y); return;

	GetInputString (na, g.Name);
	g.KennNummer = GetInputCard (na + 1);
	g.Sprite = Min(GetInputCard (na + 2), MaxSprites - 1);
	g.Spezial = BITSET(GetInputCard(na + 3));
	g.Ring = GetInputCard (na + 4);
	g.RingWirkung = GetInputCard(na + 5);
	g.RingDauer = GetInputCard(na + 6);
	g.Flags = {};
	for (i = 0; i <= 3; i++)
		if (GetFlagSelected(fl+i))
			INCL(g.Flags, i);

	if (neu == 0 || neu == 1) {
		DeleteGegenstand(x, y);
		NewGegenstand(x, y, g);
		LevelChanged = TRUE;
	} else if (neu == 2) {
		GegenKlasse[g.Sprite] = g;
		FelderChanged = TRUE;
	}
	MakeScreen(x, y);
}


void InputParameter(unsigned x, unsigned y, int new, unsigned a)
{
	ParameterTyp p; unsigned i, f, d, ab, ar;

	if (new) {
		p = Parameter[1]; p.Art = a;
		f = Level[x,y].Feld;
		for (i = 1; i <= AnzahlParameter; i++)
			if (Level[Parameter[i].x][Parameter[i].y].Feld == f)
				p = Parameter[i];
	} else if (LevelParameter IN Level[x,y].Spezial)
		p = Parameter[Max(FindParameter(x,y),1)];
	else
		return;

	NewScreen(11, 6, 17, 11, " PARAMETER ");
	ar = AddObject(2, 2, 13, 1, "Art: ", Outlined|Editable);
	d = AddObject(2, 4, 6, 1, "Par1: ", Editable);
	d = AddObject(2, 5, 6, 1, "Par2: ", Editable);
	d = AddObject(2, 6, 6, 1, "Par3: ", Editable);
	d = AddObject(9, 4, 6, 1, "Par4: ", Editable);
	d = AddObject(9, 5, 6, 1, "Par5: ", Editable);
	d = AddObject(9, 6, 6, 1, "Par6: ", Editable);
	d = AddObject(2, 8, 6, 1, "OK", Centered|Exit|Outlined);
	ab = AddObject(9, 8, 6, 1, "Abbruch", Centered|Exit|Outlined);

	SetInputCard(ar, p.Art);
	SetInputCard(ar + 1, p.xhoch);
	SetInputCard(ar + 2, p.yhoch);
	SetInputCard(ar + 3, p.Levelhoch);
	SetInputCard(ar + 4, p.xrunter);
	SetInputCard(ar + 5, p.yrunter);
	SetInputCard(ar + 6, p.Levelrunter);

	DrawScreen();
	if (ab == HandleScreen()) MakeScreen(x, y); return;

	p. Art = GetInputCard(ar);
	p. xhoch = GetInputCard(ar + 1);
	p. yhoch = GetInputCard(ar + 2);
	p. Levelhoch = GetInputCard(ar + 3);
	p. xrunter = GetInputCard(ar + 4);
	p. yrunter = GetInputCard(ar + 5);
	p. Levelrunter = GetInputCard(ar + 6);

	DeleteParameter(x, y);
	NewParameter(x, y, p);
	LevelChanged = TRUE;
	MakeScreen(x, y)
}

void InputFeld (unsigned n)
{
	unsigned ab, na, d, sp, i;

	NewScreen(12, 6, 17, 12, " FELD ");
	na = AddObject(2, 2, 13, 1, "Name: ", Editable);
	sp = AddObject(2, 4, 6, 1, "Begehbar", Outlined|Selectable|Centered);
	d = AddObject(2, 5, 6, 1, "Durchsichtig", Outlined|Selectable|Centered);
	d = AddObject(2, 6, 6, 1, "Wasser", Outlined|Selectable|Centered);
	d = AddObject(2, 7, 6, 1, "Feuer", Outlined|Selectable|Centered);
	d = AddObject(9, 4, 6, 1, "Sumpf", Outlined|Selectable|Centered);
	d = AddObject(9, 5, 6, 1, "Anti Monster", Outlined|Selectable|Centered);
	d = AddObject(9, 6, 6, 1, "Hunger", Outlined|Selectable|Centered);
	d = AddObject(2, 9, 6, 1, "OK", Outlined|Centered|Exit);
	ab = AddObject(9, 9, 6, 1, "Abbruch", Outlined|Centered|Exit);

	SetInputString(na, Felder[n].Name);
	for (i = 0; i <= 6; i++)
		if (i IN Felder[n].Spezial)
			SetFlagSelected(sp + i, 1);

	DrawScreen();
	if (HandleScreen() != ab) {
		FelderChanged = TRUE;
		GetInputString(na, Felder[n].Name);
		for (i = 0; i <= 6; i++)
			if (GetFlagSelected (sp + i))
				INCL(Felder[n].Spezial, i);
			else
				EXCL(Felder[n].Spezial, i);
	}
	MakeScreen(XOff, YOff);
}

void InputLevelFeld(unsigned x, unsigned, int new, unsigned *f);
{
	if (new) {
		DeleteMonster(x, y);
		DeleteGegenstand(x, y);
		DeleteParameter(x, y);
		if (Level[x][y].Feld != *f) {
			Level[x][y].Feld = *f;
			LevelChanged = TRUE;
		}
		PrintFeld(x, y);
	} else
		*f = Level[x][y].Feld;
}

void FillBlock(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned n)
{
	unsigned i, j, x, y;

	for (i = x1; i <= x2; i++)
		for (j = y1; j <= y2; j++)
			Level[x][y].Feld = n;
}


/************************************************************************/

void LoescheLevel(unsigned f, int All)
{
	unsigned x, y;
	LevelTyp l;


	l.Feld = f;
	l.Spezial = {};

	for (x = 0; x <= MaxBreite; x++)
		for (y = 0; y <= MaxHoehe; y++)
			Level[x][y] = l;

	AnzahlGegen = 0;
	AnzahlMonster = 0;
	AnzahlParameter = 0;
	if (All) {
		LevelName = "";
		LevelSprites = "HASCSIII";
		LevelBreite = 99;
		LevelHoehe = 99;
		LevelFlags = {};
		LevelSichtweite = 11;
		LevelDialog = 0;
		LevelMaxMonster = 0;
	}
}

void ScrollLevelPart(unsigned dir, unsigned delta)
{
	unsigned dx, dy;

	dx = 0; dy = 0;
	if (dir == 1 || dir == 2 || dir == 3)
		dy = delta;
	if (dir == 1 || dir == 4 || dir == 7)
		dx = LevelBreite + 1 - delta;
	if (dir == 3 || dir == 6 || dir == 9)
		dx = delta;
	if (dir == 7 || dir == 8 || dir == 9)
		dy = LevelHoehe + 1 - delta;
	XOff = (XOff + dx) % (LevelBreite + 1);
	YOff = (YOff + dy) % (LevelHoehe + 1);
	PrintLevelPart();
}

void LevelLaden(int Links)
{
	unsigned nr, l, as, ok, ab, t, i, d;

	if (!Wirklich("Wirklich Level laden?")) return;

	NewScreen(14, 7, 17, 9, " LEVEL LADEN ");
	nr = AddObject(2, 2, 6, 1, "Nummer: ", Editable);
	as = AddObject(9, 4, 6, 1, "Leveldatei", Centered|Outlined|Exit);
	ok  = AddObject(2, 6, 6, 1, "OK", Exit|Outlined|Centered);
	ab = AddObject(9, 6, 6, 1, "Abbruch", Exit|Outlined|Centered);

	SetInputCard(nr, LevelNummer);
	DrawScreen(); d = HandleScreen();
	l = GetInputCard (nr);

	if (l > 0 AND l < 1000)
		if (d = ok) {
			LoadLevel(l);
			if (User != BenutzerNummer) {
				BenutzerNummer = User;
				LoescheLevel(0, TRUE);
			} else if (!FileError) {
				LevelNummer = l;
				LevelChanged = FALSE;
				for (i = 1; i <= AnzahlMonster; i++) {
					t = Monster[i].Typ;
					if (MonsterKlasse[t].Status == 0)
						MonsterKlasse[t] = Monster[i];
				}
				for (i = 1; i <= AnzahlGegen; i++) {
					t = Gegenstand[i].Sprite;
					if (GegenKlasse[t].KennNummer == 0)
						GegenKlasse[t] = Gegenstand[i];
				}
			}
		} else if (d = as) {
			if (SelectFile ("Leveldatei laden", "*.LEV", LevelDatei))
				ReadLevel(LevelDatei);
		}
	MakeScreen(XOff, YOff);
}

void LevelSpeichern(int Links);
{
	unsigned nr, l, as, ok, ab, bi, d;
	NewScreen(14, 7, 17, 9, " LEVEL SPEICHERN ");
	nr = AddObject(2, 2, 6, 1, "Nummer: ", Editable);
	bi = AddObject(2, 4, 6, 1, "Bild", Centered|Outlined|Exit);
	as = AddObject(9, 4, 6, 1, "Leveldatei", Centered|Outlined|Exit);
	ok  = AddObject(2, 6, 6, 1, "OK", Exit|Outlined|Centered);
	ab = AddObject(9, 6, 6, 1, "Abbruch", Exit|Outlined|Centered);

	SetInputCard(nr, LevelNummer);
	DrawScreen(); d = HandleScreen();
	l = GetInputCard(nr);

	if (l > 0 && l < 1000 && d != ab)
		if (d = ok) {
			SaveLevel(l);
			if (!FileError) {
				LevelNummer = l;
				LevelChanged = FALSE;
			}
		} else if (d = bi) {
			if (SelectFile ("Level als Bild speichern...", "*.IMG", LevelImage))
				if (SaveImage(LevelImage));
		} else if (d = as) {
			if (SelectFile ("Leveldatei speichern", "*.LEV", LevelDatei))
				WriteLevel(LevelDatei);
		}
	MakeScreen(XOff, YOff);
}

void EingabeParameter(void)
{
	unsigned ok, i, na, ab, us, fl, d;
	char MusterDatei[61];

	NewScreen(11, 2, 17, 19, " LEVEL PARAMETER ");
	na = AddObject(2, 2, 13, 1, "Name: ", Editable|Outlined);
	d = AddObject(2, 4, 13, 1, "Musterdatei: ", Outlined|Exit);
	d = AddObject(2, 6, 6, 1, "Breite: ", Editable);
	d = AddObject(9, 6, 6, 1, "H�he: ", Editable);
	d = AddObject(2, 7, 6, 1, "Sicht: ", Editable);
	d = AddObject(9, 7, 6, 1, "Dialog: ", Editable);
	d = AddObject(2, 8, 13, 1, "Max. Monsteranzahl: ", Editable);
	fl = AddObject(2, 10, 6, 1, "Kein Speich.", Centered|Outlined|Selectable);
	d = AddObject(9, 10, 6, 1, "Keine Karte", Centered|Outlined|Selectable);
	d = AddObject(2, 11, 6, 1, "Nicht Zykl.", Centered|Outlined|Selectable);
	d = AddObject(9, 11, 6, 1, "Typ aggr.", Centered|Outlined|Selectable);
	d = AddObject(2, 12, 6, 1, "Alle aggr.", Centered|Outlined|Selectable);
	d = AddObject(9, 12, 6, 1, "Auto Dialog", Centered|Outlined|Selectable);
	us = AddObject(2, 14, 13, 1, "Benutzernummer: ", Editable);
	ok = AddObject(2, 16, 6, 1, "OK", Exit|Outlined|Centered);
	ab = AddObject(9, 16, 6, 1, "Abbruch", Exit|Outlined|Centered);

	SetInputString(na, LevelName);
	SetInputString(na + 1, LevelSprites);
	SetInputCard(na + 2, LevelBreite);
	SetInputCard(na + 3, LevelHoehe);
	SetInputCard(na + 4, LevelSichtweite);
	SetInputCard(na + 5, LevelDialog);
	SetInputCard(na + 6, LevelMaxMonster);
	for (i = 0; i <=  5; i++)
		if (i IN LevelFlags)
			SetFlagSelected(fl+i, 1);
		else
			SetFlagSelected(fl+i, 0);

	SetInputCard (us, User);

	do {
		DrawScreen();
		i = HandleScreen();
		if (i = ab) {MakeScreen(XOff, YOff); return;}
		else if (i = na + 1) {
			GetInputString(na + 1, MusterDatei);
			Concat(MusterDatei, MusterDatei, ".SPR");
			if (SelectFile("Musterdatei...", "*.SPR", MusterDatei)) {
				d = FindC(MusterDatei, '.'); if (d) MusterDatei[d] = 0
				SetInputString(na + 1, MusterDatei);
			}
		}
	} while (i != ok);

	GetInputString(na, LevelName);

	GetInputString(na + 1, MusterDatei);
	if (!COMPARE(MusterDatei, LevelSprites)) {
		Concat(MusterDatei, MusterDatei, ".SPR");
		LoadOrSaveSprites(TRUE, MusterDatei);
		if (!FileError) {
			GetInputString (na + 1, LevelSprites);
			GetInputString (na + 1, MusterDatei);
			Concat(MusterDatei, MusterDatei, ".DAT");
			LoadOrSaveDat(TRUE, MusterDatei);
		}
	}

	LevelBreite = Min(GetInputCard (na + 2), MaxBreite);
	LevelHoehe = Min(GetInputCard (na + 3), MaxHoehe);
	LevelSichtweite = Min(GetInputCard (na + 4), MaxSichtweite);
	LevelDialog = GetInputCard(na + 5);
	LevelMaxMonster = Min(GetInputCard (na + 6), MaxMonster);
	LevelFlags = {};
	for (i = 0; i <=  5; i++)
		if (GetFlagSelected(fl+i) == 1)
			INCL(LevelFlags, i);
	User = GetInputCard (us); BenutzerNummer = User;
	MakeScreen(XOff, YOff);
}

void LevelAnzeigen(BITSET b);
{
	unsigned x, y;

	if (MausLinks IN b) {
		Spieler.x = LevelBreite / 2;
		Spieler.y = LevelHoehe / 2;
	} else {
		Spieler.x = (XOff + 12) % (LevelBreite + 1);
		Spieler.y = (YOff + 12) % (LevelHoehe + 1);
	}
	if (Vision(44, 0, x, y)) {
		XOff = (x + LevelBreite - 12) % (LevelBreite + 1);
		YOff = (y + LevelHoehe - 12) % (LevelHoehe + 1);
	}
	MakeScreen(XOff, YOff);
}

void FelderSpeichern(void)
{
	char s[21]
	unsigned d, ab;

	Concat(s, LevelSprites, ".DAT");
	NewScreen(14, 8, 17, 7, " FELDER SPEICHERN ");
	d = AddObject(2, 2, 13, 1, s, Centered);
	d = AddObject(2, 4, 6, 1, "OK", Exit|Outlined|Centered);
	ab = AddObject(9, 4, 6, 1, "Abbruch", Exit|Outlined|Centered);

	DrawScreen();
	if (HandleScreen() != ab) {
		LoadOrSaveDat(FALSE, s);
		FelderChanged = FALSE;
	}
	MakeScreen(XOff, YOff);
}

void DialogBearbeiten(void)
{
	unsigned nr, al, co, de, te, ab, h, i, begin, ende;

	NewScreen(12, 7, 17, 9, " DIALOG ");
	nr = AddObject(2, 2, 7, 1, "Nummer: ", Editable);
	al = AddObject(11, 2, 4, 1, "Alle", Centered|Selectable|Outlined);
	co = AddObject(2, 4, 6, 1, "Kodieren", Centered|Outlined|Exit);
	de = AddObject(9, 4, 6, 1, "Dekodieren", Centered|Outlined|Exit);
	te = AddObject(2, 6, 6, 1, "Testen", Centered|Outlined|Exit);
	ab = AddObject(9, 6, 6, 1, "Abbruch", Centered|Outlined|Exit);
	SetInputCard(nr, DialogNummer);
	DrawScreen(); h = HandleScreen();
	if (h != ab) {
		FreeCache(0);
		DialogNummer = GetInputCard(nr);
		if (h = co || h = de) {
			if (GetFlagSelected(al) == 1) { /* alle Dialoge */
				begin = 0;
				ende = 999;
			} else {
				begin = DialogNummer;
				ende = DialogNummer;
			}
			for (i = begin; i <= ende; i++)
				if (SaveDialog(i, h == co))
					SetInputCard(nr, i); DrawObject(nr); WaitTime(0);
		} else if (h = te)
			DoDialog(DialogNummer);
	}
	MakeScreen(XOff, YOff);
}

void SoundBearbeiten()
{
	unsigned h, d, la, sp, of, ab;
	int b;

	do {
		NewScreen(12, 7, 17, 9, " SOUND ");
		d = AddObject(2, 2, 4, 1, "Datei: ", 0);
		d = AddObject(6, 2, 7, 1, SampleFile, 0);
		la = AddObject(2, 4, 6, 1, "Laden", Centered|Outlined|Exit);
		sp = AddObject(9, 4, 6, 1, "Spielen", Centered|Outlined|Exit);
		of = AddObject(2, 6, 6, 1, "Aus", Centered|Outlined|Exit);
		ab = AddObject(9, 6, 6, 1, "Abbruch", Centered|Outlined|Exit);
		DrawScreen();
		h = HandleScreen();
		if (h != ab) {
			if (h = la) {
				if (SelectFile("Welcher Sound...", "*.*", SampleFile)) {
					FreeCache(0);
					if (!LoadSoundFile(SampleFile, 1, Sample))
						Sample = SoundOff;
				}
			} else if (h = sp)
				PlaySound(Sample);
			else if (h = of)
				PlaySound(SoundOff);
		}
	} while (h != ab);
	MakeScreen(XOff, YOff);
}

void XYEingabe(void)
{
	unsigned d, ab, h, x, y;

	NewScreen(12, 10, 17, 7, " KOORDINATEN ");
	x = AddObject(2, 2, 6, 1, "x = ", Editable);
	y = AddObject(9, 2, 6, 1, "y = ", Editable);
	d = AddObject(2, 4, 6, 1, "OK", Centered|Outlined|Exit);
	ab = AddObject(9, 4, 6, 1, "Abbruch", Centered|Outlined|Exit);
	SetInputCard(x, XOff); SetInputCard(y, YOff);
	DrawScreen();
	if (HandleScreen() != ab) {
		XOff = GetInputCard(x) % (LevelBreite + 1);
		YOff = GetInputCard(y) % (LevelHoehe + 1);
	}
	MakeScreen(XOff, YOff);
}

/************************************************************************/

int BlockDefine (char *t, unsigned *x1, unsigned *y1, 
		unsigned *x2, unsigned *y2)
{
	unsigned x, y, d, i, j; BITSET b; char ch;

	NewScreen(25, 0, 15, 14, t);
	d = AddObject(2, 2, 11, 1, "Linke obere Ecke...", 0);
	DrawScreen();
	WaitInput(x, y, b, ch, -1); *x1 = x / 16; *y1 = y / 16;
	if (*x1 >= 24 OR *y1 >= 24)
		return FALSE;
	d = AddObject(2, 4, 11, 1, "Rechte untere Ecke...", 0);
	DrawScreen();
	WaitInput(x, y, b, ch, -1); *x2 = x / 16; *y2 = y / 16;
	if (*x2 >= 24 OR *y2 >= 24)
		return FALSE;
	if (*x1 > *x2) {i = *x1; *x1 = *x2; *x2 = i;}
	if (*y1 > *y2) {i = *y1; *y1 = *y2; *y2 = i;}
	return TRUE;
}

void BlockFuellen(void)
{
	unsigned i, j, x, y, x1, y1, x2, y2;

	if (BlockDefine(" BLOCK F�LLEN ", x1, y1, x2, y2)) {
		for (x = x1; x <= x2; x++)
			for (y = y1; y <= y2; y++) {
				NormalKoords(x + XOff, y + YOff, i, j);
				Level[i][j].Feld = SpriteNummer;
			}
		LevelChanged = TRUE;
	}
	MakeScreen(XOff, YOff);
}

void BlockKopieren(void)
{
	unsigned i, j, x, y, x1, y1, x2, y2; BITSET b; char ch;

	if (BlockDefine (" BLOCK KOPIEREN ", x1, y1, x2, y2)) {
		for (x = x1; x <= x2; x++)
			for (y = y1; y <= y2; y++) {
				NormalKoords(x + XOff, y + YOff, i, j);
				CopyBuffer[x - x1, y - y1] = Level[i, j].Feld;
			}
		CopyWidth = x2 - x1 + 1;
		CopyHeight = y2 - y1 + 1;
	}
	MakeScreen(XOff, YOff);
}

void BlockEinfuegen(void)
{
	unsigned i, j, x, y, x1, y1, x2, y2;
	
	if (CopyWidth == 0 || CopyHeight == 0)
		return;
	if (BlockDefine(" BLOCK EINF�GEN ", x1, y1, x2, y2)) {
		for (x = 0; x <= CopyWidth - 1; x++)
			for (y = 0; y <= CopyHeight - 1; y++) {
				NormalKoords(x1 + x + XOff, y1 + y + YOff, i, j);
				Level[i][j].Feld = CopyBuffer[x][y];
			}
		LevelChanged = TRUE;
	}
	MakeScreen(XOff, YOff);
}

/************************************************************************/

void DoEdit(void)
{
	int Ende;
	unsigned x, y;
	BITSET b;
	char ch;

	void KeyToMouse(char ch, unsigned *mx, unsigned *my, BITSET *mb);
	{
		if (ch >= '1' && ch <= '9') {
			mx = 24; my = 24;
		}
	}

	void DoMenue(unsigned x, unsigned y, BITSET b; char ch)
	{
		int dummy;
		if (x < 32)
			switch (y) {
			case 0...3: Modus = y; MakeScreen(XOff, YOff); break;
			case 5: BlockFuellen(); break;
			case 6: BlockKopieren(); break;
			case 7: BlockEinfuegen(); break;
			case 9: Generator(); break;
			}
		else
			switch (y) {
			case  0: LevelLaden(MausLinks IN b); break;
			case  1: LevelSpeichern(MausLinks IN b); break;
			case  2: EingabeParameter(); break;
			case  3: LevelAnzeigen(b); break;
			case  5: FelderSpeichern(); break;
			case  6: DialogBearbeiten(); break;
			case  7: SoundBearbeiten(); break;
			case  9: SpriteEdit(LevelSprites); MakeScreen(XOff, YOff); break;
			case 10: dummy = LoadAndRun (TextEditor, ""); break;
			case 11:
				if (Wirklich("Wirklich Spiel starten?")) {
					FreeCache(0); StartGame();
					Editor = TRUE; MaxX = 80; LevelNummer = 0;
					MakeScreen(XOff, YOff);
					LevelChanged = FALSE;
					FelderChanged = FALSE;
					Spieler.OldLevels = CardSet{};
				}
				break;
			case 12: Ende = Wirklich("Wirklich Editor verlassen?"); break;
			}
	}

	void DoScroll(unsigned x, unsigned y, BITSET b, char ch)
	{
		unsigned dir, delta;

		dir = 0; delta = 0;
		if (ch >= '1' && ch <= '9') {
			dir = ch - '0'; delta = 3;
		} else {
			if (y == 24 && x == 23) dir = 6
			else if (y == 24 && x == 21) dir = 4;
			else if (y == 23 &&x == 24) dir = 2;
			else if (y == 21 &&x == 24) dir = 8;
			else if (y == 24 &&x < 9) {
				XYEingabe(); return;
			}
			if (MausLinks IN b) delta = 3;
			else if (MausRechts IN b) delta = 1;
			else delta = 0;
		}
		ScrollLevelPart(dir, delta);
	}

	void DoSprite(unsigned x, unsigned y, BITSET b, char ch)
	{
		unsigned n;

		n = Min((y - 14) * 15 + x - 25, MaxSprites - 1);
		if (MausLinks IN b) {
			SpriteNummer = n;
			PrintInfo(XOff, YOff);
		} else if (MausRechts IN b) {
			switch (Modus) {
			case 0 : InputFeld(n); break;
			case 1 : InputMonster(0, 0, 2, n); break;
			case 2 : InputGegenstand(0, 0, 2, n); break;
			}
		}
	}

	void DoLevel(unsigned x, unsigned y, BITSET b, char ch)
	{
		unsigned lx, ly, neu;

		NormalKoords(x + XOff, y + YOff, lx, ly);
		neu = 1; if (MausLinks IN b) neu = 0;
		switch (Modus) {
		case 0: InputLevelFeld(lx, ly, MausLinks IN b, SpriteNummer); break;
		case 1: InputMonster(lx, ly, neu, SpriteNummer); break;
		case 2: InputGegenstand(lx, ly, neu, SpriteNummer); break;
		case 3: InputParameter(lx, ly, MausLinks IN b, SpriteNummer); break;
		}
		PrintInfo(lx, ly);
	}

	Ende = FALSE;
	do {
		WaitInput(x, y, b, ch, -1); x = x / 16; y = y / 16;
		KeyToMouse(ch, x, y, b);

		if (x < 24 && y < 24) /* Levelausschnitt */
			DoLevel (x, y, b, ch);
		else if (x < 25 && y < 25) /* Scroll */
			DoScroll(x, y, b, ch);
		else if (y < 14) /* Men� */
			DoMenue(x, y, b, ch);
		else /* Sprites */
			DoSprite(x, y, b, ch);
	} while (!Ende);
}


unsigned CopyRight(void)
{
	unsigned d, b;

	NewScreen(7, 4, 25, 13, "");
	d = AddObject(2, 2, 20, 1, "HASCS III", Centered);
	d = AddObject(2, 4, 20, 1, "Editor Version 1.43", Centered);
	d = AddObject(2, 6, 20, 1, "Copyright � 1987-1995 Alexander Kirchner", 
			                          Centered);
	b = AddObject(7, 9, 13, 1, "Benutzernummer: ", Editable);
	DrawScreen(); d = HandleScreen();
	return GetInputCard(b);
}


void InitEditor(void);
{
	unsigned i;
	
	XOff = 0; YOff = 0; Modus = 0;
	LevelNummer = 1; SpriteNummer = 0;
	LevelChanged = FALSE;
	FelderChanged = FALSE;
	CopyWidth = 0; CopyHeight = 0;
	DialogNummer = 0;

	SampleFile = ""; /* Sound */
	Sample = SoundOff;

	Groesse = 100; /* Generator */
	Anzahl = 1;
	GZufall = 1;
	
	FOR i = 0 TO MaxSprites-1 DO
		MonsterKlasse[i].Typ = i;
		MonsterKlasse[i].Status = 0;
		GegenKlasse[i].Sprite = i;
		GegenKlasse[i].KennNummer = 0;
	END;

	Spieler.OldLevels = CardSet{}; /* Original Levels laden */

	LevelDatei = "HASCS_3.LEV";
	LevelImage = "HASCS_3.IMG";
}


void EditorInit(void)
{

	Editor = TRUE;
	SoundAusgabe = TRUE;
	DruckerAusgabe = FALSE;
	DebugMode = FALSE;

	InitWorkstation(" HASCS III - Editor ");
	InitEditor();

	if (ReadConfig()) {
		User = CopyRight();
		if (User != BenutzerNummer) {
			BenutzerNummer = User;
			LoescheLevel(0, TRUE);
		}
		MakeScreen(XOff, YOff);
		DoEdit();
	}

	ExitWorkstation(0);
}

