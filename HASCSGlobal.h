#ifndef HASCSGLOBAL_H
#define HASCSGLOBAL_H

/* Definitions-Modul der Datenstrukturen f�r HASCS II

   written by Alexander Kirchner

   Version 1.00 02.04.88 komplette Neuerstellung
           2.00 05.04.88 Parameter wie Gegenst�nde
           3.00 09.05.88 Dialog eingef�hrt
           3.10 21.05.88 Text eingef�hrt
           3.11 22.05.88 neue Feldertypen Lava, Sumpf
           3.12 26.05.88 Dialoge und Texte zusammengef�hrt
           3.20 28.05.88 Gegenstand Licht
           3.30 24.07.88 Basiswertsteigerung durch Lernen
           3.40 24.03.89 Teleport bei Dialogen
           4.00 30.06.89 Variable Textausgaben
           5.00 23.01.90 Charisma, m�nnlich/weiblich, AutoDialog
           5.10 15.02.90 Reittier
           5.20 22.02.90 nicht angreifbare Monster
           5.30 13.03.90 neue Dialogaktionen
           5.40 14.05.90 Gegenstandsdialoge
           6.00 29.05.90 Druckerausgabe, Level�berpr�fung etc.
           6.01 10.06.90 Levelsprites, Monsterblocker
           6.02 25.06.90 Hungerfelder
           6.03 16.07.90 Parameterfelder und Monster auf 200 erh�ht
           6.04 28.08.90 Dialogaktionen Spieler TP, Spieler Nahrung
           6.05 14.09.90 Dialogaktion Zufall, neue Charakterklassen
           6.10 21.09.90 Zugz�hler, Dialogaktion Zugabfrage
           6.20 19.11.90 Dialogaktion Speichern
           6.25 20.01.91 Dialogaktion LevelDialog
           6.30 17.05.91 Dialogaktion GegenHand, LevelParameter
           6.40 24.07.91 Parameterfeld Licht
           6.41 25.07.91 Level sichtbar
           6.50 30.07.91 Spieler Sprite, Dialogaktionen
           7.00 14.09.91 Dialogaktionen, Lichtfeld, Spieler Stati
           7.10 23.09.91 Tageszeiten, Spieler Typ
           7.20 09.05.92 Automapping
           7.30 22.05.92 Bilder in Dialogen
           7.40 15.07.92 Pr�fsummenabrage f�r Bilder, Konfigurationsdatei
           8.00 13.09.92 Programmaufruf in Dialogen
           8.10 28.02.93 Dialogaktion Test Beritten
           8.20 17.05.93 Dialogaktion Ziel Monster, Wait
           9.00 04.08.93 Sichtbereich 23 x 23 Felder
          10.00 10.08.93 Umstellung auf neue Dialoge
          10.10 30.12.93 Ringe in Spezialbits
          11.00 11.08.94 Gegenstands-/Monsterklassen
*/

#include "HASCSGraphics.h"


#define MaxGegen 200      /* maximale Anzahl Gegenst�nde pro Level */
#define MaxMonster 200   /* maximale Anzahl Monster pro Level */
#define MaxPar 200       /* maximale Anzahl Parameterfelder pro Level */
#define MaxRuck 20       /* maximale Anzahl Gegenst�nde im Rucksack */
#define MaxBreite 199    /* maximale Levelbreite */
#define MaxHoehe 199     /* maximale Levelh�he */
#define MaxFlags 20      /* maximale Schalterzahl */
#define MaxNahrung 200   /* maximale Nahrung */

#define MaxSichtweite 11 /* maximale Sichtweite */
#define MaxSichtmal2 22  /* 2 * MaxSichtweite */
#define SichtMitteX 12   /* Koordinaten der Mitte des Sichtbereiches */
#define SichtMitteY 12

/* Spieler Stati **********************************************************/

