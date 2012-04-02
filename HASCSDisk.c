#include "HASCSDisk.h" /* HASCSDisk module */

/* HASCS Importe */
#include "HASCSGlobal.h"
#include "HASCSGraphics.h"
#include "HASCSSystem.h"
#include "HASCSOutput"


#define BufferSize 40000


typedef char *CharPtr;

char LevelPars[100];

String60Typ LastSprites;

unsigned long ZufallsZahl;
unsigned LevelVersion;

char *Buffer;

/* Fehlermeldung *********************************************************/

void Fehler(char msg[], char file[])
{
	char s[256];
	Concat(s, msg, file);
	Error(s, 0);
}


/* Kodierung *************************************************************/

unsigned PseudoZufall(unsigned n)
{
	ZufallsZahl = (ZufallsZahl * 153 + 97) % 16777216;
	return ZufallsZahl % (long)n + 1;
}

void WriteBlock(int handle, unsigned anzahl, CharPtr a)
{
	unsigned i;
	unsigned long count, pruef, start;
	if (anzahl == 0) return;
	pruef = 0;
	ZufallsZahl = ((long)(Zufall(256)-1)*256+(long)(Zufall(256)-1))*256+
		(long)(Zufall(256)-1);
	start = ZufallsZahl; /* Startwert */
	for (i = 0; i <= anzahl-1; i++) {
		pruef = pruef * 3 + (unsigned long)*a % 65536;
		Buffer[i] = (unsigned)*a + PseudoZufall(256) % 256;
		a++;
	}
	Buffer[anzahl]   = start / 65536;
	Buffer[anzahl+1] = (start % 65536) / 256;
	Buffer[anzahl+2] = start % 256;
	Buffer[anzahl+3] = pruef / 256;
	Buffer[anzahl+4] = pruef % 256;
	count = (long)anzahl + 5;
	WriteFile(handle, count, Buffer);
}

void ReadBlock(int handle, unsigned anzahl, CharPtr a)
{
	unsigned i;
	unsigned long count, pruef, test;
	if (anzahl == 0) return;
	pruef = 0;
	count = (long)anzahl + 5;
	ReadFile(handle, count, Buffer);
	if (FileError) return;
	ZufallsZahl = (long)Buffer[anzahl]) * 65536 + 
		(long)Buffer[anzahl+1] * 256 +
		(long)Buffer[anzahl+2];
	test = (long)Buffer[anzahl+3] * 256 +
		(long)Buffer[anzahl+4];
	for (i = 0; i <= anzahl-1; i++) {
		*a = ((unsigned)Buffer[i] + 256 - PseudoZufall(256)) % 256;
		pruef = pruef * 3 + (unsigned long)*a % 65536;
		a++;
	}
	if (test != pruef && !Editor)
		Error("Prüfsummenfehler!", -1);
}


/* Felder ****************************************************************/

void LoadOrSaveDat(int Load, char *FileName)
{
	int h; String60Typ s;
	Buffer = GetBuffer(BufferSize);
	Concat(s, PrgPath, FileName);
	if (Load) {
		h = OpenFile(s);
		if (FileError)
			Fehler("Felderdaten Lesefehler: ", s); return;
		ReadBlock(h, sizeof Felder, &Felder);
		if (Editor) {
			ReadBlock(h, sizeof MonsterKlasse, &MonsterKlasse);
			ReadBlock(h, sizeof GegenKlasse, &GegenKlasse);
		}
	} else {
		h = CreateFile(s);
		if (FileError)
			Fehler("Felderdaten Schreibfehler: ", s); return;
		WriteBlock(h, sizeof Felder, &Felder);
		WriteBlock(h, sizeof MonsterKlasse, &MonsterKlasse);
		WriteBlock(h, sizeof GegenKlasse, &GegenKlasse);
	}
	CloseFile(h);
}

