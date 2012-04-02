#include "HASCSMonster.h"


int sign(int i)
{
	if (i < 0) return -1;
	else if (i > 0) return 1;
	else return 0;
}

/* Monster auf einem Feld löschen ****************************************/

void ClearMonster(unsigned x, unsigned y)
{
	unsigned xs, ys, Sprite;
	Level[x,y].Spezial &= ~LevelMonster;
	if (LevelSichtUmrechnung(x, y, xs, ys)) {
		if (LevelBekannt & SichtBereich[xs][ys].Spezial) {
			if (LevelGegenstand & SichtBereich[xs][ys].Spezial) {
				Sprite = Gegenstand[FindGegenstand(x, y)].Sprite;
				SetSprite(xs+1, ys+1, SystemSprite[Sprite]);
				OldScreen[xs][ys] = 512 + Sprite;
			} else {
				Sprite = SichtBereich[xs, ys].Feld;
				SetSprite(xs+1, ys+1, FelderSprite[Sprite]);
				OldScreen[xs][ys] = Sprite;
			}
		}
		SichtBereich[xs][ys].Spezial &= ~LevelMonster;
	}
}

/* Monster neu setzen ****************************************************/

void MonsterParameter(MonsterTyp *m)
{
	ParameterTyp *p; unsigned s;
	p = Parameter[FindParameter(m.x, m.y)];
	switch (p.Art) {
	case FTeleport :
		if (p.ZielLevel == Spieler.LevelNumber) {
			ClearMonster(m.x, m.y);
			m.x = p.ZielX; m.y = p.ZielY;
		}
		break;
	case FFalle :
		s = W6(p.Schaden);
		if (p.Anzahl > 0) {
			p.Anzahl--;
			if (p.Anzahl == 0)
				p.Art = 0;
		}
		if (s > m.TP) {
			m.TP = 0;
			m.Status = 0;
		} else
			m.TP -= s;
		break;
	case FMonsterStatus :
		if ((Typ == p.alterTyp || p.alterTyp == 0)
		 && (Status == p.alterStatus || p.alterStatus == 0))
			Status = p.neuerStatus;
		break;
	}
}

void ShowMonster(unsigned i)
{
	unsigned xs, ys;
	if (Monster[i].Status > 0 && Monster[i].Status < 1000) {
		if ((LevelParameter & Level[Monster[i].x][Monster[i].y].Spezial)) {
			MonsterParameter(Monster[i]);
			if (Monster[i].Status == 0) {
				ClearMonster(Monster[i].x, Monster[i].y);
				WaitTime(0);
				return;
			}
		}
		Level[x][y].Spezial |= LevelMonster;
		if (LevelSichtUmrechnung(Monster[i].x, Monster[i].y, xs, ys)) {
			SichtBereich[xs][ys].Spezial |= LevelMonster;
			if (LevelBekannt & SichtBereich[xs,ys].Spezial) {
				if (MonsterUnsichtbar & ~Monster[i].Spezial
				  && OldScreen[xs][ys] != 256 + Monster[i].Typ)
				{
					SetSprite(xs+1, ys+1, MonsterSprite[Monster[i].Typ]);
					OldScreen[xs][ys] = 256 + Monster[i].Typ;
					WaitTime(0); /* anzeigen */
				}
				if (Monster[i].Sprich > 1000 && Monster[i].Sprich < 2000
				  && SUnsichtbar & ~Spieler.Status) /* ansprechen */
				{
					Sprich -= 1000;
					DoMonsterDialog(Monster[i].Sprich, Monster[i]);
				}
			}
		}
	} else if (Monster[i].Status == 0) {
		ClearMonster(Monster[i].x, Monster[i].y);
		WaitTime(0);
	}
}

/* Monster pariert? *****************************************************/ 