#define SSchild (1<<0)
#define SSchwimmt (1<<1)
#define SFlink (1<<2)
#define SSchutz (1<<3)
#define SZaubern (1<<4)
#define SUnsichtbar (1<<5)
#define SFeuer (1<<6)
#define SVersteinert (1<<7)
#define SKraft (1<<8)
#define SBetrunken (1<<9)
#define SLicht (1<<10)

#define STot (1<<12)
#define SAusruhen (1<<13)
#define SReitet (1<<14)
#define SMann (1<<15)

/* Spieler Typen **********************************************************/

#define SKrieger (1<<0)
#define SAbenteurer (1<<1)
#define SMagier (1<<2)
#define SPriester (1<<3)
#define SAmazone (1<<4)

/* Gegenst�nde ************************************************************/

#define GRing 1
#define GZauberstab 2
#define GPergament 3
#define GPhiole 4
#define GWaffe 5
#define GRuestung 6
#define GGold 7
#define GSchluessel 8
#define GNahrung 9
#define GLicht 10

#define GMagisch (1<<0)
#define GVerflucht (1<<1)
#define GErkannt (1<<2)
#define GChance (1<<3)
  
/* Felder mit Parametern **************************************************/

#define FLeiterBeide 1
#define FLeiterHoch 2
#define FLeiterRunter 3
#define FTuerOffen 4
#define FTuerZu 5
#define FTuerVerschlossen 6
#define FTeleport 7
#define FDialog 8
#define FFalle 9
#define FFeldAenderung 10
#define FMonsterStatus 11
#define FLicht 12
#define FSound 13
#define FBild 14

/* Feldereigenschaften in Felder[i].Spezial *******************************/

#define FeldBegehbar (1<<0)
#define FeldDurchsichtig (1<<1)
#define FeldWasser (1<<2)       /* 8-14 Schadenspunkte */
#define FeldLava (1<<3)         /* 11-20 Punkte Schaden beim Betreten */
#define FeldSumpf (1<<4)        /* 50 % Chance f�r Bewegung */
#define FeldAntiMonster (1<<5)  /* nicht begehbar f�r Monster */
#define FeldHunger (1<<6)       /* eine Nahrungseinheit pro Zug */
#define FeldSchirm (1<<7)       /* Feld ist f�r Fernkampf nicht zu durchdringen */
  
/* Leveleigenschaften in Level[x,y].Spezial *******************************/

#define LevelMonster (1<<0)     /* hier steht ein Monster */
#define LevelBekannt (1<<1)     /* sichtbar f�r den Spieler */
#define LevelGegenstand (1<<2)  /* hier liegt ein Gegenstand */
#define LevelParameter (1<<3)   /* hier ist ein Parameterfeld */
#define LevelSichtbar (1<<4)    /* dieses Feld ist beleuchtet */
#define LevelKarte (1<<5)       /* Dieses Feld ist in der Karte */
#define LevelSpieler (1<<6)     /* von diesen Feldern wird der Spieler gesehen */

#define LevelNoSave (1<<0)      /* Speichern nicht erlaubt */
#define LevelNoMap (1<<1)       /* keine automatische Karte */
#define LevelNotZyklisch (1<<2) /* Level nicht zyklisch */
#define LevelMonType (1<<3)     /* Monstertypen werden aggressiv */
#define LevelMonAll (1<<4)      /* alle Monster werden aggressiv */
#define LevelAutoDialog (1<<5)  /* Dialog bei Rand�berschreitung */

/* Monstereigenschaften in Monster[i].Spezial *****************************/

