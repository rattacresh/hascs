/* HASCSGlobal module */
#include "compat.h"
#include "HASCSGlobal.h"

#include "HASCSOutput.h"
#include "HASCSGraphics.h"
#include "HASCSSystem.h"
#include "Screen.h"

static int ScreenReserved;

/* Bildschirm-Ausgaben ****************************************************/

void SetNewSprite(unsigned x, unsigned y)
{
	/* Setzt im Sichtbereich x, y neues Sprite und updated OldScreen */
	unsigned xl, yl, Nummer;

	if (x == MaxSichtweite && y == MaxSichtweite) {
		if (SReitet & Spieler.Status)
			Nummer = 256 + Spieler.ReitTier.Typ;
		else
			Nummer = 512 + Spieler.Sprite;
	} else {
		if (LevelBekannt & SichtBereich[x][y].Spezial) {
			SichtLevelUmrechnung(x, y, &xl, &yl);
			Level[xl][yl].Spezial |= LevelKarte;
			Nummer = SichtBereich[x][y].Feld;
			if (LevelGegenstand & SichtBereich[x][y].Spezial)
				Nummer = 512 + Gegenstand[FindGegenstand(xl, yl)].Sprite;
			if (LevelMonster & SichtBereich[x][y].Spezial) {
				Nummer = FindMonster(xl, yl);
				if (MonsterUnsichtbar & ~Monster[Nummer].Spezial)
					Nummer = 256 + Monster[Nummer].Typ;
			}
		} else
			Nummer = 513; /* SystemSprite[1] */
	}
	if (Nummer != OldScreen[x][y]) {
		if (Nummer < 256) /* Feld */
			SetSprite(x+1, y+1, &FelderSprite[Nummer]);
		else if (Nummer < 512) /* Monster */
			SetSprite(x+1 ,y+1 , &MonsterSprite[Nummer - 256]);
		else if (Nummer < 1024) /* Gegenstand */
			SetSprite(x+1, y+1, &SystemSprite[Nummer - 512]);
		OldScreen[x][y] = Nummer;
	}
}

void SetOldSprite(unsigned x, unsigned y)
/* Setzt im Sichtbereich x, y altes Sprite von OldScreen */
{
	unsigned Nummer;
	Nummer = OldScreen[x][y];
	if (Nummer < 256) /* Feld */
		SetSprite(x+1, y+1, &FelderSprite[Nummer]);
	else if (Nummer < 512) /* Monster */
		SetSprite(x+1 ,y+1, &MonsterSprite[Nummer - 256]);
	else if (Nummer < 1024) /* Gegenstand */
		SetSprite(x+1, y+1, &SystemSprite[Nummer - 512]);
	else
		SetSprite(x+1, y+1, &SystemSprite[0]);
}


void FillRectangle(int x0, int y0, int x1, int y1, SpriteType *ref_Sprite)
{
#define Sprite (*ref_Sprite)
	int x, y;
	for (x = x0; x <= x1; x++)
		for (y = y0; y <= y1; y++)
			SetSprite(x, y, &Sprite);
#undef Sprite
}

