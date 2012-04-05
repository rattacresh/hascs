#include "HASCSSpieler.h" /* HASCSSpieler module */

#include "HASCSSystem.h"
#include "HASCSGraphics.h"
#include "HASCSGlobal.h"
#include "HASCSDisk.h"
#include "HASCSOutput.h"
#include "HASCSMagic.h"
#include "HASCSMonster.h"
#include "Dialog.h"
#include "Screen.h"
#include "Sound.h"

unsigned Rueckgabe;


int SpielerFrei(unsigned x, unsigned y);

void Minus(unsigned *ref_x, unsigned y)
{
#define x (*ref_x)
	if (y > x)
		x = 0;
	else {
		x = x - y;
	}
#undef x
}

int DiskScreen()
{
	unsigned sound, load, save, quit, print, /*cancel,*/ x;
	NewScreen(8, 4, 24, 16, " HASCS III ");
	print = AddObject(2, 2, 20, 1, "#133#", BigText|Outlined|Selectable);
	if (DruckerAusgabe) SetFlagSelected(print, 1);
	sound = AddObject(2, 4, 20, 1, "#135#", BigText|Outlined|Selectable);
	if (SoundAusgabe) SetFlagSelected(sound, 1);
	load = AddObject(2, 7, 20, 1, "#130#", BigText|Outlined|Exit);
	save = 0;
	if (LevelNoSave & ~LevelFlags)
		save = AddObject(2, 9, 20, 1, "#131#", BigText|Outlined|Exit);
	quit = AddObject(2, 11, 20, 1, "#132#", BigText|Outlined|Exit);
	/*cancel =*/ AddObject(2, 13, 20, 1, "#134#", BigText|Outlined|Exit);
	DrawScreen();
	x = HandleScreen();
	DruckerAusgabe = GetFlagSelected(print) != 0;
	SoundAusgabe = GetFlagSelected(sound) != 0;
	if (x == load) {
		LoadOrSavePlayer(TRUE);
		LoadLevel(Spieler.LevelNumber);
		PrintLevelName(LevelName);
		Rueckgabe = 3;
	} else if (x == save) {
		SaveLevel(Spieler.LevelNumber);
		LoadOrSavePlayer(FALSE);
	}
	RestoreScreen();
	return x == quit;
}


int ZaubernGelungen(void)
{
	if (SchutzWurf(Spieler.Zt)) return TRUE;
	if (SZaubern & Spieler.Status && SchutzWurf(Spieler.Zt))
		return TRUE;
	if (SMagier & Spieler.Typ && SchutzWurf(Spieler.Zt))
		return TRUE;
	else {
		OutputText("#610#");
		return FALSE;
	}
}
 
int RingAnlegen(GegenstandTyp *ref_r)
{
#define r (*ref_r)
	if (Spieler.Ring.KennNummer != 0) {
		OutputText("#711#"); /* Zwei Ringe? */
		return FALSE;
	} else if (ZaubernGelungen()) { 
		Spieler.Ring = r;
		Erfahrung(15 + Zufall(15));
		if (r.Ring == 1) {
			Spieler.Status = Spieler.Status | r.RingWirkung;
			Spieler.Permanent = Spieler.Permanent | r.RingWirkung;
		} else if (r.Ring >= 10 && r.Ring <= 15) /* Basiswerterhöhung */
			ChangeBasiswert(r.Ring - 10, r.RingWirkung);
		else if (r.Ring >= 20 && r.Ring <= 25) /* Basiswertsenkung */
			ChangeBasiswert(r.Ring - 20, -r.RingWirkung);
		OutputText("#669#");
		return TRUE;
	} else
		return FALSE;
#undef r
}

void RingAblegen(GegenstandTyp *ref_r)
{
#define r (*ref_r)
	if (r.Ring == 1) { /* Statusänderung */
		Spieler.Status = Spieler.Status & ~r.RingWirkung;
		Spieler.Permanent = Spieler.Permanent & ~ r.RingWirkung;
	} else if (r.Ring >= 10 && r.Ring <= 15) /* Basiswerterhöhung */
		ChangeBasiswert(r.Ring - 10, -r.RingWirkung);
	else if (r.Ring >= 20 && r.Ring <= 25) /* Basiswertsenkung */
		ChangeBasiswert(r.Ring - 20, -r.RingWirkung);
	r.KennNummer = 0;
#undef r
}

int RuestungAnlegen(GegenstandTyp *ref_r)
{
#define r (*ref_r)
	if (Spieler.Ruestung.KennNummer == 0) {
		if (GMagisch & Spieler.Ruestung.Flags) {
			if (!ZaubernGelungen())
				return FALSE;
		}
		Spieler.Ruestung = r;
		return TRUE;
	} else {
		OutputText("#710#"); /* Zwei Rüstungen? */
		return FALSE;
	}
}
#undef r

void RuestungAblegen(GegenstandTyp *ref_r)
{
#define r (*ref_r)
	r.KennNummer = 0;
#undef r
}