void LoadOrSaveSprites(int Load; char *FileName)
{
	int h; unsigned long Count; String60Typ s;
	Assign(LastSprites, FileName);
	Concat(s, PrgPath, FileName);
	if (Load) {
		h = OpenFile(s);
		if (FileError) {
			Fehler("Muster Lesefehler: ", s); return;
		}
		Count = MaxSprites * sizeof (SpriteType);
		ReadFile(h, Count, &FelderSprite);
		ReadFile(h, Count, &MonsterSprite);
		ReadFile(h, Count, &SystemSprite);
		ReadFile(h, Count, &GegenSprite);
	} else {
		h = CreateFile(s);
		if (FileError) {
			Fehler("Muster Schreibfehler: ", s); return;
		}
		Count = MaxSprites * sizeof (SpriteType);
		WriteFile(h, Count, &FelderSprite);
		WriteFile(h, Count, &MonsterSprite);
		WriteFile(h, Count, &SystemSprite);
		WriteFile(h, Count, &GegenSprite);
	}
	CloseFile(h);
}


/* Level *****************************************************************/

void LoadOrSaveLevel(int Load, String60Typ Name)
{
	unsigned long Count;
	unsigned shortlength, i;
	String60Typ s;
	int h;

	void MakeLevelPars(void)
	{
		unsigned x;
		LevelPars[0] = VersionsNummer;
		LevelPars[1] = LevelBreite;
		LevelPars[2] = LevelHoehe;
		LevelPars[4] = AnzahlGegen;
		LevelPars[5] = AnzahlMonster;
		LevelPars[6] = AnzahlParameter;
		LevelPars[7] = BenutzerNummer / 256;
		LevelPars[8] = BenutzerNummer % 256;
		for (x = 0; x <= 19; x++) { /* Levelname, Spritename */
			LevelPars[x + 10] = LevelName[x];
			LevelPars[x + 80] = LevelSprites[x];
		}
		LevelPars[30] = LevelFlags % 256;
		LevelPars[31] = LevelFlags / 256;
		LevelPars[32] = LevelSichtweite % 256;
		LevelPars[33] = LevelSichtweite / 256;
		LevelPars[34] = LevelDialog % 256;
		LevelPars[35] = LevelDialog / 256;
		LevelPars[36] = LevelMaxMonster % 256;
		LevelPars[37] = LevelMaxMonster / 256;
	}

	unsigned MakeBuffer(void)
	{
		unsigned i, x, y, old, counter, anzahl;
		counter = 0;
		anzahl = 0;
		old = Level[0][0].Feld;
		for (x = 0; x <= LevelBreite; x++) {
			for (y = 0; y <= LevelHoehe; y++) {
				if (old == Level[x][y].Feld && anzahl < 255)
					anzahl++;
				else {
					if (anzahl < 3)
						for (i = 1; i <= anzahl; i++) {
							Buffer[counter] = old; counter++;
						}
					else {
						Buffer[counter] = 255; counter++;
						Buffer[counter] = anzahl; counter++;
						Buffer[counter] = old; counter++;
					}
					anzahl = 1;
					old = Level[x,y].Feld;
				}
			}
		}
		if (anzahl < 3)
			for (i = 1; i <= anzahl; i++) {
				Buffer[counter] = CHR(old); counter++;
			}
		else {
			Buffer[counter] = CHR(255); counter++;
			Buffer[counter] = CHR(anzahl); counter++;
			Buffer[counter] = CHR(old); counter++;
		}
		return counter;
	}

	void AuswertLevelPar(void)
	{
		unsigned x, p;
		LevelVersion    = LevelPars[0];
		LevelBreite     = LevelPars[1];
		LevelHoehe      = LevelPars[2];
		AnzahlGegen     = LevelPars[4];
		AnzahlMonster   = LevelPars[5];
		AnzahlParameter = LevelPars[6];
		BenutzerNummer  = LevelPars[7]*256+LevelPars[8];
		for (x = 0; x <= 19; x++) {
			LevelName[x] = LevelPars[10 + x];
			LevelSprites[x] = LevelPars[80+x];
		}
		LevelFlags      = LevelPars[31]*256+LevelPars[30];
		LevelSichtweite = LevelPars[33]*256+LevelPars[32];
		LevelDialog     = LevelPars[35]*256+LevelPars[34];
		LevelMaxMonster = LevelPars[37]*256+LevelPars[36];
	}

	void AuswertBuffer(unsigned counter)
	{
		unsigned x, y, i, j, anzahl, f;
		x = 0;
		y = 0;
		i = 0;
		while (i < counter) {
			if (Buffer[i] == 255) {
				anzahl = Buffer[i+1];
				f = Buffer[i+2];
				i += 3;
			} else {
				anzahl = 1;
				f = Buffer[i];
				i++;
			}
			for (j = 1; j <= anzahl; j++) {
				Level[x][y].Feld = f;
				Level[x][y].Spezial = 0;
				if (y < LevelHoehe)
					y++;
				else {
					y = 0;
					x++;
				}
			}
		}
	}

	void KarteLaden(unsigned Counter)
	{
		unsigned x, y, c, i; int k;
		x = 0; y = 0; c = 0; i = 0;
		while (x <= LevelBreite && y <= LevelHoehe) {
			if (c == 0) {
				c = Buffer[i] % 128;
				k = Buffer[i] / 128 == 1;
				i++;
				if (i >= Counter)
					return;
			}
			if (k)
				Level[x,y].Spezial |= LevelKarte;
			x++;
			if (x > LevelBreite) {
				x = 0; y++;
			}
			c--;
		}
	}
	
	unsigned KarteSpeichern();
	{
		unsigned x, y, c, i, k, n;
		x = 1; y = 0; i = 0; k = 0; c = 1;
		if (LevelKarte & Level[0][0].Spezial) k = 128;
		while (x <= LevelBreite && y <= LevelHoehe) {
			n = 0;
			if (LevelKarte & Level[x][y].Spezial) n = 128;
			if (n != k || c >= 127) {
				Buffer^[i] = CHR(k + c);
				i++;
				c = 1;
			} else {
				c++;
			}
			k = n;
			x++;
			if (x > LevelBreite) {
				x = 0; y++;
			}
		}
		Buffer[i] = k + c;
		return i + 1;
	}
	
	Buffer = GetBuffer(BufferSize);
	if (Load) { /* Level laden */
	
		h = OpenFile(Name);
		if (FileError) { return; }
		Count = sizeof LevelPars;
		ReadBlock(h, Count, &LevelPars);
		AuswertLevelPars;

		ReadFile(h, sizeof shortlength, &shortlength);
		ReadFile(h, shortlength, Buffer);
		AuswertBuffer(shortlength);

		Count = sizeof (MonsterTyp) * AnzahlMonster;
		ReadBlock(h, Count, &Monster);
		Count = sizeof (GegenstandTyp) * AnzahlGegen;
		ReadBlock(h, Count, &Gegenstand);
		Count = sizeof (ParameterTyp) * AnzahlParameter;
		ReadBlock(h, Count, &Parameter);

		if (!Editor) { /* Karte laden */
			ReadFile(h,  sizeof shortlength, &shortlength);
			if (!FileError) {
				ReadFile(h, shortlength, Buffer);
				KarteLaden(shortlength);
			}
		}

		for (i = 1; i <= AnzahlGegen; i++)
			Level[Gegenstand[i].x, Gegenstand[i].y].Spezial |= LevelGegenstand;
		if (Editor)
			for (i = 1; i <= AnzahlMonster; i++)
				Level[Monster[i].x,Monster[i].y].Spezial |= LevelMonster;
		else
			for (i = 1; i <= AnzahlMonster; i++)
				if (Status > 0 && Status < 1000)
					Level[Monster[i].x,Monster[i].y].Spezial |= LevelMonster;
		for (i = 1; i <= AnzahlParameter; i++) {
			Level[Parameter[i].x, Parameter[i].y].Spezial |= LevelParameter;
			if (Art == FLicht)
				SetOneLight(Parameter[i].x, Parameter[i].y, Weite, TRUE);
		}

	} else { /* Level speichern */

		h = CreateFile(Name);
		if (FileError) { return; }

		Count = sizeof LevelPars;
		MakeLevelPars;
		WriteBlock(h, Count, &LevelPars);

		shortlength = MakeBuffer();
		WriteFile(h, sizeof shortlength, &shortlength);
		WriteFile(h, shortlength, Buffer);

		Count = sizeof (MonsterTyp) * AnzahlMonster;
		WriteBlock(h, Count, &Monster);
		Count = sizeof (GegenstandTyp) * AnzahlGegen;
		WriteBlock(h, Count, &Gegenstand);
		Count = sizeof ParameterTyp * AnzahlParameter;
		WriteBlock(h, Count, &Parameter);
		
		if (!Editor) { /* Karte speichern */
			shortlength = KarteSpeichern();
			WriteFile(h, sizeof shortlength, &shortlength);
			WriteFile(h, shortlength, Buffer);
		}
	}

	CloseFile(h);
}