int MakeShoot(unsigned *ref_qx, unsigned *ref_qy, unsigned zx, unsigned zy, unsigned time,
		int mode)
{
#define qx (*ref_qx)
#define qy (*ref_qy)
	int x0, y0, x1, y1, dx, dy, ix, iy, ax, ay, of, ct;
	/*unsigned mx, my, i; BITSET mb; char mch;*/
	int ausserhalb, hit;

/* Führt Schuß im Sichtbereich durch von qx, qy in Richtung zx, zy */

	int SichtFrei(unsigned x, unsigned y)
	{
		int sf; unsigned i, xl, yl;
		sf = TRUE;
		if (mode || (x == zx && y == zy)) {
			if (LevelMonster & SichtBereich[x][y].Spezial) {
				SichtLevelUmrechnung(x, y, &xl, &yl);
				i = FindMonster(xl, yl);
				sf = MonsterImmun & Monster[i].Spezial;
				hit = !sf;
			}
		}
		sf = sf && FeldDurchsichtig & Felder[SichtBereich[x][y].Feld].Spezial;
		sf = sf && (x != MaxSichtweite || y != MaxSichtweite);
		return sf;
	}

	x0 = qx; y0 = qy;
	x1 = zx; y1 = zy;
	ax = 0; ay = 0;
	if (x1 >= x0) { dx = x1 - x0; ix = 1; }
	else { dx = x0 - x1; ix = -1; }
	if (y1 >= y0) { dy = y1 - y0; iy = 1; }
	else { dy = y0 - y1; iy = -1; }
	if (dx < dy) {
		ct = dx; dx = dy; dy = ct; ay = ix; ax = iy; ix = 0; iy = 0;
	}
	/*i = 0;*/
	x0 = qx; y0 = qy;
	of = dx / 2;
	hit = FALSE;
	do {
		x0 += ix; y0 += ax; of += dy;
		if (of >= dx) {
			of -= dx; x0 += ay; y0 += iy;
		}
		InvertFeld(x0+1, y0+1);
		WaitTime(time); /* Zeichnen */
		InvertFeld(x0+1, y0+1);
		ausserhalb = x0 == 0 || y0 == 0 
			|| x0 == MaxSichtmal2 || y0 == MaxSichtmal2;
	} while (SichtFrei(x0, y0) && !ausserhalb);
	qx = x0;
	qy = y0;
	return hit;
#undef qx
#undef qy
}


void RestoreScreen(void)
/* Stellt den normalen Spiel-Bildschirm wieder her */
{
	unsigned x, y;
	FillRectangle(0,  0, 39,  0, &SystemSprite[9]);
	FillRectangle(0, 24, 39, 24, &SystemSprite[9]);
	FillRectangle(0,  1,  0, 24, &SystemSprite[9]);
	FillRectangle(39, 1, 39, 24, &SystemSprite[9]);
	FillRectangle(24, 1, 24, 24, &SystemSprite[9]);
	FillRectangle(25,12, 38, 12, &SystemSprite[9]);
	FillRectangle(25,15, 38, 15, &SystemSprite[9]);
	PrintLevelName(LevelName);
	for (x = 0; x <= MaxSichtmal2; x++)
		for (y = 0; y <= MaxSichtmal2; y++)
			SetOldSprite(x, y);
	PrintCharakter(0); /* gesamten Spieler */
	PrintMenue(); /* Menü ausgeben */
	PrintOutput(); /* Textfenster */
	ScreenReserved = FALSE;
}


void ReserveScreen(unsigned x1, unsigned y1, unsigned x2, unsigned y2)
/* malt Rahmen und löscht Bildschirmausschnitt */
{
	unsigned x, y;
	if (x2 == 0 && y2 == 0)
		return;
	SetSprite(x1, y1, &SystemSprite[25]); /* links oben */
	SetSprite(x1, y2, &SystemSprite[27]); /* links unten */
	SetSprite(x2, y1, &SystemSprite[26]); /* rechts oben */
	SetSprite(x2, y2, &SystemSprite[28]); /* rechts unten */
	for (x = x1+1; x <= x2-1; x++) {
		SetSprite(x, y1, &SystemSprite[29]);
		SetSprite(x, y2, &SystemSprite[30]);
	}
	for (y = y1+1; y <= y2-1; y++) {
		SetSprite(x1, y, &SystemSprite[32]);
		SetSprite(x2, y, &SystemSprite[31]);
	}
	FillRectangle(x1+1, y1+1, x2-1, y2-1, &SystemSprite[0]);
	ScreenReserved = TRUE;
}


