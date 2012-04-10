/* HASCSMagic module */
#include "compat.h"
#include "HASCSMagic.h"

#include "HASCSGlobal.h"
#include "HASCSGraphics.h"
#include "HASCSMonster.h"
#include "HASCSSystem.h"

int Vision(unsigned Weite, unsigned Art, unsigned *ref_lx, unsigned *ref_ly)
{
#define lx (*ref_lx)
#define ly (*ref_ly)
#define MaxWeite 44

	unsigned xl, yl, Anzahl, RahmenX, RahmenY, RahmenBreite,
		m, mx, my, x, y;
	SpriteType Sprite;
	BITSET mb;
	char ch;
	int XOff, YOff;

	void ZeigeFeld(unsigned x, unsigned y)
	{
		int zeigen;
		int x1, y1;
		unsigned xk, yk, Farbe;

		zeigen = TRUE;
		x1 = XOff + (int)(x + Spieler.x) - (int)(Anzahl / 2);
		y1 = YOff + (int)(y + Spieler.y) - (int)(Anzahl / 2);
		if (LevelNotZyklisch & LevelFlags) /* nicht zyklisch */
			zeigen = x1 >= 0 && x1 <= (int)LevelBreite
				&& y1 >= 0 && y1 <= (int)LevelHoehe;
		NormalKoords(x1, y1, &xl, &yl);
		if ((1<<0) & Art) /* Karte */
			zeigen = zeigen && LevelKarte & Level[xl][yl].Spezial;
		xk = RahmenX * 4 + 4 + x;
		yk = RahmenY * 4 + 4 + y;
		if (zeigen) {
			Farbe = Felder[Level[xl][yl].Feld].Spezial % 16;
			SetSpritePart(xk, yk, Farbe, &Sprite);
		} else
			SetSpritePart(xk, yk, 0, &SystemSprite[0]);
	}

	/*Sprite = SystemSprite[Art % 2 + 34];*/
	memcpy(Sprite, SystemSprite[Art % 2 + 34], sizeof (SpriteType));

	if (Weite > MaxWeite) Weite = MaxWeite;
	m = LevelBreite;
	if (m < LevelHoehe) m = LevelHoehe;
	if (LevelNotZyklisch & LevelFlags) {
		if (Weite > m) Weite = m;
	} else
		if (Weite > m / 2) Weite = m / 2;

	RahmenBreite = Weite / 2 + 2;
	RahmenY = 11 - Weite / 4;
	RahmenX = 18 - Weite / 4;
	ReserveScreen(RahmenX, RahmenY,
		RahmenX + RahmenBreite, RahmenY + RahmenBreite);
	Anzahl = (RahmenBreite - 1) * 4;
	XOff = 0; YOff = 0;
	for (x = 0; x <= Anzahl-1; x++)
		for (y = 0; y <= Anzahl-1; y++)
			ZeigeFeld(x, y);
	if ((1<<1) & Art) /* keine Verschiebung */
		WaitInput(&mx, &my, &mb, &ch, -1);
	else {
		do {
			WaitInput(&mx, &my, &mb, &ch, -1);
			if (mb) {
				x = mx / 16; y = my / 16;
				if (x == RahmenX && y >= RahmenY
				 && y <= RahmenY + RahmenBreite)
					ch = '4';
				else if (x == RahmenX + RahmenBreite && y >= RahmenY
					&& y <= RahmenY + RahmenBreite)
					ch = '6';
				else if (y == RahmenY && x >= RahmenX 
					&& x <= RahmenX + RahmenBreite)
					ch = '8';
				else if (y == RahmenY + RahmenBreite && x >= RahmenX
					&& x <= RahmenX + RahmenBreite)
					ch = '2';
			}
			if (ch == '6' || ch == '9' || ch == '3') {
				Copy(3, (RahmenX+1)*16+4, (RahmenY+1)*16,
					(RahmenBreite-1)*16-4, (RahmenBreite-1)*16,
					(RahmenX+1)*16, (RahmenY+1)*16);
				XOff++;
				for (x = 0; x <= Anzahl-1; x++)
					ZeigeFeld(Anzahl-1, x);
			}
			if (ch == '1' || ch == '4' || ch == '7') {
				Copy(3, (RahmenX+1)*16, (RahmenY+1)*16,
					(RahmenBreite-1)*16-4, (RahmenBreite-1)*16,
					(RahmenX+1)*16+4, (RahmenY+1)*16);
				XOff--;
				for (x = 0; x <= Anzahl-1; x++)
					ZeigeFeld(0, x);
			}
			if (ch == '7' || ch == '8' || ch == '9') {
				Copy(3, (RahmenX+1)*16, (RahmenY+1)*16,
					(RahmenBreite-1)*16, (RahmenBreite-1)*16-4,
					(RahmenX+1)*16, (RahmenY+1)*16+4);
				YOff--;
				for (x = 0; x <= Anzahl-1; x++)
					ZeigeFeld(x, 0);
			}
			if (ch == '1' || ch == '2' || ch == '3') {
				Copy(3, (RahmenX+1)*16, (RahmenY+1)*16+4,
					(RahmenBreite-1)*16, (RahmenBreite-1)*16-4,
					(RahmenX+1)*16, (RahmenY+1)*16);
				YOff++;
				for (x = 0; x <= Anzahl-1; x++)
					ZeigeFeld(x, Anzahl-1);
			}
			NewXMin = RahmenX; NewYMin = RahmenY;
			NewXMax = RahmenX + RahmenBreite; NewYMax = RahmenY + RahmenBreite;
		} while (ch >= '1' && ch <= '9');
	}
	if (mb)
		if (mx >= (RahmenX+1)*16 && mx < (RahmenX+RahmenBreite-1) * 16
		 && my >= (RahmenY+1)*16 && my < (RahmenY+RahmenBreite-1) * 16)
		{
			mx = (mx - (RahmenX+1)*16) / 4;
			my = (my - (RahmenY+1)*16) / 4;
			NormalKoords((int)(Spieler.x+mx) - (int)(Anzahl / 2) + XOff,
				(int)(Spieler.y+my) - (int)(Anzahl / 2) + YOff, &lx, &ly);
			return TRUE;
		}
	return FALSE;
#undef lx
#undef ly
}