void Korrektur(char *s, size_t n;)
{
	unsigned i;
	for (i = 0; i <= n; i++) {
		if (s[i] == '\0') return;
		if (s[i] != '.' && s[i] != '\' && s[i] != ':') {
			s[i] = CAP(s[i]);
			if ((s[i] < 'A' || s[i] > 'Z')
				&& (s[i] < '0' || s[i] > '9'))
				s[i] = '_';
		}
		if (i > 7) s[i] = '\0';
	}
}

void MakeLevelName(String60Typ s, unsigned org, n)
{
	unsigned i;
	switch (org) {
	case 0 : /* Spielerlevel */
		Assign(s, Spieler.Name);
		Korrektur(s);
		Concat(s, s, ".");
		Concat(s, PlaPath, s);
		break;
	case 1 : /* Originallevel */
		Concat(s, MapPath, "MAP.");
		break;
	case 2 : /* Backuplevel */
		Concat(s, MapPath, "OLD.");
		break;
	}
	if (n == 0)
		Concat(s, s, ".???");
	else {
		i = 0;
		while (s[i]) i++
		while (s[i] != ".") i--
		s[i+1] = n / 100 + '0';
		s[i+2] = (n % 100) / 10 + '0;
		s[i+3] = n % 10 + '0';
		s[i+4] = '\0';
	}
}