int MonsterParade(MonsterTyp *m, GegenstandTyp *w, unsigned Treffer)
{
	if (MonsterPariert & ~m.Spezial)
		return FALSE;
	if ((GMagisch & w.Spezial && MonsterMagisch & ~m.Spezial)
		return FALSE;
	else
		return Zufall(20) + m.Trefferwurf >= m.Treffer;
}

/* Monster bekommt was ab ***********************************************/ 

int HitMonster(MonsterTyp *m, unsigned Damage)
{
	unsigned xs, ys, j, Reaktion, OldTyp; int Dead;
	Dead = FALSE;
	WITH  DO
		if (LevelSichtUmrechnung(m.x, m.y, xs, ys)) {
			InvertFeld(xs+1, ys+1);
			WaitTime(300);
			InvertFeld(xs+1, ys+1);
		}
		m.OldTyp = m.Typ;
		if (m.TP >= m.Damage) {
			m.TP -= m.Damage;
			m.Status = 1;
			if (m.TP < 2 && Zufall(2) == 1) m.Status = 4;
			if (m.Sprich > 2000 && m.Sprich < 3000)
				DoMonsterDialog(m.Sprich - 2000, m);
		} else { /* schon tot... */
			if (m.Sprich > 3000 && m.Sprich < 4000)
				DoMonsterDialog(m.Sprich - 3000, m);
			DeleteMonster(m.x, m.y);
			Dead = TRUE;
		}
	}

	if (LevelMonAll & LevelFlags) {
		for (j = 1; j <= AnzahlMonster; j++)
			if (Monster[j].Status > 1 && Monster[j].Status < 1000)
				Monster[j].Status = 7;
	} else if (LevelMonType & LevelFlags)
		for (j = 1; j <= AnzahlMonster; j++)
			if (Monster[j].Typ == OldTyp
			 && Monster[j].Status > 1
			 && Monster[j].Status < 1000)
				Monster[j].Status = 7;

	return Dead;
}

/* Schutzwurf eines Monsters *********************************************/

int MonsterSchutzwurf(MonsterTyp *m)
{
	unsigned w;
	w = m.Schaden + m.Bonus * 2;
	if (MonsterMagisch & m.Spezial) w = w * 2; /* magisch */
	if (w > 90) w = 90 /* maximal 95% */
	return Zufall(100) <= w;
}

/* Feld auf begehbar überprüfen ******************************************/

int MonsterFrei(MonsterTyp *m, unsigned xl, unsigned yl)
{
	int frei;
	BITSET FeldTyp;
	unsigned xs, ys, i;
	if (xl == Spieler.x && yl == Spieler.y)
		return (m.Status == 1 || m.Status == 7)
			&& SVersteinert & ~Spieler.Status;
	else if (LevelMonster & Level[xl][yl].Spezial)
		return FALSE;
	else if (LevelNotZyklisch & LevelFlags) /* nicht zyklisch */
		if (xl < 11 || yl < 11
		  || (LevelBreite - xl) < 11 || (LevelHoehe - yl) < 11)
			if (m.x != Spieler.x || m.y != Spieler.y)
				return FALSE;

	FeldTyp = Felder[Level[xl][yl].Feld].Spezial;
	if (FeldAntiMonster & FeldTyp)
		return FALSE;
	if (MonsterWasser & m.Spezial) {
		frei = FeldWasser & FeldTyp;
		if (FeldSumpf & FeldTyp)
			frei = frei && Zufall(10) < 6;
	} else if (MonsterFeuer & m.Spezial) {
		frei = FeldLava & FeldTyp;
		if (FeldSumpf & FeldTyp)
			frei = frei && Zufall(10) < 6;
	} else if (MonsterFlieg & m.Spezial)
		frei = FeldWasser & FeldTyp || FeldBegehbar & FeldTyp;
	else if (MonsterGeist & m.Spezial)
		frei = TRUE;
	else
		frei = FeldBegehbar & FeldTyp && FeldLava & ~FeldTyp
			&& FeldWasser & ~FeldTyp;
	if (!frei) /* vielleicht eine Tür ? */
		if (MonsterTuer & m.Spezial
			&& LevelParameter & Level[xl][yl].Spezial)
		{
			i = FindParameter(xl, yl);
			if (Parameter[i].Art == FTuerZu) {
				Level[xl][yl].Feld = Parameter[i].SpriteOffen;
				Parameter[i].Art = FTuerOffen;
			}
		}
	return frei;
}

