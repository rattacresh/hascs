#include "Image.h" /* Image module */

#include <stdio.h>
#include "HASCSSystem.h"
#include "HASCSDisk.h"
#include "HASCSOutput.h"
#include "HASCSGraphics.h"
#include "HASCSGlobal.h"

typedef struct {
	unsigned Version,
		Length,
		Planes,
		PatternLength,
		PixelWidth,
		PixelHeight,
		LineWidth,
		Lines; /* 2 Bytes Typ! */
	} HeaderType;

typedef unsigned char *BytePtr;

HeaderType *ImageHeader;
unsigned LineBytes, WordAusgleich;


int Decompress(BytePtr source, BytePtr dest)
{
	unsigned long Count, repetitions, i;
	BytePtr d, s;

	void PatternRun(void)
	{
		unsigned long n, i, j;
		n = *source++;
		for (i = 1; i <= n; i++)
			for (j = 1; j <= ImageHeader->PatternLength; j++) {
				d = dest + j - 1 + (i - 1) *
					   (long)ImageHeader->PatternLength;
				s = source + j - 1;
				*d = *s;
			}
		dest = dest + n * (long)ImageHeader->PatternLength;
		Count += n * (long)ImageHeader->PatternLength;
		source = source + (long)ImageHeader->PatternLength;
	}

	void BitString(void)
	{
		unsigned long i;
		i = *source++;
		while (i > 0) {
			CheckSum = (CheckSum + 1 + *source) % 32768;
			*dest++ = *source++; Count++; i--;
		}
	}

	void SolidRun(void)
	{
		unsigned long i;
		i = (unsigned)*source % 128;
		if ((unsigned)*source >= 128)
			while (i > 0) {
				*dest++ = 0xFF; Count++; i--;
			}
		else
			while (i > 0) {
				*dest++ = 0x00; Count++; i--;
			}
		source++;
	}

	Count = 0;
	repetitions = 0;
	do {
		if (*source == 0) {
			source++;
			if (*source == 0) {
				source++; /* Scanline Run */
				if (*source != 255)
					return FALSE; /* Flag nicht gefunden */
				source++;
				repetitions = *source++;
			} else
				source--;
		}
		if (*source == 0) {
			source++;
			if (*source != 0)
				PatternRun();
			else
				source--;
		} else if (*source == 128) {
			source++; BitString();
		} else
			SolidRun();
		if (Count % (long)LineBytes == 0) {
			if (WordAusgleich == 1)
				*dest++ = 0x00;
			while (repetitions > 1) {
				for (i = 0; i <= LineBytes-1+WordAusgleich; i++) {
					d = dest + i;
					s = dest - (LineBytes+WordAusgleich) + i;
					*d = *s;
				}
				dest = dest + (LineBytes+WordAusgleich);
				Count += LineBytes;
				repetitions--;
			}
		}
	} while (Count < (long)ImageHeader->Lines * LineBytes);
	return TRUE;
}


int LoadImageN(unsigned n, unsigned *ref_w, unsigned *ref_h)
{
#define w (*ref_w)
#define h (*ref_h)
	int handle;
	unsigned long length;
	void *ImgBuffer, *Start;
	char Name[128];
	unsigned i, id;
	
	id = n + 1024;
	i = /*HASCSSystem.*/GetCache(id);
	if (i != 0) {
		w = /*HASCSSystem.*/Cache[i].CacheInfo1; h = /*HASCSSystem.*/Cache[i].CacheInfo2;
		/*HASCSSystem.*/SetPicture(/*HASCSSystem.*/Cache[i].CacheInfo1, /*HASCSSystem.*/Cache[i].CacheInfo2, /*HASCSSystem.*/Cache[i].CacheBuffer);
		return TRUE;
	}

	/*HASCSOutput.*/Concat(Name, /*HASCSDisk.*/DiaPath, "PICTURE.000");
	i = LENGTH(Name) - 3;
	Name[i]   = n / 100 + '0';
	Name[i+1] = n % 100 / 10 + '0';
	Name[i+2] = n % 10 + '0';

	CheckSum = 0;
	length = /*HASCSSystem.*/FileLength(Name);
	if (/*HASCSSystem.*/FileError) return FALSE;
	handle = /*HASCSSystem.*/OpenFile(Name);
	ImgBuffer = /*HASCSSystem.*/GetBuffer(length);
	/*HASCSSystem.*/ReadFile(handle, length, ImgBuffer);
	ImageHeader = ImgBuffer;
	if (/*HASCSSystem.*/FileError
		|| ImageHeader->Version != 1
		|| ImageHeader->Length < 8
		|| ImageHeader->Planes != 1)
		return FALSE;
	if (ImageHeader->Length > 8)
		/*HASCSSystem.*/FileSeek(handle, 2 * ImageHeader->Length);
	LineBytes = (ImageHeader->LineWidth + 7) / 8;
	WordAusgleich = LineBytes % 2;
	i = /*HASCSSystem.*/NewCache(id, (long)(LineBytes+WordAusgleich) * 
				 ImageHeader->Lines);
	Start = ImgBuffer + (2 * ImageHeader->Length);
	if (Decompress(Start, /*HASCSSystem.*/Cache[i].CacheBuffer)) {
		/*HASCSSystem.*/Cache[i].CacheInfo1 = ImageHeader->LineWidth;
		/*HASCSSystem.*/Cache[i].CacheInfo2 = ImageHeader->Lines;
		/*HASCSSystem.*/SetPicture(/*HASCSSystem.*/Cache[i].CacheInfo1, /*HASCSSystem.*/Cache[i].CacheInfo2, /*HASCSSystem.*/Cache[i].CacheBuffer);
		w = /*HASCSSystem.*/Cache[i].CacheInfo1; h = /*HASCSSystem.*/Cache[i].CacheInfo2;
	}
	return TRUE;
#undef w
#undef h
}


