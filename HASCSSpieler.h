#ifndef HASCSSPIELER_H
#define HASCSSPIELER_H


#include "HASCSGlobal.h"


int ZaubernGelungen(void);

int RingAnlegen(GegenstandTyp *r);

void RingAblegen(GegenstandTyp *r);

int RuestungAnlegen(GegenstandTyp *r);

void RuestungAblegen(GegenstandTyp *r);

/* Führt die Spieleraktionen durch */
unsigned SpielerBewegung(unsigned x, unsigned y, BITSET t);



#endif /* HASCSSPIELER_H  */
