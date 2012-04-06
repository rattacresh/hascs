/* HASCSGame module */
#include "compat.h"
#include "HASCSGame.h"

#include "HASCSSystem.h"
#include "HASCSGraphics.h"
#include "HASCSOutput.h"
#include "HASCSGlobal.h"
#include "HASCSMonster.h"
#include "HASCSSpieler.h"
#include "HASCSDisk.h"
#include "HASCSMagic.h"
#include "Dialog.h"
#include "Sound.h"

unsigned Rueckgabe, oldx, oldy, Weite;
int dx[93], dy[93];
unsigned AnzahlSteigungen;


void MakeSichtBereich(int Force)
{
	unsigned i;

	void CopySichtBereich(void)
	{
		unsigned x, y, xl, yl;
		for (x = 0; x <= MaxSichtmal2; x++)
			for (y = 0; y <= MaxSichtmal2; y++) {
				SichtLevelUmrechnung(x, y, &xl, &yl);
				SichtBereich[x][y] = Level[xl][yl];
			}
	}

	void PrintSichtBereich(void)
	{
		unsigned x, y/*, xl, yl*/;
		for (x = 0; x <= MaxSichtmal2; x++)
			for (y = 0; y <= MaxSichtmal2; y++)
				SetNewSprite(x, y);
	}

	void MakeLine(int x0, int y0, int dx, int dy)
	{
		int ix, iy, ax, ay, ct, of;
		ax = 0; ay = 0; ix = 1; iy = 1;
		if (dx < 0) { dx = -dx; ix = -1; }
		if (dy < 0) { dy = -dy; iy = -1; }
		if (dx < dy) {
			ct = dx; dx = dy; dy = ct; ay = ix; ax = iy; ix = 0; iy = 0;
		}
		of = dx / 2; ct = 1;

		while (dx >= ct) {
			x0 += ix; y0 += ax; of += dy;
			if (of >= dx) {
				of -= dx; x0 += ay; y0 += iy;
			}
			if (LevelSpieler & ~SichtBereich[x0][y0].Spezial)
				SichtBereich[x0][y0].Spezial |= LevelSpieler;
				if (ct <= Weite
				 || (LevelSichtbar & SichtBereich[x0][y0].Spezial 
				  && SLicht & ~Spieler.Status))
					SichtBereich[x0][y0].Spezial |= LevelBekannt;
			if (FeldDurchsichtig & ~Felder[SichtBereich[x0][y0].Feld].Spezial)
				return;
			ct++;
		}
	}


	if (!Force) {
		if (Spieler.x == oldx && Spieler.y == oldy 
		 && Spieler.Sichtweite == Weite) { /* keine Änderungen */
			return;
		}
	}

	oldx = Spieler.x; oldy = Spieler.y;
	Weite = Spieler.Sichtweite;

	CopySichtBereich();
	for (i = 1; i <= AnzahlSteigungen; i++) {
		MakeLine(MaxSichtweite, MaxSichtweite, dx[i], dy[i]);
		MakeLine(MaxSichtweite, MaxSichtweite, -dx[i], -dy[i]);
		MakeLine(MaxSichtweite, MaxSichtweite, -dy[i], dx[i]);
		MakeLine(MaxSichtweite, MaxSichtweite, dy[i], -dx[i]);
	}
	PrintSichtBereich();

	if (Spieler.Sichtweite > 0
	 || LevelSichtbar & SichtBereich[MaxSichtweite][MaxSichtweite].Spezial)
		SichtBereich[MaxSichtweite][MaxSichtweite].Spezial |= LevelBekannt;
}


/* Spezialfelder **********************************************************/

void Falle(ParameterTyp *ref_p)
{
#define p (*ref_p)
	unsigned i, s;
	s = 0;
	for (i = 1; i <= p.Schaden; i++)
		s += Zufall(6); /* Schaden auswürfeln */
	if (SAbenteurer & Spieler.Typ) { s = s / 2; }
	TrefferPunkte(s, FALSE);
	BeginOutput();
	if ((1 << 0) & p.Flag) {
		Print(Felder[Level[Spieler.x][Spieler.y].Feld].Name);
		Print("#304#");
	} else
		Print("#300#");
	if (s == 0) Print("#301#");
	else { PrintCard(s, 1); Print("#303#");
	}
	EndOutput();
	if (p.Anzahl > 0) { /* Anzahl Wirkungen verringern */
		p.Anzahl--;
		if (p.Anzahl == 0) /* Falle löschen */
			p.Art = 0;
	}
#undef p
}

