/* HASCSGraphics module */
#include "compat.h"
#include <arpa/inet.h> /* byte order htons() ntohs()*/
#include <assert.h>
#include "HASCSGraphics.h"

#include "HASCSSystem.h"


#define Black ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7) \
	|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15))

static uint16_t Bildschirm[16000]; /* virtueller Bildschirm */
static uint16_t mask[4] = {
	(1<<15)|(1<<14)|(1<<13)|(1<<12),
	(1<<11)|(1<<10)|(1<< 9)|(1<< 8),
	(1<< 7)|(1<< 6)|(1<< 5)|(1<< 4),
	(1<< 3)|(1<< 2)|(1<< 1)|(1<< 0)
};

/* Setzt ein 16 x 16 Sprite auf Monochrombildschirm */
static void SetMonoSprite(unsigned x,unsigned y, SpriteType *ref_Sprite)
{
#define Sprite (*ref_Sprite)
	register unsigned i,j;
	assert(x<640/16 && y < 400/16);
	i = y * 640 + x;
	for (j = 0; j <= 15; j++) {
		Bildschirm[i] = htons(Sprite[j]);
		i += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
#undef Sprite
}

#if 0
/* Setzt ein 16 x 16 Sprite auf Monochrombildschirm(Oder Modus) */
static void OrMonoSprite(unsigned x,unsigned y, SpriteType *ref_Sprite)
{
#define Sprite (*ref_Sprite)
	register unsigned i, j;
	assert(x<640/16 && y < 400/16);
	i = y * 640 + x;
	for (j = 0; j <= 15; j++) {
		Bildschirm[i] = htons(ntohs(Bildschirm[i]) | Sprite[j]);
		i += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
#undef Sprite
}
#endif

static void SetMonoChar(unsigned x,unsigned y, char ch)
{
	register unsigned i,h;
	uint16_t m;
	assert(x<640/8 && y < 400/16);

	i = y * 640 + x/2;
	m = x % 2 ? 0xff00 : 0x00ff;
	for (h = 0; h <= 15; h++) {
		Bildschirm[i] = htons((ntohs(Bildschirm[i]) & m)
			| (x % 2 ? (ch%2 ? GegenSprite[ch / 2][h] & 0x00ff
					 : GegenSprite[ch / 2][h] >> 8)
			         : (ch%2 ? GegenSprite[ch / 2][h] << 8
					 : GegenSprite[ch / 2][h] & 0xff00)));
		i += 40;
	}
	x = x / 2;
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
}

static void SetMonoSpritePart(unsigned x,unsigned y,unsigned f, SpriteType *ref_Sprite)
{
#define Sprite (*ref_Sprite)
	register unsigned i,j;
	register uint16_t m;
	unsigned z[4];
	assert(x<640/4 && y < 400/4);
	j = f / 4 * 4;
	for (i = 0; i <= 3; i++) {
		switch (f % 4) {
		case 0 : z[i] = Sprite[j + i] / 4096 % 16; break;
		case 1 : z[i] = Sprite[j + i] / 256 % 16; break;
		case 2 : z[i] = Sprite[j + i] / 16 % 16; break;
		case 3 : z[i] = Sprite[j + i] % 16; break;
		}
		switch (x % 4) {
		case 0 : z[i] = z[i] * 4096; break;
		case 1 : z[i] = z[i] * 256; break;
		case 2 : z[i] = z[i] * 16; break;
		}
	}
	j = y * 160 + x / 4; m = mask[x % 4];
	for (i = 0; i <= 3; i++) {
		Bildschirm[j] = htons((ntohs(Bildschirm[j]) & ~m) | z[i]);
		j += 40;
	}
#undef Sprite
}


void EditSprite(SpriteType *ref_Sprite, unsigned x,unsigned y,unsigned c)
{
#define Sprite (*ref_Sprite)
	/* setzt Punkt x,y im Sprite auf Farbe c */
	if (c == 0)
		Sprite[y] &= ~(1<<(15 - x));
	else
		Sprite[y] |= 1<<(15 - x);
#undef Sprite
}


unsigned GetSprite(SpriteType Sprite, unsigned x, unsigned y)
{
	/* ermittelt Farbe des Punktes x, y im Sprite */
	unsigned c;
	if ((1<<(15 - x)) & Sprite[y])
		c = 1;
	else
		c = 0;
	return c;
}


void InvertFeld(unsigned x, unsigned y)
{
	register unsigned i,j;

	assert(x<640/16 && y < 400/16);
	i = 640 * y + x;
	for (j = 0; j <= 15; j++) {
		Bildschirm[i] = ~Bildschirm[i];
		i += 40;
	}
	if (NewXMin > x) NewXMin = x;
	if (NewYMin > y) NewYMin = y;
	if (NewXMax < x) NewXMax = x;
	if (NewYMax < y) NewYMax = y;
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
}
	
static void HorzLine(unsigned x, unsigned y, unsigned w)
{
	unsigned b;
	assert(x<640 && y < 400 && x+w<=640);
	b = 40 * y + x / 16;
	while (w > 0) {
		if ((x % 16) != 0 || w < 16) {
			Bildschirm[b] = htons(ntohs(Bildschirm[b]) | (1 <<(15 - x % 16)));
			x++; w--;
			if (x % 16 == 0) b++;
		} else if (w >= 16) {
			Bildschirm[b] = htons(ntohs(Bildschirm[b]) | Black);
			x += 16; w -= 16; b++;
		}
	}
}

static void VertLine(unsigned x, unsigned y, unsigned h)
{
	unsigned b, m;
	assert(x<640 && y < 400 && y+h<=400);
	b = 40 * y + x / 16;
	m = 1<<(15 - x % 16);
	while (h > 0) {
		Bildschirm[b] = htons(ntohs(Bildschirm[b]) | m);
		b += 40; h--;
	}
}

void OutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2)
{
	assert(x1<640/16 && y1 < 400/16 && x2<640/16 && y2 <400/16);
	HorzLine(x1 * 16, y1 * 16, (x2 - x1 + 1) * 16);
	HorzLine(x1 * 16, y2 * 16 + 15, (x2 - x1 + 1) * 16);
	VertLine(x1 * 16, y1 * 16, (y2 - y1 + 1) * 16);
	VertLine(x2 * 16 + 15, y1 * 16, (y2 - y1 + 1) * 16);
	if (NewXMin > x1) NewXMin = x1;
	if (NewYMin > y1) NewYMin = y1;
	if (NewXMax < x2) NewXMax = x2;
	if (NewYMax < y2) NewYMax = y2;
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
}

void DOutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2)
{
	assert(x1<640/16 && y1 < 400/16 && x2<640/16 && y2 <400/16);
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
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
}