int GetButton(unsigned *ref_x, unsigned *ref_y)
{
#define x (*ref_x)
#define y (*ref_y)
	BITSET m;
	char ch;
	WaitInput(&x, &y, &m, &ch, -1); x = x / 16; y = y / 16;
	if (ch != '\0') {
		x = MaxSichtweite; y = MaxSichtweite;
		if (ch == '1') {x--; y++;}
		else if (ch == '2') y++;
		else if (ch == '3') {x++; y++;}
		else if (ch == '4') x--;
		else if (ch == '6') x++;
		else if (ch == '7') {x--; y--;}
		else if (ch == '8') y--;
		else if (ch == '9') {x++; y--;}
		else if (ch != '5')
			return FALSE;
		SichtLevelUmrechnung(x, y, &x, &y);
		return TRUE;
	} else {
		if ((x - 1) <= MaxSichtmal2 && (y - 1) <= MaxSichtmal2) {
			SichtLevelUmrechnung((x - 1), (y - 1), &x, &y);
			return TRUE;
		} else
			return FALSE;
	}
#undef x
#undef y
}

int Near(unsigned x, unsigned y)
{
	unsigned xs, ys;
	if (LevelSichtUmrechnung(x, y, &xs, &ys))
		return xs >= MaxSichtweite-1 && xs <= MaxSichtweite+1 
		    && ys >= MaxSichtweite-1 && ys <= MaxSichtweite+1;
	else return FALSE;
}
 
int NearSicht(unsigned xs, unsigned ys)
{
	return xs >= MaxSichtweite-1 && xs <= MaxSichtweite+1
	    && ys >= MaxSichtweite-1 && ys <= MaxSichtweite+1;
}

void *DisplayRucksack(unsigned *ref_n, BITSET *ref_mb)
{
#define n (*ref_n)
#define mb (*ref_mb)
	unsigned i, mx, my;
	char ch;
	ReserveScreen(0, 4, 27, 19);
	TextMode = 1;
		PrintAt(2, 6, "#500#");
	TextMode = 0;
	for (i = 1; i <= MaxRuck; i++) {
		if (Spieler.Rucksack[i].KennNummer != 0) {
			SetSprite(2 + 12 * ((i-1) / 10), 7 + i - 10 * ((i-1) / 10),
				  &SystemSprite[Spieler.Rucksack[i].Sprite]);
			GotoXY(7 + 24 * ((i-1) / 10), 7 + i - 10 * ((i-1) / 10));
			PrintGegenstand(Spieler.Rucksack[i]);
		}
	}
	WaitInput(&mx, &my, &mb, &ch, -1); mx = mx / 16; my = my / 16;
	RestoreScreen();
	n = 0;
	if ((mx >= 2 && mx < 26 && my >= 8 && my < 18)) {
		n = (my - 7) + 10 * ((mx - 2) / 12);
		return &Spieler.Rucksack[n];
	} else if (mx >= 27 && my == 8)
		return &Spieler.rechteHand;
	else if (mx >= 27 && my == 9)
		return &Spieler.linkeHand;
	else if (mx >= 27 && my == 10)
		return &Spieler.Ruestung;
	else if (mx >= 27 && my == 11)
		return &Spieler.Ring;
	return NULL;
#undef n
#undef mb
}

void Angriff(unsigned zx, unsigned zy, unsigned WM, GegenstandTyp *ref_Waffe)
{
#define Waffe (*ref_Waffe)
	unsigned Schaden, i,/*t ,*/ Trefferwurf, Punkte;
	Trefferwurf = Zufall(20);
	if (Trefferwurf + Spieler.Ge >= 20 + WM) {
		if (Waffe.KennNummer == GWaffe)
			Schaden = Zufall(Waffe.WaffenSchaden) + Waffe.WaffenBonus +
				   Zufall(Spieler.St) / 4;
		else
			Schaden = Zufall(Spieler.St) / 4;
		if (SKrieger & Spieler.Typ)
			Schaden += Zufall(Spieler.Grad);
		else if (SAbenteurer & Spieler.Typ
		      || SPriester & Spieler.Typ
		      || SAmazone & Spieler.Typ)
			Schaden += Zufall(Spieler.Grad) / 2;
		if (SKraft & Spieler.Status) /* Krafttrunk */
			Schaden += Zufall(Schaden);
		if (Trefferwurf == 20) /* natürliche 20 */
			Schaden = Schaden * 2;
		i = FindMonster(zx, zy);
		if (MonsterParade(&Monster[i], &Waffe, Spieler.Ge + Trefferwurf)) {
			BeginOutput(); /* Du triffst! ??? pariert! */
			Print("#511#"); Print(Monster[i].Name); Print("#515#");
			EndOutput();
		} else {
			if (MonsterMagisch & Monster[i].Spezial
			  && GMagisch & ~Waffe.Flags)
				Schaden = 0; /* keine magische Waffe */
			Punkte = (Monster[i].Trefferwurf + Monster[i].Schaden * 2 +
				Monster[i].Bonus * 3) / 10;
			BeginOutput();
			Print("#511#"); PrintCard(Schaden, 1); Print("#512#");
			if (HitMonster(&Monster[i], Schaden))
				Print("#510#");
			EndOutput();
			Erfahrung(Punkte * Schaden);
		}
	} else { /* nicht getroffen */
		if (Trefferwurf == 1 && Zufall(10) == 1
		  && Waffe.KennNummer != 0)
		{
			OutputText("#513#");
			Waffe.KennNummer = 0;
			PrintCharakter(5);
		} else
			OutputText("#514#");
	}
	if (Waffe.WaffenAnwendungen > 1) {
		Waffe.WaffenAnwendungen--; PrintCharakter(5);
	} else if (Waffe.WaffenAnwendungen == 1) {
		Waffe.KennNummer = 0;
		PrintCharakter(5);
	}
#undef Waffe
}

