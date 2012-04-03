#ifndef HASCSGRAPHICS_H
#define HASCSGRAPHICS_H

/*
   Grafik- und Textausgaben auf den HASCS II Bildschirm.

   Version 1.00 02.08.93
*/


#define MaxSprites 160

//typedef BITSET SpriteType[16];
typedef unsigned char SpriteType[16*16/8];
typedef unsigned char *SpritePtr;

typedef SpriteType SpriteArrayType[MaxSprites];

SpriteArrayType FelderSprite, MonsterSprite, SystemSprite, GegenSprite;

/* setzt eine Sprite in einen 40 mal 25 Bildschirm */
void (*SetSprite)(unsigned, unsigned, SpritePtr);

/* setzt einen Buchstaben in einen 80 mal 25 Bildschirm */
void (*SetChar)(unsigned, unsigned, char);

/* setzt einen 4x4 Ausschnitt eines Sprites */
void (*SetSpritePart)(unsigned, unsigned, unsigned, SpritePtr);

/* ein Spritefeld invertieren */
void InvertFeld(unsigned x, unsigned y);

/* umrahmt das Rechteck mit einer oder zwei Linien */
void OutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2);
void DOutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2);

/* Modus für Sprite setzen: ersetzen, oder, und ... */
unsigned SetSpriteMode(unsigned mode);


/* setzt Punkt x, y im Sprite auf Farbe c */
void EditSprite(SpritePtr Sprite, unsigned x,unsigned y,unsigned c);

/* ermittelt die Farbe des Punktes x, y im Sprite */
unsigned GetSprite(SpritePtr Sprite, unsigned x, unsigned y);


/* kopiert Bildauschnitt eine Zeile nach oben */
void ScrollUp(unsigned x, unsigned y, unsigned w, unsigned h);

/* Füllt Bildschirmausschnitt */
void Fill(unsigned x, unsigned y, unsigned w, unsigned h, unsigned pattern);


#endif /* HASCSGRAPHICS_H */
