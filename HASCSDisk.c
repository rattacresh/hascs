/* HASCSDisk module */
#include "compat.h"
#include "HASCSDisk.h"

/* HASCS Importe */
#include "HASCSGlobal.h"
#include "HASCSGraphics.h"
#include "HASCSSystem.h"
#include "HASCSOutput.h"


#define BufferSize 40000


typedef char *CharPtr;

static char LevelPars[100];

static String60Typ LastSprites;

static unsigned long ZufallsZahl;
static unsigned LevelVersion;

static char *Buffer;

/* Fehlermeldung *********************************************************/

static void Fehler(char msg[], char file[])
{
	char s[256];
	Concat(s, msg, file);
	Error(s, 0);
}


/* Kodierung *************************************************************/

static unsigned PseudoZufall(unsigned n)
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
	/* Unspecified order, but that's okay here --rtc */
	ZufallsZahl = ((long)(Zufall(256)-1)*256+(long)(Zufall(256)-1))*256+
		(long)(Zufall(256)-1);
	start = ZufallsZahl; /* Startwert */
	for (i = 0; i <= anzahl-1; i++) {
		pruef = (pruef * 3 + (unsigned long)*a) % 65536;
		Buffer[i] = ((unsigned)*a++ + PseudoZufall(256)) % 256;
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
	ZufallsZahl = (long)Buffer[anzahl] * 65536 + 
		(long)Buffer[anzahl+1] * 256 +
		(long)Buffer[anzahl+2];
	test = (long)Buffer[anzahl+3] * 256 +
		(long)Buffer[anzahl+4];
	for (i = 0; i <= anzahl-1; i++) {
		*a = ((unsigned)Buffer[i] + 256 - PseudoZufall(256)) % 256;
		pruef = (pruef * 3 + (unsigned long)*a++) % 65536;
	}
	if (test != pruef && !Editor)
		Error("Prüfsummenfehler!", -1);
}

static unsigned char V1Secret[] = {
	0xC1, 0x61, 0x4B, 0x21,
	0xB7, 0xC9, 0x80, 0x41,
	0x4E, 0x32, 0x24, 0x3C,
	0x3F, 0xE4, 0x92, 0x00
};

void V1CodeBuffer(unsigned code, unsigned long size, CharPtr Buffer)
{
	unsigned char *p;

	if (code >= sizeof V1Secret)
		code = 0;
	p = V1Secret + code % (sizeof V1Secret - 1);
	while (size) {
		*Buffer = (*Buffer + *p++)%256;
		Buffer++;
		if (*p == '\0')
			p = V1Secret;
		size--;
	}
}

void V1DecodeBuffer(unsigned code, unsigned long size, CharPtr Buffer)
{
	unsigned char *p;

	if (code >= sizeof V1Secret)
		code = 0;
	p = V1Secret + code % (sizeof V1Secret - 1);
	while (size) {
		*Buffer = (*Buffer - *p++)%256;
		Buffer++;
		if (*p == '\0')
			p = V1Secret;
		size--;
	}
}

void V1DecodeFile(unsigned code, char *s, char *t)
{
	int f;
	char *p;
	unsigned long l;
	l = FileLength(s); if (l == 0) return;
	f = OpenFile(s);
	if (FileError)
		return;
	p = GetBuffer(BufferSize);
	ReadFile(f, l, p);
	CloseFile(f);

	V1DecodeBuffer(code, l, p);
	
	f = CreateFile(t); WriteFile(f, l, p); CloseFile(f);
}

/* Serialisierung ********************************************************/

static void MakeGegen(GegenstandTyp *g, CharPtr Buffer)
{
	Buffer[0] = g->x / 256;
	Buffer[1] = g->x % 256;
	Buffer[2] = g->y / 256;
	Buffer[3] = g->y % 256;
	strncpy(Buffer + 4, g->Name, sizeof g->Name);
	Buffer[26] = g->Flags / 256;
	Buffer[27] = g->Flags % 256;
	Buffer[28] = g->Dialog / 256;
	Buffer[29] = g->Dialog % 256;
	Buffer[30] = g->Sprite / 256;
	Buffer[31] = g->Sprite % 256;
	Buffer[32] = g->Spezial / 256;
	Buffer[33] = g->Spezial % 256;
	Buffer[34] = g->KennNummer / 256;
	Buffer[35] = g->KennNummer % 256;
	Buffer[36] = g->Ring / 256;
	Buffer[37] = g->Ring % 256;
	Buffer[38] = g->RingWirkung / 256;
	Buffer[39] = g->RingWirkung % 256;
	Buffer[40] = g->RingDauer / 256;
	Buffer[41] = g->RingDauer % 256;
}

static void MakeMonster(MonsterTyp *m, CharPtr Buffer)
{
	strncpy(Buffer, m->Name, sizeof m->Name);
	Buffer[22] = m->Trefferwurf / 256;
	Buffer[23] = m->Trefferwurf % 256;
	Buffer[24] = m->Schaden / 256;
	Buffer[25] = m->Schaden % 256;
	Buffer[26] = m->Bonus / 256;
	Buffer[27] = m->Bonus % 256;
	Buffer[28] = m->x / 256;
	Buffer[29] = m->x % 256;
	Buffer[30] = m->y / 256;
	Buffer[31] = m->y % 256;
	Buffer[32] = m->Typ / 256;
	Buffer[33] = m->Typ % 256;
	Buffer[34] = m->Status / 256;
	Buffer[35] = m->Status % 256;
	Buffer[36] = m->TP / 256;
	Buffer[37] = m->TP % 256;
	Buffer[38] = m->Sprich / 256;
	Buffer[39] = m->Sprich % 256;
	Buffer[40] = m->Spezial / 256;
	Buffer[41] = m->Spezial % 256;
}

