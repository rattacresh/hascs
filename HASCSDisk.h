#ifndef HASCSDISK_H
#define HASCSDISK_H

/*
   Version 22.7.92
           22.5.93
*/

unsigned VersionsNummer,
	BenutzerNummer;
    
int NewSprites;
    
char DiaPath[129], MapPath[129], PlaPath[129], PrgPath[129],
	TextEditor[129], IniFile[129];

void LoadOrSaveDat(int Load, char *FileName);

void LoadOrSaveSprites(int Load, char *FileName);

void LoadLevel(unsigned n);

void SaveLevel(unsigned n);

void LoadOrSavePlayer(int Load);

void LoadOldPlayer();

void DeleteLevels();

void WriteLevel(char *FileName);

void ReadLevel(char *FileName);

#endif
