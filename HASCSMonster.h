#ifndef HASCSMONSTER_H
#define HASCSMONSTER_H

#include "HASCSGlobal.h"

/* Pr�ft, ob Monster m das Feld x, y betreten kann */
int MonsterFrei(MonsterTyp *m, unsigned xl, unsigned yl);

/* Monster m erh�ht Damage Schaden */
int HitMonster(MonsterTyp *m, unsigned Damage);

/* Schutzwurf gegen Magie */
int MonsterSchutzwurf(MonsterTyp *m);

/* Monster pariert Angriff */
int MonsterParade(MonsterTyp *m, GegenstandTyp *w, unsigned Treffer);

/* Ist vom Feld Sichbereich[sx, sy] der Spieler zu sehen? */
int MonsterSiehtSpieler(unsigned sx, unsigned sy);

/* f�hrt Monsterbewegung durch */
void MonsterBewegung(void);

/* notwendig zum Erneuern des Sichtbereiches */
void KeineMonsterBewegung(void);


#endif /* HASCSMONSTER_H */