static void AuswertGegen(GegenstandTyp *g, CharPtr Buffer)
{
	g->x = Buffer[0]*256+Buffer[1];
	g->y = Buffer[2]*256+Buffer[3];
	strncpy(g->Name, Buffer + 4, sizeof g->Name);
	g->Flags = Buffer[26]*256+Buffer[27];
	g->Dialog = Buffer[28]*256+Buffer[29];
	g->Sprite = Buffer[30]*256+Buffer[31];
	g->Spezial = Buffer[32]*256+Buffer[33];
	g->KennNummer = Buffer[34]*256+Buffer[35];
	g->Ring = Buffer[36]*256+Buffer[37];
	g->RingWirkung = Buffer[38]*256+Buffer[39];
	g->RingDauer = Buffer[40]*256+Buffer[41];
}

static void AuswertMonster(MonsterTyp *m, CharPtr Buffer)
{
	strncpy(m->Name, Buffer, sizeof m->Name);
	m->Trefferwurf = Buffer[22]*256+Buffer[23];
	m->Schaden = Buffer[24]*256+Buffer[25];
	m->Bonus = Buffer[26]*256+Buffer[27];
	m->x = Buffer[28]*256+Buffer[29];
	m->y = Buffer[30]*256+Buffer[31];
	m->Typ = Buffer[32]*256+Buffer[33];
	m->Status = Buffer[34]*256+Buffer[35];
	m->TP = Buffer[36]*256+Buffer[37];
	m->Sprich = Buffer[38]*256+Buffer[39];
	m->Spezial = Buffer[40]*256+Buffer[41];
}

static void DoMonsterList(MonsterTyp Monster[], unsigned AnzahlMonster,
	CharPtr Buffer, void (*Do)(MonsterTyp *, CharPtr))
{
	int i;
	for (i = 0; i < AnzahlMonster; i++)
		Do(&Monster[i], Buffer + i * 42);
}

static void DoGegenList(GegenstandTyp Gegen[], unsigned AnzahlGegen,
	CharPtr Buffer, void (*Do)(GegenstandTyp *, CharPtr))
{
	int i;
	for (i = 0; i < AnzahlGegen; i++)
		Do(&Gegen[i], Buffer + i * 42);
}

/* Felder ****************************************************************/

void LoadOrSaveDat(int Load, char *FileName)
{
	int h;
	String60Typ s;

	void MakeFeld(FeldTyp *f, CharPtr Buffer)
	{
		strncpy(f->Name, Buffer, sizeof f->Name);
		Buffer[22] = f->Spezial / 256;
		Buffer[23] = f->Spezial % 256;
	}

	void AuswertFeld(FeldTyp *f, CharPtr Buffer)
	{
		strncpy(f->Name, Buffer, sizeof f->Name);
		f->Spezial = Buffer[22]*256+Buffer[23];
	}

	void DoFeldList(FeldTyp Feld[], unsigned AnzahlFeld,
		CharPtr Buffer, void (*Do)(FeldTyp *, CharPtr))
	{
		int i;
		for (i = 0; i < AnzahlFeld; i++)
			Do(&Feld[i], Buffer + i * 24);
	}

	Buffer = GetBuffer(BufferSize);
	Concat(s, PrgPath, FileName);

	/* V1 DAT format:
	 *
	 * 2800 block= 100 records, 28 bytes each
	 *    Waffen
	 *    22 bytes name
	 *    2 bytes Schaden?
	 *    2 bytes Bonus?
	 *    2 bytes Anwendungen?
	 *    index 0 = Hand
	 * 2600 block = 100 records, 26 bytes each
	 *    Rüstungen
	 *    22 bytes name
	 *    2 bytes Schutz?
	 *    2 bytes Bonus?
	 * 2080 block = 80 records, 26 bytes each
	 *    Monster
	 *    22 bytes name
	 *    2 bytes Schaden?
	 *    2 bytes Bonus?
	 * 1920 block = 80 records, 24 bytes each
	 *    Felder
	 *    22 bytes name
	 *    2 bytes Spezial
	 * total 9400 bytes
	 *
	 * separate using:
	 *
	 *  dd if=HASCS.DAT of=A.DAT bs=1 count=2800
	 *  dd if=HASCS.DAT of=B.DAT bs=1 count=2600 skip=2800
	 *  dd if=HASCS.DAT of=C.DAT bs=1 count=2080 skip=5400
	 *  dd if=HASCS.DAT of=D.DAT bs=1 count=1920 skip=7480
	 * 
	 * Then decode using V1CodeBuffer, like calling
	 * V1DecodeFile(0, "D.DAT", "D.DEC")
	 * from the debugger
	 */
	if (Load) {
		h = OpenFile(s);
		if (FileError) {
			Fehler("Felderdaten Lesefehler: ", s); return;
		}
		ReadBlock(h, 24*MaxSprites, Buffer);
		DoFeldList(Felder, MaxSprites, Buffer, AuswertFeld);
		if (Editor) {
			ReadBlock(h, 42*MaxSprites, Buffer);
			DoMonsterList(MonsterKlasse, MaxSprites,
				Buffer,	AuswertMonster);
			ReadBlock(h, 42*MaxSprites, Buffer);
			DoGegenList(GegenKlasse, MaxSprites,
				Buffer,AuswertGegen);
		}
	} else {
		h = CreateFile(s);
		if (FileError) {
			Fehler("Felderdaten Schreibfehler: ", s); return;
		}
		DoFeldList(Felder, MaxSprites, Buffer, MakeFeld);
		WriteBlock(h, 24*MaxSprites, Buffer);
		DoMonsterList(MonsterKlasse, MaxSprites, Buffer, MakeMonster);
		WriteBlock(h, 42*MaxSprites, Buffer);
		DoGegenList(GegenKlasse, MaxSprites, Buffer, MakeGegen);
		WriteBlock(h, 42*MaxSprites, Buffer);
	}
	CloseFile(h);
}

