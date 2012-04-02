#ifndef DIALOG_H
#define DIALOG_H

/*
   Dialogbehandlung von HASCS III

   written by A. Kirchner, Version 0.92  21.08.93
                                   1.20  12.06.94

   Dialogsyntax:
   <Ausgabe-String>                         Textausgabe
   #END                                     Ende
   #INPUT                                   Eingabe
   #GOTO <Var>                              Verzweigung
   #LABEL <Var>                             Sprungmarke
   #IF <Ausdruck> THEN <Kommando>           Bedingungsabfrage
   #WINDOW <Var>, <Var>, <Var>, <Var>       Fenstergröße (x, y, w, h)
   #LET <Var> = <Ausdruck>                  Zuweisung
   #WAIT <Var>                              Warten (Zeit, Taste)
   #PICTURE <String>                        Bild anzeigen
   #SELECT <Var>, <Var>, <Var>              Objekt(e) selektieren
   #AIM                                     Spielerselektion
   #COPY                                    Objekt(e) kopieren
   #DELETE                                  Objekt(e) löschen
*/

#include "HASCSGlobal.h"


#define IniDialog    1000
#define FelderDialog 1001
#define ConfigDialog 1002

int LoadDialog(unsigned n, int coded);

void ExecuteDialog();

void DoDialog(unsigned n);

void DoMonsterDialog(unsinged n, MonsterTyp *m);

void DoGegenstandDialog(unsigned n, GegenstandTyp *g);

void DoParameterDialog(unsigned n, ParameterTyp *p);

int SaveDialog(unsigned n, int coded);

void ShowPicture(unsigned n, int New);

#endif /* DIALOG_H */

