#ifndef HASCSMAGIC_H
#define HASCSMAGIC_H

/* Definitionsmodul f�r HASCS II

   enth�lt spezielle Prozeduren f�r die Magie

   Version 1.00  15.04.88
           1.10  09.05.92 Automapping
*/

/* Zeichnet verkleinerte Karte des Levels */
int Vision(unsigned Weite, unsigned Art, unsigned *lx, unsigned *ly);

void Todeshauch(unsigned Weite, unsigned Wirkung);

void Erweckung(unsigned NeuerStatus);

#endif /* HASCSMAGIC_H  */