void LoadOrSaveSprites(int Load, char *FileName)
{
	int h;
	unsigned long Count;
	String60Typ s;
	char SpriteArrayBuf[2*(sizeof (SpriteType) / sizeof (uint16_t))
		* MaxSprites];
	
	void MakeSpriteArrayBuf(SpriteArrayType s)
	{
		int i, j, k;
		for (i = k = 0; i < MaxSprites; i++)
			for (j = 0; 
			     j < sizeof (SpriteType) / sizeof (uint16_t);
			     j++, k+=2)
				SpriteArrayBuf[k] = s[i][j] / 256;
				SpriteArrayBuf[k+1] = s[i][j] % 256;
	}

	void AuswertSpriteArrayBuf(SpriteArrayType s)
	{
		int i, j, k;
		for (i = k = 0; i < MaxSprites; i++)
			for (j = 0;
			     j < sizeof (SpriteType) / sizeof (uint16_t);
			     j++, k+=2)
			{
				s[i][j] = SpriteArrayBuf[k]*256
					+ SpriteArrayBuf[k+1];
			}
	}

	Assign(LastSprites, FileName);
	Concat(s, PrgPath, FileName);
	if (Load) {
		h = OpenFile(s);
		if (FileError) {
			Fehler("Muster Lesefehler: ", s); return;
		}
		Count = sizeof SpriteArrayBuf;
		ReadFile(h, Count, SpriteArrayBuf);
		AuswertSpriteArrayBuf(FelderSprite);
		ReadFile(h, Count, SpriteArrayBuf);
		AuswertSpriteArrayBuf(MonsterSprite);
		ReadFile(h, Count, SpriteArrayBuf);
		AuswertSpriteArrayBuf(SystemSprite);
		ReadFile(h, Count, SpriteArrayBuf);
		if (FileError) /* HASCS II Sprite Sets don't have System */
			return;
		AuswertSpriteArrayBuf(GegenSprite);
	} else {
		h = CreateFile(s);
		if (FileError) {
			Fehler("Muster Schreibfehler: ", s); return;
		}
		Count = sizeof SpriteArrayBuf;
		MakeSpriteArrayBuf(FelderSprite);
		WriteFile(h, Count, SpriteArrayBuf);
		MakeSpriteArrayBuf(MonsterSprite);
		WriteFile(h, Count, SpriteArrayBuf);
		MakeSpriteArrayBuf(SystemSprite);
		WriteFile(h, Count, SpriteArrayBuf);
		MakeSpriteArrayBuf(GegenSprite);
		WriteFile(h, Count, SpriteArrayBuf);
	}
	CloseFile(h);
}


/* Level *****************************************************************/