void PrintCharakter(unsigned What)
/* Gibt Charakter(oder Teile) aus */
{
	unsigned /*i,*/ n/*, cx, cy*/;
	FillRectangle(25, 1, 38, 11, &SystemSprite[0]);

	n = Length(Spieler.Name);
	FillRectangle(25, 0, 38, 0, &SystemSprite[9]);
	TextMode = 1; PrintAt(32 - n / 2, 0, Spieler.Name); TextMode = 0;

	PrintAt(50, 1, "#10#"); GotoXY(61,1); PrintCard(Spieler.St, 2); /* Basiswerte */
	PrintAt(64, 1, "#11#"); GotoXY(75,1); PrintCard(Spieler.Ge, 2);
	PrintAt(50, 2, "#12#"); GotoXY(61,2); PrintCard(Spieler.Ko, 2);
	PrintAt(64, 2, "#13#"); GotoXY(75,2); PrintCard(Spieler.In, 2);
	PrintAt(50, 3, "#14#"); GotoXY(61,3); PrintCard(Spieler.Zt, 2);
	PrintAt(64, 3, "#15#"); GotoXY(75,3); PrintCard(Spieler.Ch, 2);

	FillRectangle(25, 4, 38, 4, &SystemSprite[45]);
	PrintAt(50, 5,"#400#"); PrintLongCard(Spieler.EP, 6); /* Erfahrungspkt. */
	Print("#401#"); PrintCard(Spieler.Grad, 2);
	PrintAt(50, 6,"#402#"); PrintCard(Spieler.TP, 5);     /* Trefferpunkte */
	Print("#403#"); PrintCard(Spieler.TPMax,3);
	PrintAt(68, 5,"#404#"); PrintCard(Spieler.Nahrung, 3); /* Nahrung */
	PrintAt(68, 6,"#405#"); PrintCard(Spieler.Gold, 5);    /* Gold */

	FillRectangle(25, 7, 38, 7, &SystemSprite[45]);

	SetSprite(25, 8, &SystemSprite[43]);
	if (Spieler.rechteHand.KennNummer != 0) {
		SetSprite(27, 8, &SystemSprite[Spieler.rechteHand.Sprite]);
		GotoXY(57, 8); PrintGegenstand(Spieler.rechteHand);
	}
	SetSprite(25, 9, &SystemSprite[44]);
	if (Spieler.linkeHand.KennNummer != 0) {
		SetSprite(27, 9, &SystemSprite[Spieler.linkeHand.Sprite]);
		GotoXY(57, 9); PrintGegenstand(Spieler.linkeHand);
	}
	SetSprite(25, 10, &SystemSprite[41]);
	if (Spieler.Ruestung.KennNummer != 0) {
		SetSprite(27, 10, &SystemSprite[Spieler.Ruestung.Sprite]);
		GotoXY(57, 10); PrintGegenstand(Spieler.Ruestung);
	}
	SetSprite(25, 11, &SystemSprite[42]);
	if (Spieler.Ring.KennNummer != 0) {
		SetSprite(27, 11, &SystemSprite[Spieler.Ring.Sprite]);
		GotoXY(57, 11); PrintGegenstand(Spieler.Ring);
	}
}

void PrintMenue(void)
/* Menü ausgeben */
{
	unsigned i;
	for (i = 0; i <= 6; i++) {
		SetSprite(2 * i + 25, 13, &GegenSprite[128 + 4 * i]);
		SetSprite(2 * i + 26, 13, &GegenSprite[129 + 4 * i]);
		SetSprite(2 * i + 25, 14, &GegenSprite[130 + 4 * i]);
		SetSprite(2 * i + 26, 14, &GegenSprite[131 + 4 * i]);
	}
}