void NahAngriff(unsigned zx, unsigned zy)
{
	int rWaffe, lWaffe;
	unsigned Bonus;

	int NahkampfWaffe(GegenstandTyp r)
	{
		return r.KennNummer == GWaffe && (1<<0) & r.Spezial;
	}

	if (SSchutz & Spieler.Status) /* Schutz */
		return;
	Bonus = 0;
	if (SAbenteurer & Spieler.Typ) Bonus = 1;
	rWaffe = NahkampfWaffe(Spieler.rechteHand);
	lWaffe = NahkampfWaffe(Spieler.linkeHand);
	if (rWaffe && lWaffe) { /* Beidhändiger Kampf */
		Angriff(zx, zy, 4 - Bonus, &Spieler.rechteHand);
		if (LevelMonster & Level[zx][zy].Spezial)
			Angriff(zx, zy, 4 - Bonus, &Spieler.linkeHand);
	} else if (rWaffe)
		Angriff(zx, zy, 1 - Bonus, &Spieler.rechteHand);
	else if (lWaffe)
		Angriff(zx, zy, 1 - Bonus, &Spieler.linkeHand);
	else if (Spieler.rechteHand.KennNummer == 0) /* Handangriff */
		Angriff(zx, zy, 1 - Bonus, &Spieler.rechteHand);
	else if (Spieler.linkeHand.KennNummer == 0)
		Angriff(zx, zy, 1 - Bonus, &Spieler.linkeHand);
}

void LeiterBenutz(int Hoch)
{
	int Leiter; ParameterTyp r; unsigned i/*, j*/;
	Leiter = FALSE;
	if (LevelParameter & Level[Spieler.x][Spieler.y].Spezial
	 && SReitet & ~Spieler.Status) {
		i = FindParameter(Spieler.x, Spieler.y);
		if (Parameter[i].Art == FLeiterBeide) {
			if (Hoch) {
				r.ZielX = Parameter[i].xhoch;
				r.ZielY = Parameter[i].yhoch;
				r.ZielLevel = Parameter[i].Levelhoch;
			} else {
				r.ZielX = Parameter[i].xrunter;
				r.ZielY = Parameter[i].yrunter;
				r.ZielLevel = Parameter[i].Levelrunter;
			}
			Leiter = TRUE;
		} else if (Hoch 
			? Parameter[i].Art == FLeiterHoch
			: Parameter[i].Art == FLeiterRunter)
		{
			r = Parameter[i];
			Leiter = TRUE;
		}
	}
	if (Leiter) {
		if (Hoch)
			OutputText("#520#");
		else
			OutputText("#521#");
		if (r.ZielLevel == 0) /* gelöst ! */
			Spieler.LevelNumber = 0;
		else {
			Spieler.x = r.ZielX; Spieler.y = r.ZielY;
			if (r.ZielLevel != Spieler.LevelNumber) { /* neues Level laden */
				SaveLevel(Spieler.LevelNumber);
				Spieler.LevelNumber = r.ZielLevel;
				LoadOrSavePlayer(FALSE);
				LoadLevel(Spieler.LevelNumber);
				PrintLevelName(LevelName);
				Rueckgabe = 3;
			}
		}
	} else
		OutputText("#523#");
}

void TuerOeffnen(unsigned px, unsigned py)
{
	unsigned i = FindParameter(px, py);
	if (Parameter[i].Art == FTuerZu) {
		Level[px][py].Feld = Parameter[i].SpriteOffen;
		Parameter[i].Art = FTuerOffen;
		OutputText("#550#"); /* geoeffnet */
		Rueckgabe = 3;
	} else if (Parameter[i].Art == FTuerVerschlossen)
		OutputText("#561#"); /* geht nicht */
}