void LoadOrSaveLevel(int Load, String60Typ Name)
{
	unsigned long Count;
	unsigned shortlength, i;
	char ShortBuffer[2];
	/*String60Typ s;*/
	int h;

	void MakeShort(void)
	{
		ShortBuffer[0] = shortlength / 256;
		ShortBuffer[1] = shortlength % 256;
	}

	void AuswertShort(void)
	{
		shortlength = ShortBuffer[0] * 256 + ShortBuffer[1];
	}

	void MakeParameter(ParameterTyp *p, CharPtr Buffer)
	{
		Buffer[0] = p->x / 256;
		Buffer[1] = p->x % 256;
		Buffer[2] = p->y / 256;
		Buffer[3] = p->y % 256;
		Buffer[4] = p->Art / 256;
		Buffer[5] = p->Art % 256;
		Buffer[6] = p->xhoch / 256;
		Buffer[7] = p->xhoch % 256;
		Buffer[8] = p->yhoch / 256;
		Buffer[9] = p->yhoch % 256;
		Buffer[10] = p->Levelhoch / 256;
		Buffer[11] = p->Levelhoch % 256;
		Buffer[12] = p->xrunter / 256;
		Buffer[13] = p->xrunter % 256;
		Buffer[14] = p->yrunter / 256;
		Buffer[15] = p->yrunter % 256;
		Buffer[16] = p->Levelrunter / 256;
		Buffer[17] = p->Levelrunter % 256;
	}

	void AuswertParameter(ParameterTyp *p, CharPtr Buffer)
	{
		p->x = Buffer[0]*256+Buffer[1];
		p->y = Buffer[2]*256+Buffer[3];
		p->Art = Buffer[4]*256+Buffer[5];
		p->xhoch = Buffer[6]*256+Buffer[7];
		p->yhoch = Buffer[8]*256+Buffer[9];
		p->Levelhoch = Buffer[10]*256+Buffer[11];
		p->xrunter = Buffer[12]*256+Buffer[13];
		p->yrunter = Buffer[14]*256+Buffer[15];
		p->Levelrunter = Buffer[16]*256+Buffer[17];
	}

	void DoParameterList(ParameterTyp Parameter[], 
		unsigned AnzahlParameter, 
		CharPtr Buffer, void (*Do)(ParameterTyp *, CharPtr))
	{
		int i;
		for (i = 0; i < AnzahlParameter; i++)
			Do(&Parameter[i], Buffer + i * 18);
	}

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
						for (i = 1; i <= anzahl; i++)
							Buffer[counter++] = old;
					else {
						Buffer[counter++] = 255;
						Buffer[counter++] = anzahl;
						Buffer[counter++] = old;
					}
					anzahl = 1;
					old = Level[x][y].Feld;
				}
			}
		}
		if (anzahl < 3)
			for (i = 1; i <= anzahl; i++) {
				Buffer[counter++] = old;
			}
		else {
			Buffer[counter++] = 255;
			Buffer[counter++] = anzahl;
			Buffer[counter++] = old;
		}
		return counter;
	}

	void AuswertLevelPars(void)
	{
		unsigned x/*, p*/;
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
				Level[x][y].Spezial |= LevelKarte;
			x++;
			if (x > LevelBreite) {
				x = 0; y++;
			}
			c--;
		}
	}
	
	unsigned KarteSpeichern()
	{
		unsigned x, y, c, i, k, n;
		x = 1; y = 0; i = 0; k = 0; c = 1;
		if (LevelKarte & Level[0][0].Spezial) k = 128;
		while (x <= LevelBreite && y <= LevelHoehe) {
			n = 0;
			if (LevelKarte & Level[x][y].Spezial) n = 128;
			if (n != k || c >= 127) {
				Buffer[i] =k + c;
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
		if (FileError) return;
		Count = sizeof LevelPars;
		ReadBlock(h, Count, LevelPars);
		AuswertLevelPars();

		ReadFile(h, 2, ShortBuffer);
		AuswertShort();
		ReadFile(h, shortlength, Buffer);
		AuswertBuffer(shortlength);

		Count = 42 * AnzahlMonster;
		ReadBlock(h, Count, Buffer);
		DoMonsterList(Monster + 1, AnzahlMonster,
			Buffer, AuswertMonster);
		Count = 42 * AnzahlGegen;
		ReadBlock(h, Count, Buffer);
		DoGegenList(Gegenstand + 1, AnzahlGegen,
			Buffer, AuswertGegen);
		Count = 18 * AnzahlParameter;
		ReadBlock(h, Count, Buffer);
		DoParameterList(Parameter + 1, AnzahlParameter,
				Buffer, AuswertParameter);

		if (!Editor) { /* Karte laden */
			ReadFile(h,  2, ShortBuffer);
			AuswertShort();
			if (!FileError) {
				ReadFile(h, shortlength, Buffer);
				KarteLaden(shortlength);
			}
		}

		for (i = 1; i <= AnzahlGegen; i++)
			Level[Gegenstand[i].x][Gegenstand[i].y].Spezial |= LevelGegenstand;
		if (Editor)
			for (i = 1; i <= AnzahlMonster; i++)
				Level[Monster[i].x][Monster[i].y].Spezial |= LevelMonster;
		else
			for (i = 1; i <= AnzahlMonster; i++)
				if (Monster[i].Status > 0 && Monster[i].Status < 1000)
					Level[Monster[i].x][Monster[i].y].Spezial |= LevelMonster;
		for (i = 1; i <= AnzahlParameter; i++) {
			Level[Parameter[i].x][Parameter[i].y].Spezial |= LevelParameter;
			if (Parameter[i].Art == FLicht)
				SetOneLight(Parameter[i].x, Parameter[i].y, Parameter[i].Weite, TRUE);
		}

	} else { /* Level speichern */

		h = CreateFile(Name);
		if (FileError) return;

		Count = sizeof LevelPars;
		MakeLevelPars();
		WriteBlock(h, Count, LevelPars);

		shortlength = MakeBuffer();
		MakeShort();
		WriteFile(h, 2, ShortBuffer);
		WriteFile(h, shortlength, Buffer);

		DoMonsterList(Monster + 1, AnzahlMonster, Buffer, MakeMonster);
		Count = 42 * AnzahlMonster;
#if 1 /* HASCS III */
		WriteBlock(h, Count, Buffer);
#else
		WriteFile(h, Count, Buffer);
#endif
		DoGegenList(Gegenstand + 1, AnzahlGegen, Buffer, MakeGegen);
		Count = 42 * AnzahlGegen;
#if 1 /* HASCS III */
		WriteBlock(h, Count, Buffer);
#else
		WriteFile(h, Count, Buffer);
#endif
		DoParameterList(Parameter + 1, AnzahlParameter, 
				Buffer, MakeParameter);
		Count = 18 * AnzahlParameter;
#if 1 /* HASCS III */
		WriteBlock(h, Count, Buffer);
#else
		WriteFile(h, Count, Buffer);
#endif
		
		if (!Editor) { /* Karte speichern */
			shortlength = KarteSpeichern();
			MakeShort();
			WriteFile(h, 2, ShortBuffer);
			WriteFile(h, shortlength, Buffer);
		}
	}

	CloseFile(h);
}


static void Korrektur(char *s)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (s[i] == '\0') return;
		if (s[i] != '.' && s[i] != DIRSEPCHR && s[i] != ':') {
			s[i] = CAP(s[i]);
			if ((s[i] < 'A' || s[i] > 'Z')
				&& (s[i] < '0' || s[i] > '9'))
				s[i] = '_';
		}
		if (i > 7) s[i] = '\0';
	}
}