/* Sieht das Monster den Spieler ? **************************************/

int MonsterSiehtSpieler(unsigned sx, unsigned sy)
{
	if (SLicht & Spieler.Status || LevelSichtweite != 0)
		return LevelBekannt & SichtBereich[sx][sy].Spezial 
			&& LevelBekannt & SichtBereich[MaxSichtweite][MaxSichtweite].Spezial;
	else
		return LevelSpieler & SichtBereich[sx][sy].Spezial
			&& LevelBekannt & SichtBereich[MaxSichtweite][MaxSichtweite].Spezial;
}

/* Monster bewegen sich *************************************************/

void MonsterBewegung(void)
{
	unsigned i, nochmal;

	void GreifAn(unsigned i, unsigned WM)
	{
		unsigned Wurf, a, s;

		int Parade(GegenstandTyp *r, unsigned Angriff)
		{
#define ParadeMoeglich 4
#define ParadeHalbe 5
			int pariert, unsigned as;
			pariert = FALSE;
			if (r.KennNummer == GWaffe)
				if (ParadeMoeglich & r.Spezial) {
					if (ParadeHalbe & r.Spezial)
						pariert = Zufall(15) + Spieler.Ge >= Angriff;
					else
						pariert = Zufall(20) + Spieler.Ge >= Angriff;
					as = Zufall(r.WaffenSchaden) + r.WaffenBonus;
					if (pariert) {
						if (s > as && Zufall(200) <= s - as) {
							r.KennNummer = 0;
							PrintCharakter(5);
						} else if (r.WaffenAnwendungen > 0) {
							r.WaffenAnwendungen--;
							if (r.WaffenAnwendungen == 0)
								r.KennNummer = 0;
							PrintCharakter(5);
						}
					}
				}
			return pariert;
		}

		Wurf = Zufall(20);
		if (SAusruhen & Spieler.Status)
			Wurf = 20;
		if (Wurf + Monster[i].Trefferwurf >= 20 + Monster[i]. WM && Wurf != 1) {
			if (Monster[i].Schaden > 0)
				s = Zufall(Monster[i].Schaden) + Monster[i].Bonus;
			else
				s = 0;
			if (Wurf == 20) s = s * 2; /* natürliche 20 */
			/* Parademöglichkeiten testen: */
			if (Parade(Spieler.rechteHand, Wurf + Monster[i].Trefferwurf)
				&& SAusruhen & ~Spieler.Status)
			{
				BeginOutput;
				Print(Monster[i].Name);
				Print("#801#"); /* pariert */
				EndOutput;
				if (Spieler.rechteHand.KennNummer == 0)
					OutputText("#807#");
			} else if (Parade(Spieler.linkeHand, Wurf + Monster[i].Trefferwurf)
				&& SAusruhen & ~Spieler.Status)
			{
				BeginOutput;
				Print(Monster[i].Name);
				Print("#801#"); /* pariert */
				EndOutput;
				if (Spieler.linkeHand.KennNummer == 0)
					OutputText("#807#");
			} else { /* leider nicht pariert ... */
				a = 0;
				if (Spieler.Ruestung.KennNummer == GRuestung)
					if (GMagisch & Spieler.Ruestung.Flags
					 || MonsterMagisch & ~Spezial)
						a = Zufall(Spieler.Ruestung.RuestSchutz) +
							     Spieler.Ruestung.RuestBonus;
				if (SSchild & Spieler.Status) /* Zauberschild */
					a += Zufall(15);
				if (SSchutz & Spieler.Status)
					s = 0;  /* Ring Schutz */
				if (s > a) { /* Getroffen und Schaden durchgekommen */
					InvertFeld(SichtMitteX, SichtMitteY);
					WaitTime(300);
					BeginOutput;
					Print(Monster[i].Name);
					Print("#802#");
					PrintCard(s - a, 1);
					Print("#803#");
					EndOutput;
					TrefferPunkte(s - a, FALSE);
					InvertFeld(SichtMitteX, SichtMitteY);
					WaitTime(0);
				} else {
					BeginOutput;
					Print(Monster[i].Name);
					Print("#805#");
					EndOutput;
				}
				if (Spieler.Ruestung.KennNummer == GRuestung) {
					if (s > a && Zufall(500) <= s - a) {
						OutputText("#804#");
						Spieler.Ruestung.KennNummer = 0;
						PrintCharakter(5);
					} else if (Spieler.Ruestung.RuestAnwendungen > 0) {
						Spieler.Ruestung.RuestAnwendungen--;
						if (Spieler.Ruestung.RuestAnwendungen == 0) {
							OutputText("#804#");
							Spieler.Ruestung.KennNummer = 0;
						}
						PrintCharakter(5);
					}
				}
			}
		} else {
			BeginOutput;
			Print(Monster[i].Name);
			Print("#806#");
			EndOutput;
		}
		WaitTime(0); /* ausgeben */
	}

	void MonsterGeht(unsigned i, unsigned zx, unsigned zy)
	{
		unsigned xs, ys, Sprite;
		if (zx == Spieler.x && zy == Spieler.y)
			GreifAn(i, 0);
		else {
			ClearMonster(Monster[i].x, Monster[i].y);
			Monster[i].x = zx;
			Monster[i].y = zy;
		}
	}

	void ZufallsBewegung(unsigned i)
	{
		int mx, my; unsigned xl, yl;
		mx = Monster[i].x;
		my = Monster[i].y;
		switch (Zufall(3)) {
			case 1 : mx++; break; case 2 : mx--; break;
		}
		switch (Zufall(3)) {
			case 1 : my++; break; case 2 : my--; break;
		}
		NormalKoords(mx, my, xl, yl);
		if (MonsterFrei(Monster[i], xl, yl)) 
			MonsterGeht(i, xl, yl);
	}
			
	void WegBewegung(unsigned i)
	{
		unsigned zx,zy; int mx,my;
		if (LevelSichtUmrechnung(Monster[i].x,Monster[i].y,zx,zy)) {
			mx = Monster[i].x; my = Monster[i].y;
			if (zx > MaxSichtweite) mx++;
			else if (zx < MaxSichtweite) mx--;
			if (zy > MaxSichtweite) my++;
			else if (zy < MaxSichtweite) my--;
			NormalKoords(mx, my, zx, zy);
			if (MonsterFrei(Monster[i], zx, zy))
				MonsterGeht(i, zx, zy)
			else
				ZufallsBewegung(i);
		} else ZufallsBewegung(i);
	}

	void ZielBewegung(unsigned i, int aggressiv)
	{
		unsigned zx, zy, mx, my, lx, ly; int dx, dy; int hit, ms;
		if ((SUnsichtbar | SVersteinert) & Spieler.Status) {
			ZufallsBewegung(i);
			return;
		}
		ms = FALSE; zx = 0; zy = 0;
		if (LevelSichtUmrechnung(Monster[i].x, Monster[i].y, zx, zy)) {
			ms = MonsterSiehtSpieler(zx, zy);
			if (ms AND aggressiv AND(MonsterFern & Monster[i].Spezial)) {
				if (Zufall(2) == 1) {
					hit = MakeShoot(zx, zy, MaxSichtweite, MaxSichtweite, 100, TRUE);
					if ((zx == MaxSichtweite && zy == MaxSichtweite)) {
						GreifAn(i, 2); /* WM - 2 auf Fernkampf */
					}
					return;
				}
			}
		}
		if ((Status == 1) OR ms) {
			mx = Monster[i].x; my = Monster[i].y;
			dx = (int)Spieler.x - mx;
			dy = (int)Spieler.y - my;
			if (LevelNotZyklisch & ~LevelFlags) { /* zykl. Level */
				if (ABS(dx) > LevelBreite / 2) { dx = -dx }
				if (ABS(dy) > LevelHoehe / 2) { dy = -dy }
			}
			NormalKoords((int)mx+sign(dx),
				(int)my+sign(dy), lx, ly);
			if (MonsterFrei(Monster[i], lx, ly)) {
				MonsterGeht(i, lx, ly);
				return;
			} else if (ABS(dx) > ABS(dy) 
				&& MonsterFrei(Monster[i], lx, my))
			{
				MonsterGeht(i, lx, my);
				return;
			} else if (ABS(dy) > ABS(dx)
				&& MonsterFrei(Monster[i], mx, ly))
			{
				MonsterGeht(i, mx, ly);
				return;
			}
		}
		ZufallsBewegung(i);
	}

	void RichtungsBewegung(unsigned i)
	{
		int mx, my; unsigned xl, yl;
		mx = Monster[i].x;
		my = Monster[i].y;
		switch (Monster[i].Status) {
		case 11 : mx++; break;
		case 12 : mx++; my++; break;
		case 13 : my++; break;
		case 14 : mx--; my++; break;
		case 15 : mx--; break;
		case 16 : mx--; my--; break;
		case 17 : my--; break;
		case 18 : mx++; my--; break;
		}
		NormalKoords(mx, my, xl, yl);
		if (MonsterFrei(Monster[i], xl, yl))
			MonsterGeht(i, xl, yl);
	}

	void Aktivierung(unsigned i)
	{
		unsigned xl, yl;
		if (LevelSichtUmrechnung(Monster[i].x,Monster[i].y,xl,yl))
			if (LevelBekannt & SichtBereich[xl,yl].Spezial)
				Monster[i].Status = 7;
	}

	void Generator(unsigned i)
	{
		unsigned sx, sy;
		if ((LevelMaxMonster > 0 && AnzahlMonster >= LevelMaxMonster)
			|| AnzahlMonster >= MaxMonster)
			return;
		if (LevelMonster & Level[x][y].Spezial || Zufall(1000) > Monster[i].Sprich)
			return;
		if (LevelSichtUmrechnung(Monster[i].x, Monster[i].y, sx, sy))
			if (LevelBekannt & SichtBereich[sx][sy].Spezial
			 || (sx == MaxSichtweite && sy == MaxSichtweite))
				return;
		AnzahlMonster++;
		Monster[AnzahlMonster] = Monster[i];
		Monster[AnzahlMonster].Status = Status * 1000;
		Monster[AnzahlMonster].Sprich = Bonus / 256;
		Monster[AnzahlMonster].Bonus = Bonus * 256;
	}

	for (i = 1; i <= AnzahlMonster; i++) {
		if (MonsterSchnell & Monster[i].Spezial)
			nochmal = 2;
		else if (MonsterLangsam & Monster[i].Spezial)
			nochmal = Zufall(2) - 1;
		else
			nochmal = 1;
		while (nochmal > 0) {
			switch (Monster[i].Status) {
			case 1 : ZielBewegung(i, TRUE); break;
			case 2 : ZufallsBewegung(i);
				if (Zufall(1000) > (980 + Spieler.Ch)) {
					Monster[i].Status = 7;
				}
				break;
			case 3 : if (Zufall(1000) > (980 + Spieler.Ch)) {
					Monster[i].Status = 7;
				}
				break;
			case 4 : WegBewegung(i);
				if (Zufall(20) == 1) { INC(Monster[i].TP) }
				if (Zufall(1000) > (980 + Spieler.Ch)) {
					Monster[i].Status = 7;
				}
				break;
			case 6 : ZufallsBewegung(i); break;
			case 7 : ZielBewegung(i, TRUE);
				/* if (Zufall(1000) < Spieler.Ch) {
					   Monster[i].Status = Zufall(2) + 1;
				 } */
				break;
			case 8 : WegBewegung(i); break;
			case 9 : if ({SUnsichtbar, SVersteinert} * Spieler.Status == {}) {
					Aktivierung(i);
				}
				break;
			case 10 : ZielBewegung(i, FALSE); break;
			case 11...18 : RichtungsBewegung(i); break;
			default:
				if (Monster[i].Status >= 1000) Generator(i);
			}
			ShowMonster(i);
			nochmal--;
		}
	}
}


/* Monster nur zeigen ****************************************************/

void KeineMonsterBewegung (void)
{
	unsigned i;
	for (i = 1; i <= AnzahlMonster; i++)
		ShowMonster(i);
}