void Besteigen(void)
{
	unsigned tx, ty/*, l*/, i/*, j*/; int ok;
	BeginOutput(); Print("#530#"); /* Wohin auf-/absteigen? */
	if (GetButton(&tx, &ty)) {
		if (SReitet & Spieler.Status) { /* absteigen */
			if (Near(tx, ty)) {
				Spieler.Status &= ~SReitet;
				if (SpielerFrei(tx, ty)
				  && LevelMonster & ~Level[tx][ty].Spezial)
				{
					Spieler.ReitTier.x = Spieler.x;
					Spieler.ReitTier.y = Spieler.y;
					Level[Spieler.x][Spieler.y].Spezial |= LevelMonster;
					Spieler.x = tx; Spieler.y = ty;
					if (AnzahlMonster < MaxMonster)
						AnzahlMonster++;
					Monster[AnzahlMonster] = Spieler.ReitTier;
					SetNewSprite(MaxSichtweite, MaxSichtweite);
					Print("#531#"); /* ok */
					EndOutput();
					return;
				} else
					Spieler.Status |= SReitet;
			}
		} else { /* aufsteigen */
			if (Near(tx,ty) && LevelMonster & Level[tx][ty].Spezial) {
				i = FindMonster(tx, ty);
				if (MonsterReitbar & Monster[i].Spezial) {
					ok = TRUE;
					if (Monster[i].Status == 1 || Monster[i].Status == 7) {
						/* Monster will aber nicht bestiegen werden! */
						ok = (Zufall(20) + Zufall(Spieler.Ge)) > Monster[i].Trefferwurf;
						/* drauf sind wir, aber ob wir oben bleiben? */
						ok = ok && (Zufall(20) + Zufall(Spieler.St)) > Monster[i].Trefferwurf;
					}
					if (ok) {
						Spieler.x = tx; Spieler.y = ty;
						Spieler.Status |= SReitet;
						Spieler.ReitTier = Monster[i];
						Level[tx][ty].Spezial &= ~LevelMonster;
						while (i < AnzahlMonster) {
							Monster[i] = Monster[i+1];
							i++;
						}
						AnzahlMonster--;
						SetNewSprite(MaxSichtweite, MaxSichtweite);
						Print("#531#"); /* ok */
						EndOutput();
						return;
					} else {
						Print("#533#"); /* nicht geschafft */
						EndOutput();
						return;
					}
				}
			}
		}
	}
	Print("#532#"); /* geht nicht */;
	EndOutput();
}

void Wegwerfen(unsigned n)
{
	while (n < MaxRuck) {
		Spieler.Rucksack[n] = Spieler.Rucksack[n+1];
		n++;
	}
	Spieler.Rucksack[MaxRuck].KennNummer = 0;
}

unsigned RucksackFrei(void)
{
	unsigned i;
	for (i = 1; i <= MaxRuck; i++)
		if (Spieler.Rucksack[i].KennNummer == 0)
			return i;
	return 0;
}

void Ausruhen(void)
{
	unsigned hunger;
	if (Spieler.TP >= Spieler.TPMax)
		return;
	hunger = (Spieler.TPMax - Spieler.TP) / 5;
	if (Spieler.Nahrung >= hunger) {
		OutputText("#590#");
		Spieler.Nahrung -= hunger;
		Spieler.Status |= SAusruhen;
		PrintCharakter(3);
	} else
		OutputText("#591#");
}