void Teleport(ParameterTyp *ref_p)
{
#define p (*ref_p)
	/*unsigned i, j; MonsterTyp Reittier;*/
	WaitTime(0); /* ausgeben */
	if (p.ZielLevel == 0) /* gelöst ! */
		Spieler.LevelNumber = 0;
	else {
		if (SReitet & Spieler.Status) {
			Spieler.ReitTier.x = p.ZielX;
			Spieler.ReitTier.y = p.ZielY;
		}
		Spieler.x = p.ZielX; Spieler.y = p.ZielY;
		if (p.ZielLevel != Spieler.LevelNumber) { /* neues Level laden */
			SaveLevel(Spieler.LevelNumber);
			Spieler.LevelNumber = p.ZielLevel;
			LoadOrSavePlayer(FALSE);
			LoadLevel(Spieler.LevelNumber);
		}
	}
#undef p
}

void Feldaenderung(ParameterTyp *ref_p)
{
#define p (*ref_p)
	unsigned /*xs, ys,*/ i, a;
	Level[p.FeldX][p.FeldY].Feld = p.FeldNummer;
	if (LevelParameter & Level[p.FeldX][p.FeldY].Spezial) {
		i = FindParameter(p.FeldX, p.FeldY);
		a = Parameter[i].Art;
		Parameter[i].Art = p.ParNummer;
		if (a == FLicht) {
			SetOneLight(Parameter[i].x, Parameter[i].y, Parameter[i].Weite, FALSE);
			SetLightLevel(FALSE);
		} else if (p.ParNummer == FLicht)
			SetOneLight(Parameter[i].x, Parameter[i].y, Parameter[i].Weite, TRUE);
	}
	MakeSichtBereich(TRUE); /* neuer Sichtbereich */
#undef p
}

void Info(ParameterTyp *ref_p)
{
#define p (*ref_p)
	if (p.automatisch > 0) { /* Automatischer Text */
		if (p.Zaehler > 0) {
			p.Zaehler--;
			if (p.Zaehler == 0) {
				p.Art = 0;
			}
		}
		if (p.automatisch == 2)
			p.automatisch = 0;
		switch (p.Art) {
		case FDialog : DoParameterDialog(p.Nummer, &p); break;
		case FSound : PlaySoundN(p.Nummer); break;
		case FBild  : ShowPicture(p.Nummer, TRUE); break;
		}
	}
#undef p
}

/**************************************************************************/

void MakeTeleport(void)
{
	ParameterTyp p;
	BeginOutput();
	p.ZielX = 0; Print("x="); InputCard(&p.ZielX, 4);
	Print("  ");
	p.ZielY = 0; Print("y="); InputCard(&p.ZielY, 4);
	Print("  ");
	p.ZielLevel = 0; Print("l="); InputCard(&p.ZielLevel, 4);
	EndOutput();
	Teleport(&p);
}

/**************************************************************************/