int SaveImage(char *Name)
{
	typedef unsigned char BildZeile[1000];

	BildZeile z1, z2;

	/*struct { unsigned char x, n, b1, b2; } PatternRun;*/
	struct { unsigned char x, n, flag, c; } ScanlineRun;
	HeaderType ImgHeader;

	unsigned z, counter, LineLength, NewLineLength;
	int fh;
			 
	unsigned FillLine(unsigned z, BildZeile *ref_x)
	{
		unsigned c,  i,  j; BITSET f;

		BITSET GetSpritePattern(unsigned x, unsigned y, unsigned z)
		{
			/*HASCSGraphics.*/SpriteType Sprite; 
			if (/*HASCSGlobal.*/LevelMonster & /*HASCSGlobal.*/Level[x][y].Spezial)
#if 0
				Sprite = /*HASCSGraphics.*/MonsterSprite[/*HASCSGlobal.*/Monster
							       [/*HASCSGlobal.*/FindMonster(x,y)].Typ];
#endif
				memcpy(Sprite,/*HASCSGraphics.*/MonsterSprite[/*HASCSGlobal.*/Monster
							       [/*HASCSGlobal.*/FindMonster(x,y)].Typ], sizeof (SpriteType));
			else if (/*HASCSGlobal.*/LevelGegenstand & /*HASCSGlobal.*/Level[x][y].Spezial)
#if 0
				Sprite = /*HASCSGraphics.*/SystemSprite[/*HASCSGlobal.*/Gegenstand
							       [/*HASCSGlobal.*/FindGegenstand(x,y)].Sprite];
#endif
				memcpy(Sprite, /*HASCSGraphics.*/SystemSprite[/*HASCSGlobal.*/Gegenstand
							       [/*HASCSGlobal.*/FindGegenstand(x,y)].Sprite], sizeof (SpriteType));
			else
#if 0
				Sprite = /*HASCSGraphics.*/FelderSprite[/*HASCSGlobal.*/Level[x][y].Feld];
#endif
				memcpy(Sprite, /*HASCSGraphics.*/FelderSprite[/*HASCSGlobal.*/Level[x][y].Feld], sizeof (SpriteType));
			return Sprite[z % 16];
		 }

#define x (*ref_x)
		i = 0;
		j = 0;
		f = GetSpritePattern(i, z / 16, z);
		do {
			c = 0;
			for (;;) {
				c++;
				if (i+c > /*HASCSGlobal.*/LevelBreite)
					break;
				if (f != GetSpritePattern(i+c, z / 16, z))
					break;
			}
			x[j++] = 0;
			x[j++] = c;
			x[j++] = GetSpritePattern(i, z / 16, z) / 256;
			x[j++] = GetSpritePattern(i, z / 16, z) % 256;
			i = i + c;
			if (i <= /*HASCSGlobal.*/LevelBreite)
				f = GetSpritePattern(i, z / 16, z);
		} while (i <= /*HASCSGlobal.*/LevelBreite);
		return j;
#undef x
	}

	int Ungleich(BildZeile *ref_x, BildZeile *ref_y)
	{
#define x (*ref_x)
#define y (*ref_y)
		unsigned i;
		for (i = 0; i <= LineLength-1; i++)
			if (x[i] != y[i]) return TRUE;
		return FALSE;
#undef x
#undef y
	}
	 
	fh = /*HASCSSystem.*/CreateFile(Name);
	if (/*HASCSSystem.*/FileError) return FALSE;

	ImgHeader.Version = 1;
	ImgHeader.Length  = 8;
	ImgHeader.Planes  = 1;
	ImgHeader.PatternLength = 2;
	ImgHeader.PixelWidth = 0x0174;
	ImgHeader.PixelHeight = 0x0174;
	ImgHeader.LineWidth = (/*HASCSGlobal.*/LevelBreite+1) * 16;
	ImgHeader.Lines = (/*HASCSGlobal.*/LevelHoehe+1) * 16;
	
	/* FIXME machine dependent --rtc */
	/*HASCSSystem.*/WriteFile(fh, sizeof ImgHeader, &ImgHeader);
	if (/*HASCSSystem.*/FileError) return FALSE;
		
	z = 0;
	LineLength = FillLine(z, &z1);

	do {
		counter = 0;
		for (;;) {
			counter++;
			if (counter >= 255 || z+counter >= ImgHeader.Lines) 
				break;
			NewLineLength = FillLine(z+counter, &z2);
			if (Ungleich(&z1, &z2))
				break;
		}
		if (counter > 1) {
			ScanlineRun.x = 0; ScanlineRun.n = 0; ScanlineRun.flag = 0xFF; ScanlineRun.c = counter;
			/*HASCSSystem.*/WriteFile(fh, sizeof ScanlineRun, &ScanlineRun);
			if (/*HASCSSystem.*/FileError) return FALSE;
		}
		/*HASCSSystem.*/WriteFile(fh, LineLength, &z1);
		if (/*HASCSSystem.*/FileError) return FALSE;

		/*z1 = z2;*/memcpy(z1,z2,sizeof (BildZeile)); LineLength = NewLineLength;
		z = z + counter;
	} while (z < ImgHeader.Lines);

	/*HASCSSystem.*/CloseFile(fh);
	return TRUE;
}