void SaveLevel(unsigned n)
{
	String60Typ LName, backup;
	if (Editor) {
		MakeLevelName(backup, 2, n);
		DeleteFile(backup); /* Backup löschen */
		MakeLevelName(LName, 1, n);
		RenameFile(LName, backup);
	} else {
		MakeLevelName(LName, 0, n);
		Spieler.OldLevels |= n;
	}
	LoadOrSaveLevel(FALSE, LName);
	if (FileError) {
		Concat(LName, "Level Schreibfehler: ", LName); Error(LName, 0);
	}
}

void LoadLevel(unsigned n)
{
	String60Typ s, LName;
	if (n & Spieler.OldLevels)
		MakeLevelName(LName, 0, n); /* saved Level */
	else
		MakeLevelName(LName, 1, n); /* original Level */
	LoadOrSaveLevel(TRUE, LName); /* Player Level */
	if (FileError) {
		Concat(s, "Level Lesefehler: ", LName); Error(s, 0);
	} else {
		Spieler.Sichtweite = SetLightRange();
		if (Compare(LevelSprites, ""))
			LevelSprites = "HASCSIII";
		Concat(s, LevelSprites, ".SPR");
		if (!Compare(LastSprites, s)) {
			NewSprites = TRUE;
			LoadOrSaveSprites(TRUE, s);
			Concat(s, LevelSprites, ".DAT");
			LoadOrSaveDat(TRUE, s);
		}
	}
}

void DeleteLevels(void)
{
	String60Typ s;
	MakeLevelName(s, 0, 0);
	do DeleteFile(s); while (FileError);
}


/* Spieler ***************************************************************/

void LoadOrSavePlayer(int Load);
{
	int h; String60Typ s;
	Buffer = GetBuffer(BufferSize);
	Assign(s, Spieler.Name);
	Korrektur(s);
	Concat(s, PlaPath, s);
	Concat(s, s, ".PLA");

	if (Load) { 
		h = OpenFile(s);
		if (FileError) {
			Concat(s, "Spieler Lesefehler: ", s); Error(s, 0); return
		}
		ReadBlock(h, sizeof Spieler, &Spieler);
	} else {
		h = CreateFile(s);
		if (FileError) {
			Concat(s, "Spieler Schreibfehler: ", s); Error(s, 0);
		}
		WriteBlock(h, sizeof Spieler, &Spieler);
	}
	CloseFile(h);
}

