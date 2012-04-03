#include "HASCSGraphics.h" /* HASCSGraphics module */

#include "HASCSSystem.h"


//#define Black ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15))
#define Black 0xff; // ???

//BITSET Bildschirm[16000];
unsigned char Bildschirm[640*400/8]; /* virtueller Bildschirm */

unsigned char *Bild2, /*0..31999*/
	*Sprite2; /*0..31 */
BITSET mask[4];

/* Setzt ein 16 x 16 Sprite auf Monochrombildschirm */
void SetMonoSprite(unsigned x, unsigned y, SpritePtr Sprite)
{
	unsigned i, j;
	i = y * 640 + x;
	for (j = 0; j <= 15; j++) {
		Bildschirm[i] = Sprite[j];
		i += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
}

/* Setzt ein 16 x 16 Sprite auf Monochrombildschirm(Oder Modus) */
void OrMonoSprite(unsigned x, unsigned y, SpritePtr Sprite)
{
	unsigned i, j;
	i = y * 640 + x;
	for (j = 0; j <= 15; j++) {
		Bildschirm[i] = Bildschirm[i] | Sprite[j];
		i += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
} 


void SetMonoChar(unsigned x,unsigned y, char ch)
{
	/*register*/ unsigned i,j,h;

	i = y * 1280 + x;
	Sprite2 = GegenSprite[ch / 2];
	j = ch % 2;
	for (h = 0; h <= 15; h++) {
		Bild2[i] = Sprite2[j];
		i += 80; j += 2;
	}
	x = x / 2;
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
}

void SetMonoSpritePart(unsigned x,unsigned y,unsigned f, unsigned char *Sprite)
{
	/*register*/ unsigned i,j;
	/*register*/ BITSET m;
	unsigned z[4];
	j = f / 4 * 4;
	for (i = 0; i <= 3; i++) {
		switch (f % 4) {
		case 0 : z[i] = Sprite[j + i] / 4096 % 16; break;
		case 1 : z[i] = Sprite[j + i] / 256 % 16; break;
		case 2 : z[i] = Sprite[j + i] / 16 % 16; break;
		case 3 : z[i] = Sprite[j + i] * 16; break;
		}
		switch (x % 4) {
		case 0 : z[i] = z[i] * 4096; break;
		case 1 : z[i] = z[i] * 256; break;
		case 2 : z[i] = z[i] * 16; break;
		}
	}
	j = y * 160 + x / 4; m = mask[x % 4];
	for (i = 0; i <= 3; i++) {
		Bildschirm[j] = (Bildschirm[j] & ~m) | z[i];
		j += 40;
	}
}


/* setzt Punkt x,y im Sprite auf Farbe c */
void EditSprite(SpritePtr Sprite, unsigned x, unsigned y,unsigned c)
{
	if (c == 0)
		Sprite[y] &= ~(1<<(15 - x));
	else
		Sprite[y] |= 1<<(15 - x);
}


/* ermittelt Farbe des Punktes x, y im Sprite */
unsigned GetSprite(SpritePtr Sprite, unsigned x, unsigned y)
{
	unsigned c;
	if ((1<<(15 - x)) & Sprite[y])
		c = 1;
	else
		c = 0;
	return c;
}


void InvertFeld(unsigned x, unsigned y)
{
	/*register*/ unsigned i,j;

	i = 640 * y + x;
	for (j = 0; j <= 15; j++) {
		Bildschirm[i] = ~Bildschirm[i];
		i += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
}
	
void HorzLine(unsigned x, unsigned y, unsigned w)
{
	unsigned b;
	b = 40 * y + x / 16;
	while (w > 0) {
		if ((x % 16) != 0 || w < 16) {
			Bildschirm[b] |= (1 <<(15 - x % 16));
			x++; w--;
			if (x % 16 == 0) b++;
		} else if (w >= 16) {
			Bildschirm[b] = Bildschirm[b] | Black;
			x += 16; w -= 16; b++;
		}
	}
}

void VertLine(unsigned x, unsigned y, unsigned h)
{
	unsigned b, m;
	b = 40 * y + x / 16;
	m = 15 - x % 16;
	while (h > 0) {
		Bildschirm[b] |= m;
		b += 40; h--;
	}
}

void OutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2)
{
	HorzLine(x1 * 16, y1 * 16, (x2 - x1 + 1) * 16);
	HorzLine(x1 * 16, y2 * 16 + 15, (x2 - x1 + 1) * 16);
	VertLine(x1 * 16, y1 * 16, (y2 - y1 + 1) * 16);
	VertLine(x2 * 16 + 15, y1 * 16, (y2 - y1 + 1) * 16);
	if (NewXMin > x1) NewXMin = x1;
	if (NewYMin > y1) NewYMin = y1;
	if (NewXMax < x2) NewXMax = x2;
	if (NewYMax < y2) NewYMax = y2;
}

void DOutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2)
{
	HorzLine(x1 * 16, y1 * 16, (x2 - x1 + 1) * 16);
	HorzLine(x1 * 16, y2 * 16 + 15, (x2 - x1 + 1) * 16);
	VertLine(x1 * 16, y1 * 16, (y2 - y1 + 1) * 16);
	VertLine(x2 * 16 + 15, y1 * 16, (y2 - y1 + 1) * 16);
	HorzLine(x1 * 16, y1 * 16 - 1, (x2 - x1 + 1) * 16);
	HorzLine(x1 * 16, y2 * 16 + 16, (x2 - x1 + 1) * 16);
	VertLine(x1 * 16 - 1, y1 * 16, (y2 - y1 + 1) * 16);
	VertLine(x2 * 16 + 16, y1 * 16, (y2 - y1 + 1) * 16);
	x1--; y1--; x2++; y2++;
	if (NewXMin > x1) NewXMin = x1;
	if (NewYMin > y1) NewYMin = y1;
	if (NewXMax < x2) NewXMax = x2;
	if (NewYMax < y2) NewYMax = y2;
}

unsigned SetSpriteMode(unsigned mode)
{
	return 0; /* Modus ersetzen */
}


void ScrollUp(unsigned x, unsigned y, unsigned w, unsigned h)
{
	/*register*/ unsigned a, i, j, k;
	a = 640 * y + x;
	for (k = 1; k <= h - 1; k++) /* h - 1 Zeilen hochschieben */
		for (j = 0; j <= 15; j++) {
			for (i = 0; i <= w - 1; i++)
				Bildschirm[a+i] = Bildschirm[a+i+640];
			a += 40;
		}
	for (j = 0; j <= 15; j++) { /* unterste Zeile löschen */
		for (i = 0; i <= w - 1; i++)
			Bildschirm[a+i] = 0;
		a += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x + w - 1) NewXMax = x + w - 1;
	if (NewYMax < y + w - 1) NewYMax = y + w - 1;
}


void Fill(unsigned x, unsigned y, unsigned w, unsigned h, unsigned pattern)
{
	unsigned a, i, j, k;
	a = 640 * y + x;
	for (k = 1; k <= h; k++)
		for (j = 0; j <= 15; j++) {
			for (i = 0; i <= w - 1; i++)
				Bildschirm[a+i] = pattern;
			a += 40;
		}	
}


void GraphicsInit(void)
{
	SetSprite     = SetMonoSprite;
	SetChar       = SetMonoChar;
	SetSpritePart = SetMonoSpritePart;
	Bild2         = Bildschirm;
	SetBuffer(640, 400, &Bildschirm);
	mask[0] = (1<<15)|(1<<14)|(1<<13)|(1<<12);
	mask[1] = (1<<11)|(1<<10)|(1<< 9)|(1<< 8);
	mask[2] = (1<< 7)|(1<< 6)|(1<< 5)|(1<< 4);
	mask[3] = (1<< 3)|(1<< 2)|(1<< 1)|(1<< 0);
}