void Todeshauch(unsigned Weite, unsigned Wirkung)
{
	unsigned i, x, y, xl, yl; /*int b;*/
	for (x = MaxSichtweite-Weite; x <= MaxSichtweite+Weite; x++)
		for (y = MaxSichtweite-Weite; y <= MaxSichtweite+Weite; y++)
			InvertFeld(x+1, y+1);
	WaitTime(0);
	for (x = MaxSichtweite-Weite; x <= MaxSichtweite+Weite; x++)
		for (y = MaxSichtweite-Weite; y <= MaxSichtweite+Weite; y++) {
			SichtLevelUmrechnung(x, y, &xl, &yl);
			if (LevelMonster & Level[xl][yl].Spezial) {
				i = FindMonster(xl, yl);
				if (MonsterImmun & ~Monster[i].Spezial)
					/*b =*/ HitMonster(&Monster[i], W6(Wirkung));
			}
		}
	for (x = MaxSichtweite-Weite; x <= MaxSichtweite+Weite; x++)
		for (y = MaxSichtweite-Weite; y <= MaxSichtweite+Weite; y++)
			InvertFeld(x+1, y+1);
}

void Erweckung(unsigned NeuerStatus)
{
	unsigned i, nummer, min, Entfernung;
	nummer = 0; i = 1; min = UINT_MAX;
	while (i <= AnzahlMonster) {
		if (Monster[i].Status == 0) {
			Entfernung = ABS((int)Spieler.x - (int)Monster[i].x)
				+ ABS((int)Spieler.y - (int)Monster[i].y);
			if (Entfernung < min) {
				min = Entfernung;
				nummer = i;
			}
		}
		i++;
	}
	if (nummer != 0)
		Monster[nummer].Status = NeuerStatus;
}