static void MakeLevelName(String60Typ s, unsigned org, unsigned n)
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
		Concat(s, s, "???");
	else {
		i = 0;
		while (s[i] != '\0') i++;
		while (s[i] != '.') i--;
		s[i+1] = n / 100 + '0';
		s[i+2] = (n % 100) / 10 + '0';
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
		Spieler.OldLevels[n/8] |= 1<<(n%8);
	}
	LoadOrSaveLevel(FALSE, LName);
	if (FileError) {
		Concat(LName, "Level Schreibfehler: ", LName); Error(LName, 0);
	}
}

void LoadLevel(unsigned n)
{
	String60Typ s, LName;
	if (1<<(n%8) & Spieler.OldLevels[n/8])
		MakeLevelName(LName, 0, n); /* saved Level */
	else
		MakeLevelName(LName, 1, n); /* original Level */
	LoadOrSaveLevel(TRUE, LName); /* Player Level */
	if (FileError) {
		Concat(s, "Level Lesefehler: ", LName); Error(s, 0);
	} else {
		Spieler.Sichtweite = SetLightRange();
		if (Compare(LevelSprites, ""))
			Assign(LevelSprites, "HASCSIII");
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


static void CodeBuffer(unsigned long code, unsigned size, char *Buffer)
{
	SetzeZufall(code);
	while (size > 0) {
		*Buffer++ ^= Zufall(256) - 1;
		size--;
	}
	SetzeZufall(0L);
}

static unsigned long MakeCode(char *s)
{
	unsigned long code = 290568; int i;
	for (i = 0; i < HIGH(s); i++) {
		if (s[i] == '\0')
			return code;
		code += s[i];
	}
	return code;
}


/* Spieler ***************************************************************/

void LoadOrSavePlayer(int Load)
{
	int h; 
	String60Typ s;
	unsigned long code;
	char SpielerPars[1292];

	void MakeSpieler(void)
	{
		CharPtr SpielerPars2, SpielerPars3;
		int i;
		Assign(SpielerPars, Spieler.Name);
		SpielerPars[22] = Spieler.x / 256;
		SpielerPars[23] = Spieler.x % 256;
		SpielerPars[24] = Spieler.y / 256;
		SpielerPars[25] = Spieler.y % 256;
		SpielerPars[26] = Spieler.TPMax / 256;
		SpielerPars[27] = Spieler.TPMax % 256;
		SpielerPars[28] = Spieler.TP / 256;
		SpielerPars[29] = Spieler.TP % 256;
		SpielerPars[30] = Spieler.Gold / 256;
		SpielerPars[31] = Spieler.Gold % 256;
		SpielerPars[32] = Spieler.Nahrung / 256;
		SpielerPars[33] = Spieler.Nahrung % 256;
		SpielerPars[34] = Spieler.Grad / 256;
		SpielerPars[35] = Spieler.Grad % 256;
		SpielerPars[36] = Spieler.St / 256;
		SpielerPars[37] = Spieler.St % 256;
		SpielerPars[38] = Spieler.Ko / 256;
		SpielerPars[39] = Spieler.Ko % 256;
		SpielerPars[40] = Spieler.Ge / 256;
		SpielerPars[41] = Spieler.Ge % 256;
		SpielerPars[42] = Spieler.In / 256;
		SpielerPars[43] = Spieler.In % 256;
		SpielerPars[44] = Spieler.Zt / 256;
		SpielerPars[45] = Spieler.Zt % 256;
		SpielerPars[46] = Spieler.Ch / 256;
		SpielerPars[47] = Spieler.Ch % 256;
		SpielerPars[48] = Spieler.Sprite / 256;
		SpielerPars[49] = Spieler.Sprite % 256;
		SpielerPars[50] = Spieler.Sichtweite / 256;
		SpielerPars[51] = Spieler.Sichtweite % 256;
		SpielerPars[52] = Spieler.AnzGegenstaende / 256;
		SpielerPars[53] = Spieler.AnzGegenstaende % 256;
		SpielerPars[54] = Spieler.LevelNumber / 256;
		SpielerPars[55] = Spieler.LevelNumber % 256;
		SpielerPars[56] = Spieler.Lernen / 256;
		SpielerPars[57] = Spieler.Lernen % 256;

		SpielerPars[58] = Spieler.Status / 256;
		SpielerPars[59] = Spieler.Status % 256;
		SpielerPars[60] = Spieler.Permanent / 256;
		SpielerPars[61] = Spieler.Permanent % 256;
		SpielerPars[62] = Spieler.Typ / 256;
		SpielerPars[63] = Spieler.Typ % 256;

		SpielerPars[64] = 0;
		SpielerPars[65] = (Spieler.EP / 65536) % 256;
		SpielerPars[66] = Spieler.EP / 256;
		SpielerPars[67] = Spieler.EP % 256;
		SpielerPars[68] = 0;
		SpielerPars[69] = (Spieler.EPnext / 65536) % 256;
		SpielerPars[70] = Spieler.EPnext / 256;
		SpielerPars[71] = Spieler.EPnext % 256;
		SpielerPars[72] = 0;
		SpielerPars[73] = (Spieler.Moves / 65536) % 256;
		SpielerPars[74] = Spieler.Moves / 256;
		SpielerPars[75] = Spieler.Moves % 256;

		MakeMonster(&Spieler.ReitTier, SpielerPars + 76);

		MakeGegen(&Spieler.rechteHand, SpielerPars + 118);

		MakeGegen(&Spieler.linkeHand, SpielerPars + 160);
		MakeGegen(&Spieler.Ruestung, SpielerPars + 202);
		MakeGegen(&Spieler.Ring, SpielerPars + 244);

		for (i = 1; i <= MaxRuck; i++)
			MakeGegen(&Spieler.Rucksack[i], 
				SpielerPars + 286 + (i - 1) * 42);

		SpielerPars2 = SpielerPars + 286 + MaxRuck * 42;
		for (i = 1; i <= MaxFlags; i++) {
			SpielerPars2[(i-1)*2] = Spieler.Flags[i] / 256;
			SpielerPars2[(i-1)*2+1] = Spieler.Flags[i] % 256;
		}

		SpielerPars3 = SpielerPars2 + MaxFlags * 2;

		memcpy(SpielerPars3, Spieler.OldLevels, 
			sizeof Spieler.OldLevels / 8);
	}
	void AuswertSpieler(void)
	{
		CharPtr SpielerPars2, SpielerPars3;
		int i;
		Assign(Spieler.Name, SpielerPars);
		Spieler.x = SpielerPars[22]*256+SpielerPars[23];
		Spieler.y = SpielerPars[24]*256+SpielerPars[25];
		Spieler.TPMax = SpielerPars[26]*256+SpielerPars[27];
		Spieler.TP = SpielerPars[28]*256+SpielerPars[29];
		Spieler.Gold =  SpielerPars[30]*256+SpielerPars[31];
		Spieler.Nahrung = SpielerPars[32]*256+SpielerPars[33];
		Spieler.Grad = SpielerPars[34]*256+SpielerPars[35];
		Spieler.St = SpielerPars[36]*256+SpielerPars[37];
		Spieler.Ko = SpielerPars[38]*256+SpielerPars[39];
		Spieler.Ge = SpielerPars[40]*256+SpielerPars[41];
		Spieler.In = SpielerPars[42]*256+SpielerPars[43];
		Spieler.Zt = SpielerPars[44]*256+SpielerPars[45];
		Spieler.Ch = SpielerPars[46]*256+SpielerPars[47];
		Spieler.Sprite = SpielerPars[48]*256+SpielerPars[49];
		Spieler.Sichtweite = SpielerPars[50]*256+SpielerPars[51];
		Spieler.AnzGegenstaende = SpielerPars[52]*256+SpielerPars[53];
		Spieler.LevelNumber = SpielerPars[54]*256+SpielerPars[55];
		Spieler.Lernen = SpielerPars[56]*256+SpielerPars[57];

		Spieler.Status = SpielerPars[58]*256+SpielerPars[59];
		Spieler.Permanent = SpielerPars[60]*256+SpielerPars[61];
		Spieler.Typ = SpielerPars[62]*256+SpielerPars[63];

		Spieler.EP = SpielerPars[65]*65536
			+ SpielerPars[66]*256+SpielerPars[67];
		Spieler.EPnext = SpielerPars[69]*65536
			+ SpielerPars[70]*256+SpielerPars[71];
		Spieler.Moves = SpielerPars[73]*65536
			+ SpielerPars[74]*256+SpielerPars[75];
		AuswertMonster(&Spieler.ReitTier, SpielerPars + 76);

		AuswertGegen(&Spieler.rechteHand, SpielerPars + 118);

		AuswertGegen(&Spieler.linkeHand, SpielerPars + 160);
		AuswertGegen(&Spieler.Ruestung, SpielerPars + 202);
		AuswertGegen(&Spieler.Ring, SpielerPars + 244);

		for (i = 1; i <= MaxRuck; i++)
			AuswertGegen(&Spieler.Rucksack[i], 
				SpielerPars + 286 + (i - 1) * 42);

		SpielerPars2 = SpielerPars + 286 + MaxRuck * 42;
		for (i = 1; i <= MaxFlags; i++)
			Spieler.Flags[i] = SpielerPars2[(i-1)*2]*256
				+SpielerPars2[(i-1)*2+1];

		SpielerPars3 = SpielerPars2 + MaxFlags * 2;

		memcpy(Spieler.OldLevels, SpielerPars3, 
			sizeof Spieler.OldLevels / 8);
	}

	Buffer = GetBuffer(BufferSize);
	Assign(s, Spieler.Name);
	Korrektur(s);
	code = MakeCode(s);
	Concat(s, PlaPath, s);
	Concat(s, s, ".PLA");

	/* HASCS I 578 bytes (all payload) */
	/* HASCS II 3399 bytes = 3394 bytes payload + 5 bytes trailer =  */
	/* HASCS III 1.00 bis 1.31 1292 bytes (all payload) */
	/* HASCS III 1.43 1297 bytes = 1929 bytes playload + 5 bytes trailer */
	if (Load) { 
		h = OpenFile(s);
		if (FileError) {
			Concat(s, "Spieler Lesefehler: ", s); Error(s, 0); return;
		}
#if 0 /* HASCSIII 1.00 bis 1.31 */
		ReadFile(h, sizeof SpielerPars, SpielerPars);
		CodeBuffer(SpielerPars, sizeof SpielerPars, code);
#elseif 0 /* HASCS I */
		ReadFile(h, ..., SpielerPars);
		V1DecodeBuffer(SpielerPars, ..., 0);
#else /* HASCS II, HASCS IIII 1.43 */
		ReadBlock(h, sizeof SpielerPars, SpielerPars);
#endif
		AuswertSpieler();
	} else {
		h = CreateFile(s);
		if (FileError) {
			Concat(s, "Spieler Schreibfehler: ", s); Error(s, 0);
		}
		MakeSpieler();
#if 0 /* HASCSIII 1.00 bis 1.31 */
		CodeBuffer(Spieler, SIZE(Spieler), code);
		WriteFile(h, sizeof SpielerPars, SpielerPars);
#elseif 0 /* HASCS I */
		V1CodeBuffer(SpielerPars, ..., 0);
		WriteFile(h, ..., SpielerPars);
#else /* HASCS II, HASCS IIII 1.43 */
		WriteBlock(h, sizeof SpielerPars, SpielerPars);
#endif
	}
	CloseFile(h);
}

void LoadOldPlayer(void)
{
	/*unsigned long Count;*/ unsigned i; /*String60Typ s; int b;*/
	LoadOrSavePlayer(TRUE);
	Spieler.Gold = 0; Spieler.Nahrung = 0; Spieler.Moves = 0;
	Spieler.Permanent = STot|SAusruhen|SReitet|SMann;
	Spieler.Status = Spieler.Status & (SMann|SReitet);
	Spieler.AnzGegenstaende = 0; Spieler.LevelNumber = 1;
	Spieler.rechteHand.KennNummer = 0; Spieler.linkeHand.KennNummer = 0;
	Spieler.Ruestung.KennNummer = 0; Spieler.Ring.KennNummer = 0;
	for (i = 1; i <= MaxRuck; i++)
		Spieler.Rucksack[i].KennNummer = 0;
	for (i = 1; i <= MaxFlags; i++)
		Spieler.Flags[i] = 0;
	memset(Spieler.OldLevels, 0, sizeof Spieler.OldLevels);
}


/* Leveldatei *************************************************/

static void Skip(CharPtr *ref_p, char c)
{
#define p (*ref_p)
	while (*p != c && *p >= ' ') p++;
	if (*p != '\0') p++;
#undef p
}

static void GetLine(CharPtr *ref_p, char *s)
{
#define p (*ref_p)
	unsigned i;
	i = 0;
	while (*p >= ' ') {
		s[i] = *p; i++; p++;
	}
	s[i] = '\0';
#undef p
}

static unsigned GetCard(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned v;
	v = 0;
	while (*p == ' ' || *p == '#') p++;
	while (*p >= '0' && *p <= '9') {
		v = v * 10 + *p - '0';
		p++;
	}
	return v;
#undef p
}

static int NextLine(CharPtr *ref_p)
{
#define p (*ref_p)
	while (*p >= ' ') p++;
	while (*p != '\0' && *p != 10) p++;
	if (*p != 0) p++;
	return *p != '\0';
#undef p
}

static void SetString(CharPtr *ref_p, char *s)
{
#define p (*ref_p)
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (s[i] == '\0') return;
		*p++ = s[i];
	}
#undef p
}

static void SetCard(CharPtr *ref_p, unsigned c, unsigned l)
{
#define p (*ref_p)
	String20Typ s;
	CardToString(c, l, s);
	SetString(&p, s);
	if (l != 1) SetString(&p, "#");
#undef p
}

static void SetLF(CharPtr *ref_p)
{
#define p (*ref_p)
	*p++ = 13;
	*p++ = 10;
#undef p
}

void WriteLevel(char *Name)
{
	CharPtr p;
	unsigned i;
	int h;
		
	void WriteMonster(MonsterTyp *ref_m)
	{
#define m (*ref_m)
		SetString(&p, "M#");
		SetCard(&p, m.x, 4); SetCard(&p, m.y, 4); SetCard(&p, m.Typ, 4); SetCard(&p, m.Status, 4);
		SetCard(&p, m.Trefferwurf, 4); SetCard(&p, m.Schaden, 4); SetCard(&p, m.Bonus, 4);
		SetCard(&p, m.TP, 4); SetCard(&p, m.Sprich, 4); SetCard(&p, m.Spezial, 4);
		SetString(&p, m.Name);
		SetLF(&p);
#undef m
	}

	void WriteGegenstand(GegenstandTyp *ref_g)
	{
#define g (*ref_g)
		SetString(&p, "G#");
		SetCard(&p, g.x, 4); SetCard(&p, g.y, 4); SetCard(&p, g.Sprite, 4);
		SetCard(&p, g.Flags, 4); SetCard(&p, g.Spezial, 4);
		SetCard(&p, g.KennNummer, 4); SetCard(&p, g.Ring, 4); SetCard(&p, g.RingWirkung, 4);
		SetCard(&p, g.RingDauer, 4); SetString(&p, g.Name);
		SetLF(&p);
#undef g
	}

	void WriteParameter(ParameterTyp *ref_a)
	{
#define a (*ref_a)
		SetString(&p, "P#");
		SetCard(&p, a.x, 4); SetCard(&p, a.y, 4); SetCard(&p, a.Art, 4);
		SetCard(&p, a.xhoch, 4); SetCard(&p, a.yhoch, 4); SetCard(&p, a.Levelhoch, 4);
		SetCard(&p, a.xrunter, 4); SetCard(&p, a.yrunter, 4); SetCard(&p, a.Levelrunter, 4);
		SetLF(&p);
#undef a
	}

	Buffer = GetBuffer(BufferSize);
	p = Buffer;
	SetString(&p, "*    x    y  Typ Stat Tref Scha Bonu   TP  Dia Spez Name");
	SetLF(&p);
	for (i = 1; i <= AnzahlMonster; i++) WriteMonster(&Monster[i]);
	SetString(&p, "*    x    y Must Flag Spez  Typ   P1   P2   P3 Name");
	SetLF(&p);
	for (i = 1; i <= AnzahlGegen; i++) WriteGegenstand(&Gegenstand[i]);
	SetString(&p, "*    x    y  Typ   P1   P2   P3   P4   P5   P6");
	SetLF(&p);
	for (i = 1; i <= AnzahlParameter; i++) WriteParameter(&Parameter[i]);
/*
	for (i = 0; i <= MaxSprites-1; i++) {
		SetString(&p, "#FELD.NAME("); SetCard(&p, i, 1); SetString(&p, ') = "');
		SetString(&p, Felder[i].Name); SetString(&p, '"'); SetLF(&p);
		SetString(&p, "#FELD.SPEZIAL("); SetCard(&p, i, 1); SetString(&p, ') = ');
		SetCard(&p, Felder[i].Spezial % 128, 1); SetLF(&p);
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
		m.x = GetCard(&p); m.y = GetCard(&p); m.Typ = GetCard(&p); m.Status = GetCard(&p);
		m.Trefferwurf = GetCard(&p); m.Schaden = GetCard(&p); m.Bonus = GetCard(&p);
		m.TP = GetCard(&p); m.Sprich = GetCard(&p); m.Spezial = GetCard(&p);
		Skip(&p, '#'); GetLine(&p, m.Name);
		NewMonster(m.x, m.y, &m);
	}
	
	void ReadGegen(void)
	{
		GegenstandTyp g;
		p++;
		g.x = GetCard(&p); g.y = GetCard(&p); g.Sprite = GetCard(&p);
		g.Flags = GetCard(&p); g.Spezial = GetCard(&p);
		g.KennNummer = GetCard(&p); g.Ring = GetCard(&p); g.RingWirkung = GetCard(&p);
		g.RingDauer = GetCard(&p); Skip(&p, '#'); GetLine(&p, g.Name);
		NewGegenstand(g.x, g.y, &g);
	}
	
	void ReadParameter(void)
	{
		ParameterTyp o;
		p++;
		o.x = GetCard(&p); o.y = GetCard(&p); o.Art = GetCard(&p);
		o.xhoch = GetCard(&p); o.yhoch = GetCard(&p); o.Levelhoch = GetCard(&p);
		o.xrunter = GetCard(&p); o.yrunter = GetCard(&p); o.Levelrunter = GetCard(&p);
		NewParameter(o.x, o.y, &o);
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
	} while (NextLine(&p));
}


static void __attribute__ ((constructor)) at_init(void)
{
	/* VersionsNummer = 7; erste Farbversion */
	/* VersionsNummer = 8; HASCS II schwarz-weiß */
	/* VersionsNummer = 9; HASCS III */
	VersionsNummer = 10; /* mit Karte */
	
	*LastSprites = *"";
	NewSprites = FALSE;

	Buffer = GetBuffer(BufferSize);
}

#if 0
int main(void)
{
	int h = OpenFile("RTC.PLA");
	char xblock[1292];
	unsigned long code = MakeCode("RTC");

	ReadFile(h, sizeof xblock, xblock);
	CloseFile(h);
	printf("decoding...\n");
	CodeBuffer(code, sizeof xblock, xblock);
	h = CreateFile("RTC.DEC");
	WriteFile(h, sizeof xblock, xblock);
	CloseFile(h);
	return 0;
}
#endif
