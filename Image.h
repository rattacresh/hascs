#ifndef IMAGE_H
#define IMAGE_H

/*
   Version 1.00  22.5.92
           2.00  1.1.93   SaveImage
*/


unsigned CheckSum;

/* lädt ein Bild im GEM Image Format und liefert Breite und Höhe */
int LoadImageN(unsigned n, unsigned w, h);

/* speichert das aktuelle Level als GEM Image Bild in eine Datei */
int SaveImage(char *Name);


#endif /* IMAGE_H */