#define MonsterMagisch (1<<0)      /* ist magisch */
#define MonsterSchnell (1<<1)      /* bewegt sich doppelt so schnell */
#define MonsterFern (1<<2)         /* kann Fernkampf */
#define MonsterTuer (1<<3)         /* kann T�ren �ffnen */
#define MonsterFlieg (1<<4)        /* geht auch �ber Wasser */
#define MonsterWasser (1<<5)       /* geht nur in Wasser */
#define MonsterGeist (1<<6)        /* geht durch alles durch */
#define MonsterFeuer (1<<7)        /* geht nur durch Feuer/Lava */
#define MonsterReitbar (1<<8)      /* kann geritten werden */
#define MonsterImmun (1<<9)        /* kann nicht angegriffen werden */
#define MonsterPariert (1<<10)     /* kann parieren */
#define MonsterUnsichtbar (1<<11)  /* ist unsichtbar */
#define MonsterLangsam (1<<12)     /* Monster bewegt sich nur mit 50% pro Zug */


typedef char String20Typ[21]; /* allgemeine String Typen */
typedef char String60Typ[61];

// CardSet = SET OF CARDINAL[1..999]; /* Menge der alten Levels */
typedef unsigned CardSet[1000]; /* Menge der alten Levels */

typedef struct {
	unsigned x, y;           /* Koordinaten */
	String20Typ Name;        /* Bezeichnung */
	BITSET Flags;            /* magisch, verlfucht ... */
	unsigned Dialog, Sprite; /* Dialognummer, Spritenummer */
	BITSET Spezial;          /* Sondereigenschaften */

	unsigned KennNummer;
	union { /* maximal 3 unsigned ints */
		struct { unsigned Ring, RingWirkung, RingDauer; };
		struct { unsigned Zauberstab, ZStabWirkung, ZStabAbw; };
		struct { unsigned Pergament, PergamentWirkung, PergamentAnwendungen; };
		struct { unsigned Phiole, PhioleWirkung, PhioleAnwendungen; };
		struct { unsigned WaffenSchaden, WaffenBonus, WaffenAnwendungen; };
		struct { unsigned RuestSchutz, RuestBonus, RuestAnwendungen; };
		struct { unsigned Gold; };
		struct { unsigned SchluesselX, SchluesselY, SchluesselLevel; };
		struct { unsigned Nahrung; };
		struct { unsigned LichtArt, LichtWeite, LichtDauer; };
		struct { unsigned DialogNr, DialogWirkung, DialogAnzahl; };
	};
} GegenstandTyp;

typedef struct {
	String20Typ Name;
	BITSET Spezial; /* begehbar, durchsichtig, Wasser etc. */
} FeldTyp;

typedef struct {
	unsigned x, y;
	unsigned Art;
	union { /* maximal 6 unsigned ints */
		struct { unsigned xhoch, yhoch, Levelhoch, xrunter, yrunter, Levelrunter; };
		struct { unsigned ZielX, ZielY, ZielLevel; };
		struct { unsigned Schaden, Anzahl, Chance1, Chance2, Flag; };
		struct { unsigned Nummer, Zaehler, automatisch; };
		struct { unsigned FeldX, FeldY, FeldNummer, ParNummer; };
		struct { unsigned SpriteOffen, SpriteZu, SpriteVerschlossen; };
		struct { unsigned alterTyp, alterStatus, neuerStatus; };
		struct { unsigned Weite, Dauer; };
	};
} ParameterTyp;


typedef struct {
	String20Typ Name;
	unsigned Trefferwurf, Schaden, Bonus, x, y, Typ, Status, TP;
	unsigned Sprich;
	BITSET Spezial; /* Spezialf�higkeiten */
} MonsterTyp;

typedef struct {
	String20Typ Name;
	unsigned x, y, TPMax, TP, Gold, Nahrung, Grad,
		St, Ko, Ge, In, Zt, Ch,
		Sprite, Sichtweite,
		AnzGegenstaende, LevelNumber, Lernen;
	BITSET Status, Permanent, Typ;
	unsigned long EP, EPnext, Moves;
	MonsterTyp ReitTier;
	GegenstandTyp rechteHand, linkeHand, Ruestung, Ring;
	GegenstandTyp Rucksack[MaxRuck+1];
	unsigned Flags[MaxFlags+1];
	CardSet OldLevels;
} SpielerTyp;