unsigned SetSpriteMode(unsigned mode)
{
	return 0; /* Modus ersetzen */
}


void ScrollUp(unsigned x, unsigned y, unsigned w, unsigned h)
{
	assert(x<640/16 && y < 400/16 && x+w<=640/16 && y+h <=400/16);
	register unsigned a, i, j, k;
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
	if (NewYMax < y + h - 1) NewYMax = y + h - 1;
	assert(NewXMin < 640/16 && NewXMax < 640/16
		&& NewYMin < 400/16 && NewYMax < 400/16
		&& NewXMin <= NewXMax && NewYMin <= NewYMax);
}


void Fill(unsigned x, unsigned y, unsigned w, unsigned h, unsigned pattern)
{
	assert(x<640/16 && y < 400/16 && x+w<=640/16 && y+h <=400/16);
	unsigned a, i, j, k;
	a = 640 * y + x;
	for (k = 1; k <= h; k++)
		for (j = 0; j <= 15; j++) {
			for (i = 0; i <= w - 1; i++)
				Bildschirm[a+i] = htons(pattern);
			a += 40;
		}
}


static void __attribute__ ((constructor)) at_init(void)
{
	SetSprite     = SetMonoSprite;
	SetChar       = SetMonoChar;
	SetSpritePart = SetMonoSpritePart;
	SetBuffer(640, 400, &Bildschirm);
}