void PrintGegenstand(GegenstandTyp g)
/* Ausgabe eines Gegenstandes */
{
	unsigned x;
	if (g.KennNummer == 0) return; /* leer */
	if (GErkannt & g.Flags) {
		if (GMagisch & g.Flags) Print("#424#");
		if (GVerflucht & g.Flags) Print("#425#");
	}
	switch (g.KennNummer) {
	case GRing      : if (GErkannt & g.Flags) {
				Print(g.Name); Print(" "); PrintCard(g.RingDauer, 1);
			} else
				Print("#420#");
			break;
	case GZauberstab: if (GErkannt & g.Flags) {
				Print(g.Name); Print(" "); PrintCard(g.ZStabAbw, 1); Print("%");
			} else
				Print("#421#");
			break;
	case GPergament : if (GErkannt & g.Flags) {
				Print(g.Name);
				if (g.PergamentAnwendungen > 0) {
					Print(" ");
					PrintCard(g.PergamentAnwendungen, 1);
				}
			} else
				Print("#422#");
			break;
	case GPhiole    : if (GErkannt & g.Flags) { 
				Print(g.Name); 
				if (g.PhioleAnwendungen > 0) {
					Print(" ");
					PrintCard(g.PhioleAnwendungen, 1);
				}
			} else
				Print("#423#");
			break;
	case GWaffe:
	case GRuestung  : Print(g.Name);
			if (g.KennNummer == GWaffe) x = g.WaffenAnwendungen;
			else x = g.RuestAnwendungen;
			if (x > 0 && GErkannt & g.Flags) {
				Print(" "); PrintCard(x, 1);
			}
			break;
	case GGold      : Print(g.Name);
			if (GErkannt & g.Flags) {
				Print(" ");
				PrintCard(g.Gold, 1);
			}
			break;
	case GSchluessel: Print(g.Name);
			break;
	case GNahrung   : Print(g.Name);
			if (GErkannt & g.Flags) {
				Print(" ");
				PrintCard(g.Nahrung, 1);
			}
			break;
	case GLicht     : Print(g.Name);
			if (GErkannt & g.Flags) {
				Print(" ");
				PrintCard(g.LichtDauer, 1);
			}
			break;
	default: /* Irgendein Gegenstand */
		Print(g.Name);
		if (GErkannt & g.Flags && g.DialogAnzahl > 0) {
			Print(" "); PrintCard(g.DialogAnzahl, 1);
		}
	} /* switch */
}

void PrintLevelName(char *s)
{
	unsigned i;
	i = Length(s);
	FillRectangle(1, 0, 23, 0, &SystemSprite[9]);
	TextMode = 1; PrintAt(12 - i / 2, 0, s); TextMode = 0;  
}

void DisplayCharakter(SpielerTyp s)
{
}

/* Hilfsprozeduren ********************************************************/

static unsigned max(unsigned a, unsigned b)
{
	if (a > b)
		return a;
	else
		return b;
}

/* Spieler **************************************************************/

unsigned W6(unsigned i)
{
	unsigned j, s;
	s = 0;
	for (j = 1; j <= i; j++) s += Zufall(6);
	return s;
}

int SchutzWurf(unsigned x)
{
	return Zufall(20) <= x;
}


unsigned SetLightRange()
{
	unsigned s;
	if (SLicht & Spieler.Status) /* magisches Licht */
		return Spieler.Sichtweite;
	s = LevelSichtweite;
	if (Spieler.rechteHand.KennNummer == GLicht)
		s = max(Spieler.rechteHand.LichtWeite, s);
	if (Spieler.linkeHand.KennNummer == GLicht)
		s = max(Spieler.linkeHand.LichtWeite, s);
	return s;
}


void SetOneLight(int x, int y, int w, int on)
{
	int i, j; unsigned k, l;

	for (i = x-w; i <= x+w; i++)
		for (j = y-w; j <= y+w; j++) {
			NormalKoords(i, j, &k, &l);
			if (on)
				Level[k][l].Spezial |= LevelSichtbar;
			else
				Level[k][l].Spezial &= ~LevelSichtbar;
		}
}


void SetLightLevel(int clear)
{
	unsigned x, y, i;
	if (clear)
		for (x = 0; x <= LevelBreite; x++)
			for (y = 0; y <= LevelHoehe; y++)
				Level[x][y].Spezial &= ~LevelSichtbar;
	for (i = 1; i <= AnzahlParameter; i++)
		if (Parameter[i].Art == FLicht)
			SetOneLight(Parameter[i].x, Parameter[i].y, Parameter[i].Weite, TRUE);
}