void LoadOldPlayer(void);
{
	unsigned long Count; unsigned i; String60Typ s; int b;
	LoadOrSavePlayer(TRUE);
	Spieler.Gold = 0; Spieler.Nahrung = 0; Spieler.Moves = 0;
	Spieler.Permanent = STot|SAusruhen|SReitet|SMann;
	Spieler.Status = Spieler.Status & (SMann|SReitet);
	Spieler.AnzGegenstaende = 0; Spieler.LevelNumber = 1;
	Spieler.rechteHand.KennNummer = 0; Spieler.linkeHand.KennNummer = 0;
	Spieler.Ruestung.KennNummer = 0; Spieler.Ring.KennNummer = 0;
	for (i = 1; i <= MaxRuck; i++)
		Rucksack[i].KennNummer = 0;
	for (i = 1; i <= MaxFlags; i++)
		Flags[i] = 0;
	OldLevels = 0;
}


/* Leveldatei *************************************************/

void Skip(CharPtr p, char c)
{
	while (*p != c && *p >= ' ') p++;
	if (*p != '\0') p++;
}

void GetLine(CharPtr p, char *s)
{
	unsigned i;
	i = 0;
	while (*p >= ' ') {
		s[i] = *p; i++; p++;
	}
	s[i] = '\0';
}

unsigned GetCard(CharPtr p)
{
	unsigned v;
	v = 0;
	while (*p == ' ' || *p == '#') p++;
	while (*p >= '0' && *p <= '9') {
		v = v * 10 + *p - '0';
		p++;
	}
	return v;
}

int NextLine(CharPtr p)
{
	while (*p >= ' ') p++;
	while (*p != '\0' && *p != 10) p++;
	if (*p != 0) p++;
	return *p != '\0';
}

void SetString(CharPtr p, char *s, size_t n)
{
	unsigned i;
	for (i = 0; i <= n; i++) {
		if (s[i] == '\0') return;
		*p++ = s[i];
	}
}

void SetCard(CharPtr p, unsigned c, l)
{
	String20Typ s;
	CardToString(c, l, s);
	SetString(p, s);
	if (l != 1) SetString(p, "#");
}

void SetLF(CharPtr p )
{
	*p++ = 13;
	*p++ = 10;
}