void Benutze(GegenstandTyp *ref_r)
{
	void Fernkampf(GegenstandTyp *ref_r)
	{
#define r (*ref_r)
		unsigned x, y, x2, y2, mx, my; int hit;
		if ((1 << 1) & ~r.Spezial) {
			OutputText("#667#"); return;
		}
		OutputText("#600#");
		if (!GetButton(&x2, &y2)) return;
		if (LevelSichtUmrechnung(x2, y2, &x2, &y2)) {
			x = MaxSichtweite; y = MaxSichtweite;
			hit = MakeShoot(&x, &y, x2, y2, 100, TRUE);
			SichtLevelUmrechnung(x, y, &mx, &my);
			if (hit) {
				if (SAmazone & Spieler.Typ) /* Amazone */
					Angriff(mx, my, 0, &r);
				else
					Angriff(mx, my, 2, &r); /* Fernkampf mit WM -2 */
			} else {
				if (r.WaffenAnwendungen > 1)
					r.WaffenAnwendungen--;
				else if (r.WaffenAnwendungen == 1)
					r.KennNummer = 0;
			}
			if ((1 << 3) & r.Spezial && r.KennNummer != 0) {
				if (SchutzWurf(Spieler.Ge) || SchutzWurf(Spieler.Ge)) {
					if (!LegeGegenstand(mx, my, r))
						OutputText("#601#");
				} else
					OutputText("#601#");
				r.KennNummer = 0;
			}
		}
#undef r
	}

	void NimmZauberstab(GegenstandTyp *ref_r)
	{
#define r (*ref_r)
		unsigned x, y, mx, my, i; /*int b;*/

		void Verzaubert(MonsterTyp *ref_m, unsigned *ref_Stab, unsigned *ref_Wirkung)
		{
#define m (*ref_m)
#define Stab (*ref_Stab)
#define Wirkung (*ref_Wirkung)
			unsigned i, j; /*int b;*/ MonsterTyp nm;
			switch (Stab) {
			case 1 : j = W6(Wirkung);
				BeginOutput(); /* erst Ausgabe, da Monster tot sein könnte! */
				Print(Name); Print("#620#"); PrintCard(j, 1); Print("#626#");
				EndOutput();
				/*b =*/ HitMonster(&m, j);
				return;
			case 2 : m.Status = Wirkung; break;
			case 3 : m.Spezial |= Wirkung; break;
			case 4 : m.Spezial &=  ~Wirkung; break;
			case 5 : Minus(&m.Trefferwurf, Wirkung); break;
			case 6 : Minus(&m.Schaden, Wirkung); break;
			case 7 : Minus(&m.Bonus, Wirkung); break;
			case 8 : BeginOutput(); Print(Name); Print("#628#"); EndOutput();
				do {
					NormalKoords(
						Zufall(2 * Wirkung + 1) + m.x - Wirkung - 1,
						Zufall(2 * Wirkung + 1) + m.y - Wirkung - 1, &i, &j);
				} while (!MonsterFrei(&m, i, j));
				nm = m; DeleteMonster(x, y); NewMonster(i, j, &nm);
				return;
			case 10 : DoMonsterDialog(Wirkung, &m);
				return;
			}
			BeginOutput(); Print(Name); Print("#628#"); EndOutput();
#undef m
#undef Stab
#undef Wirkung
		}

		if (ZaubernGelungen()) {
			OutputText("#624#"); /* Wohin... */
			if (GetButton(&mx, &my)) {
				/*b =*/ LevelSichtUmrechnung(mx, my, &mx, &my);
				if (mx <= MaxSichtmal2 && my <= MaxSichtmal2) {
					x = MaxSichtweite; y = MaxSichtweite;
					if (MakeShoot(&x, &y, mx, my, 30, r.Spezial == 0)) {
						SichtLevelUmrechnung(x, y, &mx, &my);
						i = FindMonster(mx, my);
						if (MonsterSchutzwurf(&Monster[i]))
							OutputText("#625#"); /* keine Wirkung */
						else
							Verzaubert(&Monster[i], &r.Zauberstab, &r.ZStabWirkung);
					}
				}
			}
			Erfahrung(25 + Zufall(25));
			if (Zufall(100) <= r.ZStabAbw) {
				OutputText("#666#"); /* brennt aus */
				r.KennNummer = 0;
			}
		}
#undef r
	}

	void NimmPhiole(GegenstandTyp *ref_r)
	{
#define r (*ref_r)
		unsigned i, j; GegenstandTyp *g; BITSET b;
		switch (r.Phiole) {
		case 1: TrefferPunkte(W6(r.PhioleWirkung), TRUE); /* Heilung */
			OutputText("#650#"); break;
		case 2: TrefferPunkte(W6(r.PhioleWirkung), FALSE); /* Gift */
			OutputText("#656#"); break;
		case 3: Spieler.Status = Spieler.Status | r.PhioleWirkung;
			OutputText("#651#"); break;
		case 4: Spieler.Status = Spieler.Status & ~r.PhioleWirkung;
			OutputText("#651#"); break;
		case 5: Todeshauch(1, r.PhioleWirkung); /* Todeshauch */
			OutputText("#654#"); break;
		case 6 ... 7: OutputText("#630#");
			g = DisplayRucksack(&i, &b);
			if (g)
				if (g->KennNummer ) {
					if (r.Pergament == 6)
						g->Flags |= r.PhioleWirkung;
					else
						g->Flags &= ~r.PhioleWirkung;
					BeginOutput(); Print("#631#"); PrintGegenstand(*g); EndOutput();
				}
			break;
		case 8: OutputText("#632#");
			Erweckung(r.PhioleWirkung);
			break;
		case 9: OutputText("#633#");
			do {
				NormalKoords((int)Zufall(2 * r.PhioleWirkung + 1) + Spieler.x
					           - r.PhioleWirkung - 1,
					           (int)Zufall(2 * r.PhioleWirkung + 1) + Spieler.y
					           - r.PhioleWirkung - 1,
					           &i, &j);
			} while (!SpielerFrei(i, j));
			Spieler.x = i; Spieler.y = j;
			break;
		case 10: Spieler.Status |= SLicht;
			Spieler.Sichtweite = r.PhioleWirkung;
			OutputText("#651#");
			break;
		case 11: if (Vision(r.PhioleWirkung, 2, &i, &j));
			RestoreScreen();
			break;
		}
		if (r.PhioleAnwendungen > 0) {
			r.PhioleAnwendungen--;
			if (r.PhioleAnwendungen == 0)
				r.KennNummer = 0;
		}
#undef r
	}

	void NimmPergament(GegenstandTyp *ref_r)
	{
#define r (*ref_r)
		if (ZaubernGelungen()) { 
			Erfahrung(50 + Zufall(50));
			NimmPhiole(&r);
		} else {
			if (r.PergamentAnwendungen > 0) {
				r.PergamentAnwendungen--;
				if (r.PergamentAnwendungen == 0)
					r.KennNummer = 0;
			}
		}
#undef r
	}

	void BenutzeSchluessel(GegenstandTyp *ref_r)
	{
#define r (*ref_r)
		unsigned x, y, i;

		int KeyFits(void)
		{
			unsigned f;
			if ((1 << 0) & r.Spezial) { /* Muster-Schluessel */
				f = Level[x][y].Feld;
				return f == r.SchluesselX || f == r.SchluesselY
				    || f == r.SchluesselLevel;
			} else
				return x == r.SchluesselX && y == r.SchluesselY
				    && Spieler.LevelNumber == r.SchluesselLevel;
		}

		BeginOutput(); Print("#540#"); /* Welche Tür... */
		if (!GetButton(&x, &y)) { EndOutput(); return; }
		if (Near(x, y) && LevelParameter & Level[x][y].Spezial) {
			i = FindParameter(x, y);
			if (Parameter[i].Art == FTuerZu) { /* Abschließen */
				if (KeyFits()) {
					Parameter[i].Art = FTuerVerschlossen;
					Level[x][y].Feld = Parameter[i].SpriteVerschlossen;
					Print("#541#"); /* verschlossen */
				} else
					Print("#542#"); /* Schlüssel paßt nicht */
			} else if (Parameter[i].Art == FTuerVerschlossen) { /* Aufschließen */
				if (KeyFits()) {
					Parameter[i].Art = FTuerZu;
					Level[x][y].Feld = Parameter[i].SpriteZu;
					Print("#560#"); /* aufgeschlossen */
				} else
					Print("#542#"); /* Schlüssel paßt nicht */
			}
		}
		EndOutput();
#undef r
	}

	void NimmNahrung(GegenstandTyp *ref_r)
	{
#define r (*ref_r)
		Spieler.Nahrung += r.Nahrung;
		if (Spieler.Nahrung > MaxNahrung)
			Spieler.Nahrung = MaxNahrung;
		PrintCharakter(3);
		r.KennNummer = 0;
#undef r
	}
	
#define r (*ref_r)
	switch (r.KennNummer) {
	case GZauberstab: NimmZauberstab(&r); break;
	case GPergament:  NimmPergament(&r); break;
	case GPhiole:     NimmPhiole(&r); break;
	case GWaffe:      Fernkampf(&r); break;
	case GSchluessel: BenutzeSchluessel(&r); break;
	case GNahrung:    NimmNahrung(&r); break;
	default:
		if (r.KennNummer > 10) /* Sondergegenstand */
			if (r.DialogNr != 0) {
				DoGegenstandDialog(r.DialogNr, &r);
				if (r.DialogAnzahl > 0) {
					r.DialogAnzahl--;
					if (r.DialogAnzahl == 0)
						r.KennNummer = 0;
				}
			}
	}
#undef r
}