unsigned GetBasiswert(unsigned n)
{
	switch (n) {
	case 0 : return Spieler.St;
	case 1 : return Spieler.Ge;
	case 2 : return Spieler.Ko;
	case 3 : return Spieler.In;
	case 4 : return Spieler.Zt;
	case 5 : return Spieler.Ch;
	default: return 0;
	}
}

void ChangeBasiswert(unsigned n, int x)
{
	int z;
	z = (int)GetBasiswert(n) + x;
	if (z < 0) z = 0;
	switch (n) {
	case 0 : Spieler.St = z; break;
	case 1 : Spieler.Ge = z; break;
	case 2 : Spieler.Ko = z; break;
	case 3 : Spieler.In = z; break;
	case 4 : Spieler.Zt = z; break;
	case 5 : Spieler.Ch = z; break;
	}
}

/************************************************************************/

unsigned FindMonster(unsigned mx, unsigned my)
{
	unsigned i;
	for (i = 1; i <= AnzahlMonster; i++)
		if (Monster[i].x == mx && Monster[i].y == my)
			if (Editor || (Monster[i].Status > 0 && Monster[i].Status < 1000))
				return i;
	return 0;
}

void DeleteMonster(unsigned mx, unsigned my)
{
	unsigned i, sx, sy;
	i = FindMonster(mx, my);
	if (i != 0) {
		if (LevelSichtUmrechnung(mx, my , &sx, &sy)) {
			SichtBereich[sx][sy].Spezial &= ~LevelMonster;
			if (!ScreenReserved)
				SetNewSprite(sx, sy);
		}
		Level[mx][my].Spezial &= ~LevelMonster;
		while (i < AnzahlMonster) {
			Monster[i] = Monster[i+1];
			i++;
		}
		AnzahlMonster--;
	}
}

unsigned FindGegenstand(unsigned x, unsigned y)
{
	unsigned i;
	for (i = AnzahlGegen; i >= 1; i--)
		if (Gegenstand[i].x == x && Gegenstand[i].y == y)
			return i;
	return 0;
}

void DeleteGegenstand(unsigned gx, unsigned gy)
{
	unsigned i, sx, sy; int last;
	i = FindGegenstand(gx, gy);
	if (i != 0) {
		while (i < AnzahlGegen) {
			Gegenstand[i] = Gegenstand[i+1];
			i++;
		}
		AnzahlGegen--;
		last = FindGegenstand(gx, gy) == 0;
		if (LevelSichtUmrechnung(gx, gy , &sx, &sy)) {
			if (last)
				SichtBereich[sx][sy].Spezial &= ~LevelGegenstand;
			if (!ScreenReserved)
				SetNewSprite(sx, sy);
		}
		if (last)
			Level[gx][gy].Spezial &= ~LevelGegenstand;
	}
}

unsigned FindParameter(unsigned x, unsigned y)
{
	unsigned i;
	for (i = 1; i <= AnzahlParameter; i++)
		if (Parameter[i].x == x && Parameter[i].y == y)
			return i;
	return 0;
}

void DeleteParameter(unsigned px, unsigned py)
{
	unsigned i, sx, sy;
	i = FindParameter(px, py);
	if (i != 0) {
		if (LevelSichtUmrechnung(px, py , &sx, &sy))
			SichtBereich[sx][sy].Spezial &= ~LevelParameter;
		Level[px][py].Spezial &= ~LevelParameter;
		while (i < AnzahlParameter) {
			Parameter[i] = Parameter[i+1];
			i++;
		}
		AnzahlParameter--;
	}
}

void NewMonster(unsigned mx, unsigned my, MonsterTyp *ref_m)
{
#define m (*ref_m)
	unsigned sx, sy;
	if (AnzahlMonster < MaxMonster && mx <= MaxBreite && my <= MaxHoehe) {
		AnzahlMonster++;
		Monster[AnzahlMonster] = m;
		Monster[AnzahlMonster].x = mx;
		Monster[AnzahlMonster].y = my;
		Level[mx][my].Spezial |= LevelMonster;
		if (LevelSichtUmrechnung(mx, my, &sx, &sy)) {
			SichtBereich[sx][sy].Spezial |= LevelMonster;
			if (!ScreenReserved)
				SetNewSprite(sx, sy);
		}
	}
#undef m
}

