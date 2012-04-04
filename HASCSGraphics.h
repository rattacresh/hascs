#ifndef HASCSGRAPHICS_H
#define HASCSGRAPHICS_H

#include "compat.h"
/*
   Grafik- und Textausgaben auf den HASCS II Bildschirm.

   Version 1.00 02.08.93
   Version 2.00 03.04.12  ;-)
*/


#define MaxSprites 160

typedef uint16_t SpriteType[16];

typedef SpriteType SpriteArrayType[MaxSprites];

SpriteArrayType FelderSprite, MonsterSprite, SystemSprite, GegenSprite;

/* setzt eine Sprite in einen 40 mal 25 Bildschirm */
void (*SetSprite)(unsigned, unsigned, SpriteType*);

/* setzt einen Buchstaben in einen 80 mal 25 Bildschirm */
void (*SetChar)(unsigned, unsigned, char);

/* setzt einen 4x4 Ausschnitt eines Sprites */
void (*SetSpritePart)(unsigned, unsigned, unsigned, SpriteType*);

/* ein Spritefeld invertieren */
void InvertFeld(unsigned x, unsigned y);

void OutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2);
/* umrahmt das Rechteck mit einer oder zwei Linien */
void DOutlineBar(unsigned x1, unsigned y1, unsigned x2, unsigned y2);

/* Modus für Sprite setzen: ersetzen, oder, und ... */
unsigned SetSpriteMode(unsigned mode);


/* setzt Punkt x, y im Sprite auf Farbe c */
void EditSprite(SpriteType *Sprite, unsigned x,unsigned y,unsigned c);

/* ermittelt die Farbe des Punktes x, y im Sprite */
unsigned GetSprite(SpriteType Sprite, unsigned x, unsigned y);


/* kopiert Bildauschnitt eine Zeile nach oben */
void ScrollUp(unsigned x, unsigned y, unsigned w, unsigned h);

/* Füllt Bildschirmausschnitt */
void Fill(unsigned x, unsigned y, unsigned w, unsigned h, unsigned pattern);

/* Muss vor der Benutzung von HASCSGraphics-Funktionen einmal
   aufgerufen werden. */
void GraphicsInit(void);

#endif /* HASCSGRAPHICS_H */