void LegeAb(GegenstandTyp *ref_r)
{
#define r (*ref_r)
	unsigned x, y; int abgelegt;

	abgelegt = FALSE;
	if (r.KennNummer == 0) /* leer */
		return;
	else if (GVerflucht & r.Flags) { /* verfluchter Gegenstand */
		OutputText("#671#"); /* geht nicht! */
		return;
	}
	BeginOutput(); Print("#720#"); /* Wohin ablegen... */
	if (GetButton(&x, &y)) { /* ins Level */
		if (Near(x, y)) {
			if (LegeGegenstand(x, y, r)) {
				abgelegt = TRUE;
				Print("#721#"); /* OK */
			} else
				Print("#722#"); /* unmöglich */
		} else
			Print("#722#");
	} else { /* in den Rucksack */
		if (RucksackFrei() == 0) { /* Rucksack voll */
			Print("#670#");
		} else {
			Spieler.Rucksack[RucksackFrei()] = r;
			Print("#721#"); /* OK */
			abgelegt = TRUE;
		}
	}
	EndOutput();
	if (abgelegt) {
		if (r.KennNummer == GRing)
			RingAblegen(&r);
		else if (r.KennNummer == GRuestung)
			RuestungAblegen(&r);
		else if (r.KennNummer == GLicht) {
			r.KennNummer = 0;
			Spieler.Sichtweite = SetLightRange();
		} else
			r.KennNummer = 0;
		PrintCharakter(5);
	}
#undef r
}

/**********************************************************************/

void InfoGegenstand(unsigned gx, unsigned gy)
{
	unsigned i;

	BeginOutput();
	i = FindGegenstand(gx,gy);
	Print("#695#");
	PrintGegenstand(Gegenstand[i]);
	EndOutput();
}