void NewGegenstand(unsigned gx, unsigned gy, GegenstandTyp *ref_g)
{
#define g (*ref_g)
	unsigned sx, sy;
	if (AnzahlGegen < MaxGegen && gx <= MaxBreite && gy <= MaxHoehe) {
		AnzahlGegen++;
		Gegenstand[AnzahlGegen] = g;
		Gegenstand[AnzahlGegen].x = gx;
		Gegenstand[AnzahlGegen].y = gy;
		Level[gx][gy].Spezial |= LevelGegenstand;
		if (LevelSichtUmrechnung(gx, gy, &sx, &sy)) {
			SichtBereich[sx][sy].Spezial |= LevelGegenstand;
			if (!ScreenReserved)
				SetNewSprite(sx, sy);
		}
	}
#undef g
}

void NewParameter(unsigned px, unsigned py, ParameterTyp *ref_p)
{
#define p (*ref_p)
	unsigned sx, sy;
	if (AnzahlParameter < MaxPar && px <= MaxBreite && py <= MaxHoehe) {
		AnzahlParameter++;
		Parameter[AnzahlParameter] = p;
		Parameter[AnzahlParameter].x = px;
		Parameter[AnzahlParameter].y = py;
		Level[px][py].Spezial |= LevelParameter;
		if (LevelSichtUmrechnung(px, py, &sx, &sy))
			SichtBereich[sx][sy].Spezial |= LevelParameter;
	}
#undef p
}

/************************************************************************/

void Erfahrung(unsigned Punkte)
{
	unsigned g, p, t;

	int FrageSteigern()
	{
		unsigned /*d,*/ ja; int yes;
		NewScreen(14, 9, 14, 7, "");
		/*d =*/ AddObject(2, 2, 10, 1, "#450#", 0);
		ja = AddObject(2, 4, 4, 1, "#451#", BigText|Outlined|Exit);
		/*d =*/ AddObject(8, 4, 4, 1, "#452#", BigText|Outlined|Exit);
		DrawScreen(); yes = HandleScreen() == ja; RestoreScreen();
		return yes;
	}

	Spieler.EP += Punkte;
	while (Spieler.EP >= Spieler.EPnext) {
		Spieler.EPnext = 2 * Spieler.EPnext;
		if (FrageSteigern()) {
			Spieler.Grad++;
			g = Zufall(6) - 1; /* Basiswert */
			p = Zufall(2); /* Bonus */
			ChangeBasiswert(g, p);
			t = Zufall(Spieler.Ko) / 2 + 4; /* Trefferpunkte */
			Spieler.TPMax += t; Spieler.TP += t;
			BeginOutput();
			Print("#453#"); PrintCard(Spieler.Grad, 1);
			Print("#454#"); PrintCard(t, 1); Print(", ");
			switch (g) {
			case 0 : Print("#10#"); break;
			case 1 : Print("#11#"); break;
			case 2 : Print("#12#"); break;
			case 3 : Print("#13#"); break;
			case 4 : Print("#14#"); break;
			case 5 : Print("#15#"); break;
			}
			Print("#455#"); PrintCard(p, 1);
			EndOutput();
		} else
			Spieler.Lernen++; /* neue Lernmöglichkeit */
		PrintCharakter(1);
		PrintCharakter(3);
	}
	PrintCharakter(2);
}
	
void TrefferPunkte(unsigned Punkte, int Plus)
{
	if (Plus) { /* Dazuzählen */
		Spieler.TP += Punkte;
		if (Spieler.TP > Spieler.TPMax) Spieler.TP = Spieler.TPMax;
		PrintCharakter(3);
	} else {
		Spieler.Status &= ~SAusruhen; /* Aufwachen !!! */
		if (Punkte > Spieler.TP) {
			Spieler.Status |= STot; /* Exitus ... */
			Spieler.TP = 0;
			PrintCharakter(3);
		} else {
			Spieler.TP -= Punkte;
			PrintCharakter(3);
		}
	}
}