typedef struct {
	unsigned Feld;
	BITSET Spezial;
} LevelTyp;


/* Level-Variablen ********************************************************/

LevelTyp Level[MaxBreite+1][MaxHoehe+1];
unsigned LevelBreite, LevelHoehe;
String20Typ LevelName, LevelSprites;
BITSET LevelFlags;
unsigned LevelSichtweite, LevelDialog, LevelMaxMonster;

/* Daten-Felder ***********************************************************/

FeldTyp Felder[MaxSprites];
GegenstandTyp Gegenstand[MaxGegen+1];
MonsterTyp Monster[MaxMonster+1];
ParameterTyp Parameter[MaxPar+1];

MonsterTyp MonsterKlasse[MaxSprites];
GegenstandTyp GegenKlasse[MaxSprites];

/* Screen-Variablen *******************************************************/

LevelTyp SichtBereich[MaxSichtmal2+1][MaxSichtmal2+1];
unsigned OldScreen [MaxSichtmal2+1][MaxSichtmal2+1];

/* allgemeine Variablen ***************************************************/

SpielerTyp Spieler;
unsigned AnzahlGegen, AnzahlMonster, AnzahlParameter;
int Editor, /* Editiermodus */
	DruckerAusgabe, /* Dialogausgabe auf Drucker */
	SoundAusgabe, /* Ton an */
	DebugMode; /* Sonderfunktionen w�hrend des Spieles */

/* Bildschirm-Ausgaben ****************************************************/

void SetNewSprite(unsigned x, unsigned y);
void SetOldSprite(unsigned x, unsigned y);
int MakeShoot(unsigned *qx, unsigned *qy, unsigned zx, unsigned zy, unsigned time,
	int mode);
void FillRectangle (int x0, int y0, int x1, int y1, SpriteType *Sprite);
void RestoreScreen();
void ReserveScreen(unsigned x1, unsigned y1, unsigned x2, unsigned y2);

void PrintCharakter(unsigned Was);
void PrintMenue(void);
void PrintGegenstand (GegenstandTyp *g);
void PrintLevelName(char *s);

void DisplayCharakter(SpielerTyp *s);

/* Hilfsprozeduren ********************************************************/

unsigned FindMonster(unsigned mx, unsigned my);
unsigned FindGegenstand(unsigned x, unsigned y);
unsigned FindParameter(unsigned x, unsigned y);
void DeleteMonster(unsigned mx, unsigned my);
void DeleteGegenstand(unsigned gx, unsigned gy);
void DeleteParameter(unsigned px, unsigned py);
void NewMonster(unsigned mx, unsigned my, MonsterTyp *m);
void NewGegenstand(unsigned gx, unsigned gy, GegenstandTyp *g);
void NewParameter(unsigned px, unsigned py, ParameterTyp *p);

/* Spieler ****************************************************************/

unsigned W6(unsigned i);
int SchutzWurf(unsigned x);
unsigned SetLightRange();
void SetOneLight(int x, int y, int w, int on);
void SetLightLevel(int clear);
unsigned GetBasiswert(unsigned n);
unsigned ChangeBasiswert(unsigned n, int x);

void Erfahrung(unsigned Punkte);
void TrefferPunkte(unsigned Punkte, int Plus);

int NimmGegenstand(unsigned px, unsigned py, int Einmal,
                          GegenstandTyp *g);
int LegeGegenstand(unsigned px, unsigned py, GegenstandTyp *g);


/* Koordinatentransformationen ********************************************/

void NormalKoords(int xh, int yh, unsigned *i, unsigned *j);
void SichtLevelUmrechnung(unsigned x, unsigned y, unsigned *i, unsigned *j);
int LevelSichtUmrechnung(unsigned x, unsigned y, unsigned *i, unsigned *j);

#endif /* HASCSGLOBAL_H */