void  InfoFeld(unsigned x, unsigned y)
{
	unsigned i;

	int SucheFalle(ParameterTyp *ref_p)
	{
#define p (*ref_p)
		unsigned EP;
		EP = 0;
		if (SchutzWurf(Spieler.In)
		  || (SAbenteurer & Spieler.Typ && SchutzWurf(Spieler.In)))
		{
			if (Zufall(100) > p.Chance1) {
				BeginOutput(); Print("#571#"); /* Falle! */
				if (SchutzWurf(Spieler.Ge)
				 || (SAbenteurer & Spieler.Typ && SchutzWurf(Spieler.Ge)))
				{
					if (Zufall(100) > p.Chance2) {
						Print("#572#"); /* Entschärft! */
						p.Art = 0;
						EP = 10 + Zufall(10 + p.Schaden * 6);
					}
				}
				EndOutput();
				Erfahrung(EP);
				return TRUE;
			}
		}
		return FALSE;
#undef p
	}

	if (Near(x, y) && LevelParameter & Level[x][y].Spezial) {
		i = FindParameter(x, y);
		if (Parameter[i].Art == FFalle) {
			if (SucheFalle(&Parameter[i]))
				return;
		} else if ((Parameter[i].Art == FDialog || Parameter[i].Art == FBild || Parameter[i].Art == FSound)) {
			if ((Parameter[i].automatisch == 0)) {
				if (Parameter[i].Art == FDialog) DoParameterDialog(Parameter[i].Nummer, &Parameter[i]);
				else if (Parameter[i].Art == FSound) PlaySoundN(Parameter[i].Nummer);
				else if (Parameter[i].Art == FBild) ShowPicture(Parameter[i].Nummer, TRUE);
				if (Parameter[i].Zaehler > 0) {
					Parameter[i].Zaehler--;
					if (Parameter[i].Zaehler == 0)
						Parameter[i].Art = 0;
				}
				return;
			}
		} else if (Parameter[i].Art == FTuerOffen) {
			Level[x][y].Feld = Parameter[i].SpriteZu;
			Parameter[i].Art = FTuerZu;
			OutputText("#543#"); /* Du schließt die Tür! */
			Rueckgabe = 3;
			return;
		}
	}
	BeginOutput();
	Print("#700#"); Print(Felder[Level[x][y].Feld].Name);
	EndOutput();
}

void InfoMonster(unsigned mx, unsigned my)
{
	unsigned i;
	i = FindMonster(mx, my);
	if (Monster[i].Status == 0) {
		if (LevelGegenstand & Level[mx][my].Spezial)
			InfoGegenstand(mx, my);
		else
			InfoFeld(mx, my);
	} else {
		BeginOutput();
		Print("#680#"); Print(Name); /* Du siehst: ... */
		EndOutput();
		if (Monster[i].Sprich > 0 && Monster[i].Sprich < 1000 && Monster[i].Status > 1)
			DoMonsterDialog(Monster[i].Sprich, &Monster[i]);
	}
}

/**********************************************************************/

void Nimm(unsigned number)
{
	int rfrei, lfrei; GegenstandTyp r;

	int Beidhaendig(GegenstandTyp r)
	{
		return r.KennNummer == GWaffe && (1 << 2) & r.Spezial;
	}
		
	rfrei = Spieler.rechteHand.KennNummer == 0;
	lfrei = Spieler.linkeHand.KennNummer == 0;
	if (Beidhaendig(Spieler.rechteHand)) lfrei = FALSE;
	r = Spieler.Rucksack[number];
	switch (r.KennNummer) {
	case GRuestung:
		if (RuestungAnlegen(&r))
			Wegwerfen(number);
		break;
	case GRing:
		if (RingAnlegen(&r))
			Wegwerfen(number);
		break;
	default:
		if (!rfrei && !lfrei)
			OutputText("#712#");
		else if (Beidhaendig(r) && !(rfrei && lfrei))
			OutputText("#713#");
		else {
			if (rfrei) Spieler.rechteHand = r;
			else Spieler.linkeHand = r;
			Wegwerfen(number);
			if (r.KennNummer == GLicht)
				Spieler.Sichtweite = SetLightRange();
		}
	}
}

void DoRucksack(void)
{
	unsigned  x,y,i; BITSET b; /*void *g;*/
	/*g =*/ DisplayRucksack(&i, &b);
	if (i >= 1 && i <= MaxRuck && (1 << 0) & b) { /* in die Hand nehmen */
		if ((Spieler.Rucksack[i].KennNummer != 0)) {
			Nimm(i);
			PrintCharakter(5);
		}
	} else if (i >= 1 && i <= MaxRuck && (1 << 1) & b) /* ablegen */
		if (Spieler.Rucksack[i].KennNummer) {
			BeginOutput(); Print("#720#"); /* Wohin ablegen... */
			if (GetButton(&x, &y)) /* dahin ablegen */
				if (Near(x, y))
					if (LegeGegenstand(x, y, Spieler.Rucksack[i])) {
						Wegwerfen(i);
						Print("#721#"); /* OK */
						EndOutput();
						return;
					}
			Print("#722#");
			EndOutput();
		}
}

void DoGegenstand(unsigned x, unsigned y)
{
	GegenstandTyp g;
	int aufgenommen;

	aufgenommen = NimmGegenstand(x, y, TRUE, &g);
	BeginOutput();
		Print("#322#");      /* Du findest: */
		PrintGegenstand(g);
	EndOutput();
	if (aufgenommen) {
		PrintCharakter(3);
		PrintCharakter(4);
	} else if (g.KennNummer == GNahrung)
		PrintCharakter(3);
	else
		OutputText("#335#"); /* Rucksack voll! */
}

void Karte()
{
	unsigned d;
	if (LevelNoMap & ~LevelFlags) {
		if (Vision(44, 1, &d, &d)) { }
		RestoreScreen();
	}
}

/**********************************************************************/