unsigned DoGame(void)
{
	unsigned mx, my, c, SpielerAktion; BITSET mt, FeldSpezial;
	int beschleunigt, /*aufgenommen,*/ b;
	char ch;
	/*ParameterTyp p;*/

	void KeyBoard(char Taste)
	{
		unsigned i, x, y;
		switch (CAP(Taste)) {
		case '1' : mt = MausLinks; mx = SichtMitteX - 1; my = SichtMitteY + 1; break;
		case '2' : mt = MausLinks; mx = SichtMitteX    ; my = SichtMitteY + 1; break;
		case '3' : mt = MausLinks; mx = SichtMitteX + 1; my = SichtMitteY + 1; break;
		case '4' : mt = MausLinks; mx = SichtMitteX - 1; my = SichtMitteY; break;
		case '5' : mt = MausLinks; mx = SichtMitteX    ; my = SichtMitteY; break;
		case '6' : mt = MausLinks; mx = SichtMitteX + 1; my = SichtMitteY; break;
		case '7' : mt = MausLinks; mx = SichtMitteX - 1; my = SichtMitteY - 1; break;
		case '8' : mt = MausLinks; mx = SichtMitteX    ; my = SichtMitteY - 1; break;
		case '9' : mt = MausLinks; mx = SichtMitteX + 1; my = SichtMitteY - 1; break;
		case '(' : mt = MausLinks; mx = 25; my = 13; break; /* Leiter hoch */
		case ')' : mt = MausLinks; mx = 27; my = 13; break; /* Leiter runter */
		case '/' : mt = MausLinks; mx = 29; my = 13; break; /* Besteigen */
		case 'K' : mt = MausLinks; mx = 33; my = 13; break; /* Karte */
		case 'R' : mt = MausLinks; mx = 31; my = 13; break; /* Rucksack */
		case '0' : mt = MausLinks; mx = 25; my = 8; break; /* rechte Hand */
		case '.' : mt = MausLinks; mx = 25; my = 9; break; /* linke Hand */

		case 'P' : DruckerAusgabe = !DruckerAusgabe; break;
		case 'T' : if (DebugMode) {
				MakeTeleport();
			}
			break;
		case 'M' : if (DebugMode) {
				BeginOutput();
				Print("M="); PrintCard(AnzahlMonster, 1);
				Print(" G="); PrintCard(AnzahlGegen, 1);
				Print(" P="); PrintCard(AnzahlParameter, 1);
				Print(" Z="); PrintLongCard(Spieler.Moves, 1);
				Print(" x="); PrintCard(Spieler.x, 1);
				Print(" y="); PrintCard(Spieler.y, 1);
				Print(" l="); PrintCard(Spieler.LevelNumber, 1);
				Print(" s="); PrintCard(Spieler.Status, 1);
				Print(" p="); PrintCard(Spieler.Permanent, 1);
				Print(" t="); PrintCard(Spieler.Typ, 1);
				EndOutput();
			}
			break;
		case 'F' : if (DebugMode) {
				BeginOutput();
				for (i = 1; i <= MaxFlags; i++) {
					Print("S"); PrintCard(i,1); Print("=");
					PrintCard(Spieler.Flags[i], 1); Print(", ");
				}
				EndOutput();
			}
			break;
		case 'V' : if (DebugMode) {
				if (Vision(44, 0, &x, &y)) {
					Spieler.x = x; Spieler.y = y;
					MakeSichtBereich(FALSE);
				}
				RestoreScreen();
			}
			break;
		}
	}

	void MinusLicht(GegenstandTyp *ref_Hand)
	{
#define Hand (*ref_Hand)
		if (Zufall(10) == 1) /* 10% Chance */
			if (Hand.LichtDauer > 0) {
				Hand.LichtDauer--;
				if (Hand.LichtDauer == 0) {
					BeginOutput(); Print(Hand.Name); Print("#320#"); EndOutput();
					Hand.KennNummer = 0;
					Spieler.Sichtweite = SetLightRange();
				}
				PrintCharakter(5);
			}
#undef Hand
	}


	Rueckgabe = 0;
	beschleunigt = FALSE;
	do {

		if (LevelNotZyklisch & LevelFlags) /* nicht zyklisch ? */
			if (Spieler.x < 11 || Spieler.y < 11
			 || (LevelBreite - Spieler.x) < 11 || (LevelHoehe - Spieler.y) < 11)
				if (LevelDialog != 0)
					DoDialog(LevelDialog);

		if (NewSprites) {
			NewSprites = FALSE;
			RestoreScreen();
		}
		MakeSichtBereich(FALSE);

		FeldSpezial = Felder[Level[Spieler.x][Spieler.y].Feld].Spezial;
		if (FeldLava & FeldSpezial
		  && ((SReitet|SFeuer) & Spieler.Status) == 0)
		{
			c = 10 + Zufall(10); TrefferPunkte(c, FALSE);
			BeginOutput();
			Print(Felder[Level[Spieler.x][Spieler.y].Feld].Name);
			Print("! "); PrintCard(c, 1); Print("#321#");
			EndOutput();
		}
		if (FeldWasser & FeldSpezial 
		 && ((SReitet|SSchwimmt) & Spieler.Status) == 0)
		{
			c = 4 + Zufall(4); TrefferPunkte(c, FALSE);
			BeginOutput();
			Print(Felder[Level[Spieler.x][Spieler.y].Feld].Name);
			Print("! "); PrintCard(c, 1); Print("#321#");
			EndOutput();
		}
		if (FeldHunger & FeldSpezial && Spieler.Nahrung > 0) {
			Spieler.Nahrung--;
			PrintCharakter(3);
		}

		if (LevelAutoDialog & LevelFlags) /* Auto-Dialog */
			if (LevelDialog != 0)
				DoDialog(LevelDialog);

#if 0
		if (LevelSpezial[1] != 0) { /* Tag und Nacht */
			c = 0;
			OutputText("");
			PrintLongCard(Spieler.Moves % (2 * (long)LevelSpezial[1]), 1);
			if (Spieler.Moves % (2 * (long)LevelSpezial[1]) <
				 LevelSpezial[1])
				c = LevelSpezial[2];
			PrintCard(c, 3);
			if (c != LevelSpezial[2]) {
				LevelSpezial[2] = c;
				Spieler.Sichtweite = SetLightRange();
			}
		}
#endif
		if (LevelParameter & Level[Spieler.x][Spieler.y].Spezial) {
			c = FindParameter(Spieler.x, Spieler.y);
			switch (Parameter[c].Art) {
			case FFalle         : Falle(&Parameter[c]); break;
			case FTeleport      : Teleport(&Parameter[c]);
				MakeSichtBereich(TRUE);
				PrintLevelName(LevelName);
				break;
			case FFeldAenderung : Feldaenderung(&Parameter[c]); break;
			case FDialog:
			case FBild:
			case FSound         : Info(&Parameter[c]); break;
			}
		}

		Spieler.Moves++; /* Zugzähler */

		if (Spieler.Nahrung > 0) {
			if (SAusruhen & ~Spieler.Status && SVersteinert & ~Spieler.Status) {
				if (Zufall(10) == 1) {
					Spieler.Nahrung--; PrintCharakter(3);
				}
				if (Zufall(200) <= Spieler.Ko && Spieler.TP < Spieler.TPMax) {/* Regeneration */
					Spieler.TP++; PrintCharakter(3);
				}
			}
		} else if (Spieler.Nahrung == 0 && Zufall(10) == 1) { /* Hunger ! */
			OutputText("#336#"); TrefferPunkte(1, FALSE);
		}

		if (SAusruhen & Spieler.Status) { /* ruhe sanft... */
			if (Spieler.TP >= Spieler.TPMax) { /* aufgewacht */
				Spieler.Status &= ~SAusruhen; OutputText("#337#");
			} else if (Zufall(40) <= Spieler.Ko) {
				Spieler.TP++;
				PrintCharakter(3);
			}
		}
		if (Spieler.Status & ~Spieler.Permanent) /* Sonderstatus */
			if (Zufall(100) <= 2) {
				if (SLicht & Spieler.Status) {
					Spieler.Status &= ~SLicht;
					Spieler.Sichtweite = SetLightRange();
					MakeSichtBereich(TRUE);
				}
				Spieler.Status = Spieler.Status & Spieler.Permanent; /* Sonderstati löschen */
			}

		if (Spieler.rechteHand.KennNummer == GLicht) /* Lichtspender ? */
			MinusLicht(&Spieler.rechteHand);
		if (Spieler.linkeHand.KennNummer == GLicht)
			MinusLicht(&Spieler.linkeHand);

		if (Spieler.Ring.KennNummer == GRing) /* Ring wird getragen */
			if (Zufall(10) == 1) /* 10% Chance */
				if (Spieler.Ring.RingDauer > 0) {
					Spieler.Ring.RingDauer--;
					if (Spieler.Ring.RingDauer == 0) {
						OutputText("#338#");
						RingAblegen(&Spieler.Ring);
					}
					PrintCharakter(5);
				}

		/* Monster sind auch mal dran ... */

		if (SReitet & Spieler.Status && SpielerAktion == 1) /* Bewegung */
			b = MonsterSchnell & Spieler.ReitTier.Spezial;
		else
			b = SFlink & Spieler.Status;
		if (b) {
			beschleunigt = !beschleunigt;
			if (beschleunigt) /* ... oder auch nicht */
				KeineMonsterBewegung();
			else
				MonsterBewegung();
		} else
			MonsterBewegung();

		if (Spieler.LevelNumber == 0)
			Rueckgabe = 3; /* gelöst ! */
		else if (STot & Spieler.Status)
			Rueckgabe = 1; /* Schon wieder tot ? */
		else if (SAusruhen & ~Spieler.Status 
			&& SVersteinert & ~Spieler.Status)
		{
			WaitInput(&mx, &my, &mt, &ch, -1);
			if (mt == 0)
				KeyBoard(ch); /* Tasten-Befehl ? */
			else
				mx = mx / 16; my = my / 16;
			if (Rueckgabe == 0) {
				SpielerAktion = SpielerBewegung(mx, my, mt);
				if (SpielerAktion == 2)
					Rueckgabe = 2; /* Quit */
				else if (SpielerAktion == 3)
					MakeSichtBereich(TRUE); /* Tür geöffnet oder geschlossen */
			}
		} else
			WaitTime(0); /* Neuzeichnen */
	} while (Rueckgabe == 0);

	return Rueckgabe;

}

void FindeSteigungen(void)
{
	int x, y;

	void NeueSteigung(int nx, int ny)
	{
		unsigned i;
		if (x == 0 && ny == 0) return;
		for (i = 1; i <= AnzahlSteigungen; i++) {
			if ((dx[i] * ny == dy[i] * nx)) {
				if (ABS(nx) > ABS(dx[i])
				 || ABS(ny) > ABS(dy[i]))
				{
					dx[i] = nx; dy[i] = ny;
				}
				return; /* Steigung schon vorhanden */ 
			}
		}
		AnzahlSteigungen++;
		dx[AnzahlSteigungen] = nx;
		dy[AnzahlSteigungen] = ny;
	}

	AnzahlSteigungen = 0;
	for (x = 0; x <= MaxSichtweite; x++)
		for (y = 0; y <= MaxSichtweite-1; y++)
			NeueSteigung(x - MaxSichtweite, y - MaxSichtweite);
}

static void __attribute__ ((constructor)) at_init(void)
{
	oldx = 999; oldy = 999; Weite = 999;
	FindeSteigungen();
}
