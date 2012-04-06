#ifndef HASCSOUTPUT_H
#define HASCSOUTPUT_H

/*

   Textausgabe f�r HASCS II

   written by Alexander Kirchner

   Version 2.0  23.08.89
           2.1  21.09.90 PrintLongCard
           2.2  16.10.90 GetXY
           2.3  27.01.91 Text-Datenstruktur �bernommen
           3.0  29.06.91 Entfernung aller nicht HASCS Funktionen
           3.1  30.06.91 Stringfunktionen
           3.2  09.08.93 StringToInt,cap...

*/

#define MaxText 200      /* maximale Anzahl Texte */

typedef struct {  
	char String[61];
	unsigned Nummer, Sample;
} TextTyp;


unsigned TextMode;       /* 0 = normal, 1 = HASCS */
TextTyp Text[MaxText+1];   /* Ausgabetexte */
unsigned AnzahlTexte;
  
unsigned MaxX; /* letzte m�gliche Cursorposition zur Ausgabe */

void  (*PrintChar)(char); /* ZeichenAusgabe */

/* Bildschirmsteuerung */

void Cls(); /* Ganzen Bildschirm l�schen */
void GotoXY(unsigned x, unsigned y); /* Cursorpositionierung */
void GetXY(unsigned *x, unsigned *y); /* aktuelle Cursorposition */

/* Bildschirmausgaben */

void Print(char *s); /* String ausgeben */
void PrintAt(unsigned x, unsigned y, char *s); /* String an x, y ausgeben */
void PrintCard(unsigned c, unsigned l); /* Zahl ausgeben */
void PrintLongCard(unsigned long x, unsigned z);

/* Bildschirmeingaben */

void InputString(char *s, unsigned l);
void InputCard(unsigned *c, unsigned l);
void InputClick(char *s, unsigned l, unsigned *x, unsigned *y, BITSET *b);

/* Textfensterausgaben */

void BeginOutput(void); /* Bereitet Ausgabe im Textfenster vor */
void EndOutput(void);  /* Schlie�t Ausgabe im Textfenster ab */
void OutputText(char *s); /* beides */
void PrintOutput(); /* gesamtes Textfenster ausgeben */

/* Stringfunktionen */

void CardToString(unsigned c, unsigned l, char *s);
unsigned StringToCard(char *s);
int StringToInt(char *s);
int Compare(char *s, char *p);
int COMPARE(char *s, char *p);
int SMALLER(char *s, char *p);
void Assign(char *s, char *p); /* s := p */
void Concat(char *s, char *p, char *r); /* s := p + r */
unsigned Length(char *s);
void Split(char *p, char *r, char *s, unsigned i);
unsigned FindC(char *s, char c);
int InString(char *s1, char *s2);
char cap(char ch);
void AssignC(char *s, char c);

#endif /* HASCSOUTPUT_H */