void WriteLevel(char *Name)
{
	CharPtr p;
	unsigned i;
	int h;
		
	void WriteMonster(MonsterTyp *m)
	{
		SetString(p, "M#");
		SetCard(p, m->x, 4); SetCard(p, m->y, 4); SetCard(p, m->Typ, 4); SetCard(p, m->Status, 4);
		SetCard(p, m->Trefferwurf, 4); SetCard(p, m->Schaden, 4); SetCard(p, m->Bonus, 4);
		SetCard(p, m->TP, 4); SetCard(p, m->Sprich, 4); SetCard(p, m->Spezial, 4);
		SetString(p, m->Name);
		SetLF(p);
	}

	void WriteGegenstand(GegenstandTyp *g)
	{
		SetString(p, "G#");
		SetCard(p, g->x, 4); SetCard(p, g->y, 4); SetCard(p, g->Sprite, 4);
		SetCard(p, g->Flags, 4); SetCard(p, g->Spezial, 4);
		SetCard(p, g->KennNummer, 4); SetCard(p, g->Ring, 4); SetCard(p, g->RingWirkung, 4);
		SetCard(p, g->RingDauer, 4); SetString(p, g->Name);
		SetLF(p);
	}

	void WriteParameter(ParameterTyp *a)
	{
		SetString(p, "P#");
		SetCard(p, a->x, 4); SetCard(p, a->y, 4); SetCard(p, a->Art, 4);
		SetCard(p, a->xhoch, 4); SetCard(p, a->yhoch, 4); SetCard(p, a->Levelhoch, 4);
		SetCard(p, a->xrunter, 4); SetCard(p, a->yrunter, 4); SetCard(p, a->Levelrunter, 4);
		SetLF(p);
	}

{
	Buffer = GetBuffer(BufferSize);
	p = Buffer;
	SetString(p, "*    x    y  Typ Stat Tref Scha Bonu   TP  Dia Spez Name");
	SetLF(p);
	for (i = 1; i <= AnzahlMonster; i++) WriteMonster(Monster[i]);
	SetString(p, "*    x    y Must Flag Spez  Typ   P1   P2   P3 Name");
	SetLF(p);
	for (i = 1; i <= AnzahlGegen; i++) WriteGegenstand(Gegenstand[i]);
	SetString(p, "*    x    y  Typ   P1   P2   P3   P4   P5   P6");
	SetLF(p);
	for (i = 1; i <= AnzahlParameter; i++) WriteParameter(Parameter[i]);
/*
	for (i = 0; i <= MaxSprites-1; i++) {
		SetString(p, "#FELD.NAME("); SetCard(p, i, 1); SetString(p, ') = "');
		SetString(p, Felder[i].Name); SetString(p, '"'); SetLF(p);
		SetString(p, "#FELD.SPEZIAL("); SetCard(p, i, 1); SetString(p, ') = ');
		SetCard(p, Felder[i].Spezial % 128, 1); SetLF(p);
	}
*/
	h = CreateFile(Name);
	WriteFile(h, p - Buffer, Buffer);
	CloseFile(h);
}

void ReadLevel(char *Name)
{
	unsigned long l;
	int h;
	CharPtr p;

	void ReadMonster(void)
	{
		MonsterTyp m;
		p++;
		m.x = GetCard(p); m.y = GetCard(p); m.Typ = GetCard(p); m.Status = GetCard(p);
		m.Trefferwurf = GetCard(p); m.Schaden = GetCard(p); m.Bonus = GetCard(p);
		m.TP = GetCard(p); m.Sprich = GetCard(p); m.Spezial = GetCard(p);
		Skip(p, '#'); GetLine(p, m.Name);
		NewMonster(m.x, m.y, m);
	}
	
	void ReadGegen(void)
	{
		GegenstandTyp g;
		p++;
		g.x = GetCard(p); g.y = GetCard(p); g.Sprite = GetCard(p);
		g.Flags = GetCard(p); g.Spezial = GetCard(p);
		g.KennNummer = GetCard(p); g.Ring = GetCard(p); g.RingWirkung = GetCard(p);
		g.RingDauer = GetCard(p); Skip(p, '#'); GetLine(p, g.Name);
		NewGegenstand(g.x, g.y, g);
	}
	
	void ReadParameter(void)
	{
		ParameterTyp o;
		p++;
		o.x = GetCard(p); o.y = GetCard(p); o.Art = GetCard(p);
		o.xhoch = GetCard(p); o.yhoch = GetCard(p); o.Levelhoch = GetCard(p);
		o.xrunter = GetCard(p); o.yrunter = GetCard(p); o.Levelrunter = GetCard(p);
		NewParameter(o.x, o.y, o);
	}

	l = FileLength(Name);
	if (FileError) return;
	Buffer = GetBuffer(l);
	h = OpenFile(Name); ReadFile(h, l, Buffer); CloseFile(h);
	while (AnzahlMonster > 0) DeleteMonster(Monster[1].x, Monster[1].y);
	while (AnzahlGegen > 0) DeleteGegenstand(Gegenstand[1].x, Gegenstand[1].y);
	while (AnzahlParameter > 0) DeleteParameter(Parameter[1].x, Parameter[1].y);
	p = Buffer;
	do {
		switch (*p) {
		case 'M' : ReadMonster(); break;
		case 'G' : ReadGegen(); break;
		case 'P' : ReadParameter(); break;
		}
	} while (NextLine(p));
}


void InitDisk(void)
{
	/* VersionsNummer = 7; erste Farbversion */
	/* VersionsNummer = 8; HASCS II schwarz-weiß */
	/* VersionsNummer = 9; HASCS III */
	VersionsNummer = 10; /* mit Karte */
	
	LastSprites = "";
	NewSprites = FALSE;

	Buffer = GetBuffer(BufferSize);
}

