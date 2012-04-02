#ifndef HASCSGAME_H
#define HASCSGAME_H

#include "HASCSGlobal.h"

void MakeSichtBereich(int Force);
/* erzeugt Sichtbereich und zeigt ihn an */

void Teleport(ParameterTyp *p);
/* teleportiert den Spieler samt Reittier */

unsigned DoGame();
/* führt Spiel durch bis Abbruch */

#endif