int SpielerFrei(unsigned x, unsigned y)
{
	BITSET Spezial; /*unsigned i;*/
	Spezial = Felder[Level[x][y].Feld].Spezial;
	if (SReitet & Spieler.Status) /* reitet */
		return MonsterFrei(&Spieler.ReitTier, x, y);
	else if (LevelMonster & Level[x][y].Spezial)
		return FALSE;
	else if (FeldBegehbar & Spezial && FeldSumpf & Spezial)
		return Zufall(10) <= 5;
	else if (FeldBegehbar & Spezial)
		return TRUE;
	else if (FeldWasser & Spezial)
		return SSchwimmt & Spieler.Status;
	return FALSE;
}

int IsMonster(unsigned x, unsigned y)
{
	unsigned i;
	if (LevelMonster & Level[x][y].Spezial) {
		i = FindMonster(x, y);
		return MonsterImmun & ~Monster[i].Spezial;
	}
	return FALSE;
}

int IsGegenstand(unsigned x, unsigned y)
{
	return LevelGegenstand & Level[x][y].Spezial
		&& FeldBegehbar & Felder[Level[x][y].Feld].Spezial;
}

int IsTuer(unsigned x, unsigned y)
{
	unsigned i;
	if (LevelParameter & Level[x][y].Spezial) {
		i = FindParameter(x, y);
		return Parameter[i].Art == FTuerZu || Parameter[i].Art == FTuerVerschlossen;
	}
	return FALSE;
}


unsigned SpielerBewegung(unsigned x, unsigned y, BITSET t)
{
	unsigned /*i,*/ ix, iy, fx, fy;
	/*BITSET b;*/
	/*char ch;*/
	/*int NichtFrei;*/

	Rueckgabe = 0;

	if (SBetrunken & Spieler.Status && Zufall(10) <= 5) { /* betrunken */
		x = Zufall(23); y = Zufall(23); t = (1<<0);
	}

	if (x > 0 && y > 0
	 && x < 24 && y < 24 && (1 << 0) & t) /* Bewegung */
	{

		ix = MaxSichtweite; iy = MaxSichtweite;
		if (x > SichtMitteX) ix++; else if (x < SichtMitteX) ix--;
		if (y > SichtMitteY) iy++; else if (y < SichtMitteY) iy--;
		SichtLevelUmrechnung(ix, iy, &fx, &fy);

		if (NearSicht(x - 1, y - 1) && IsMonster(fx, fy))
			NahAngriff(fx, fy); /* Nahkampf */
		else if (NearSicht(x - 1, y - 1) && IsGegenstand(fx, fy))
			DoGegenstand(fx, fy);
		else if (SpielerFrei(fx,fy)) { /* Bewegung */
			Rueckgabe = 1;
			Spieler.x = fx; Spieler.y = fy;
		} else if (IsTuer(fx, fy))
			TuerOeffnen(fx, fy);
		else {
			if (FeldSumpf & Felder[Level[fx][fy].Feld].Spezial)
				OutputText("#731#");
			else
				OutputText("#730#");
		}

	} else if (x > 0 && y > 0
		&& x < 24 && y < 24 && (1 << 1) & t) /* Info */
	{
		ix = x - 1; iy = y  - 1;
		SichtLevelUmrechnung(ix, iy, &fx, &fy);

		if (LevelBekannt & SichtBereich[ix][iy].Spezial) {
			if (LevelMonster & Level[fx][fy].Spezial)
				InfoMonster(fx, fy);
			else if (LevelGegenstand & Level[fx][ fy].Spezial)
				InfoGegenstand(fx, fy);
			else
				InfoFeld(fx, fy);
		}

	} else if (x >= 25 && x <= 38
	     && y >= 13 && y <= 14 && (1 << 0) & t) /* Menü */
	{
		switch (x) {
		case 25 ... 26: LeiterBenutz(TRUE); break;   /* hoch */
		case 27 ... 28: LeiterBenutz(FALSE); break; /* runter */
		case 29 ... 30: Besteigen(); break;
		case 31 ... 32: DoRucksack(); break;
		case 33 ... 34: Karte(); break;
		case 35 ... 36: Ausruhen(); break;
		case 37 ... 38: if (DiskScreen()) Rueckgabe = 2; /* Quit */
		}
	} else if (x >= 25 && y == 8) {
		if ((1 << 0) & t) {
			Benutze(&Spieler.rechteHand); PrintCharakter(5);
		} else if ((1 << 1) & t)
			LegeAb(&Spieler.rechteHand);
	} else if (x >= 25 && y == 9) {
		if ((1 << 0) & t) {
			Benutze(&Spieler.linkeHand); PrintCharakter(5);
		} else if ((1 << 1) & t)
			LegeAb(&Spieler.linkeHand);
	} else if (x >= 25 && y == 10) {
		if ((1 << 1) & t)
			LegeAb(&Spieler.Ruestung);
	} else if (x >= 25 && y == 11) {
		if ((1 << 0) & t) {
			Benutze(&Spieler.Ring); PrintCharakter(5);
		} else if ((1 << 1) & t)
			LegeAb(&Spieler.Ring);
	}

	return Rueckgabe;

}