int NimmGegenstand(unsigned px, unsigned py, int Einmal, GegenstandTyp *ref_r)
{
#define r (*ref_r)
	unsigned Nummer, i; int loeschen/*, erkannt*/;
	loeschen = TRUE;
	Nummer = FindGegenstand(px, py);
	r = Gegenstand[Nummer];

	if (GChance & r.Flags) {
		r.Flags &= ~GChance; r.Flags |= GErkannt;
		if (((GMagisch|GVerflucht) & r.Flags) != 0)
			if (!SchutzWurf(Spieler.In)) {
				if ((SMagier & Spieler.Typ || SPriester & Spieler.Typ)) {
					if (!SchutzWurf(Spieler.In))
						r.Flags &= ~GErkannt;
				} else
					r.Flags &= ~GErkannt;
			}
	}

	Gegenstand[Nummer] = r;

	if (r.KennNummer == GGold)
		Spieler.Gold += r.Gold;

	else if (r.KennNummer == GNahrung && r.Spezial == 0) {
		if (Spieler.Nahrung + r.Nahrung <= MaxNahrung)
			Spieler.Nahrung += r.Nahrung;
		else {
			if (Einmal)
				Gegenstand[Nummer].Nahrung =
					Gegenstand[Nummer].Nahrung - (MaxNahrung - Spieler.Nahrung);
			Spieler.Nahrung = MaxNahrung; loeschen = FALSE;
		}
	} else {
		i = 1;
		while (i < MaxRuck && Spieler.Rucksack[i].KennNummer != 0)
			i++;
		if (Spieler.Rucksack[i].KennNummer == 0)
			Spieler.Rucksack[i] = r;
		else
			loeschen = FALSE;
	}

	if (loeschen && Einmal)
		DeleteGegenstand(px, py);

	return loeschen;
#undef r
}


int LegeGegenstand(unsigned px, unsigned py, GegenstandTyp r)
{
	/*unsigned i, sx, sy;*/
	if (FeldBegehbar & Felder[Level[px][py].Feld].Spezial 
	 && AnzahlGegen < MaxGegen)
	{
		NewGegenstand(px, py, &r);
		return TRUE;
	} else
		return FALSE;
}


/* Koordinatenumrechnungen ************************************************/

void NormalKoords(int xh, int yh, unsigned *ref_i, unsigned *ref_j)
/* Clippt Koordinaten ins Level */

{
#define i (*ref_i)
#define j (*ref_j)
	i = (xh + (int)LevelBreite + 1) % (LevelBreite + 1);
	j = (yh + (int)LevelHoehe + 1) % (LevelHoehe + 1);
#undef i
#undef j
}

void SichtLevelUmrechnung(unsigned x, unsigned y, unsigned *ref_i, unsigned *ref_j)
{
#define i (*ref_i)
#define j (*ref_j)
	i = (Spieler.x + x + LevelBreite + 1 - MaxSichtweite) % (LevelBreite + 1);
	j = (Spieler.y + y + LevelHoehe + 1 - MaxSichtweite) % (LevelHoehe + 1);
#undef i
#undef j
}

int LevelSichtUmrechnung(unsigned x, unsigned y, unsigned *ref_i, unsigned *ref_j)
{
#define i (*ref_i)
#define j (*ref_j)
	unsigned xh,yh;
	xh = (x + LevelBreite + 1 + MaxSichtweite - Spieler.x) % (LevelBreite + 1);
	yh = (y + LevelHoehe + 1 + MaxSichtweite - Spieler.y) % (LevelHoehe + 1);
	if (xh <= MaxSichtmal2 && yh <= MaxSichtmal2) {
		i = xh; j = yh; return TRUE;
	} else return FALSE;
#undef i
#undef j
}

static void __attribute__ ((constructor)) at_init(void)
{
	ScreenReserved = Editor;
}

