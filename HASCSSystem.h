#ifndef HASCS_SYSTEM_H
#define HASCS_SYSTEM_H

// DEFINITION MODULE HASCSSystem;

// FROM SYSTEM IMPORT ADDRESS;

/*

    Systemabhängige Programmfunktionen für HASCS II

    written by Alexander Kirchner

    Version 2.0  29.06.91
            3.0  02.07.91
            4.0  01.09.91
            5.0  21.05.92
            6.0  23.09.92 Timer
            7.0  07.02.93 GEM
            7.1  06.06.93 Fehler mit DTA Buffer behoben
            7.2  07.06.93 Rename
            7.3  16.06.93 ExitWorkstation(result), Error(s, type)
            8.0  01.08.93 32K Bildschirm
            8.2  18.08.93 ShowError
            8.5  21.08.93 SetzeZufall

           10.0  16.09.94 Umsetzung auf Megamax Compiler
           11.0  13.10.94 GetBuffer, Cache
*/

// CONST

const int MausLinks = 0; /* linker Mausknopf */
const int MausRechts = 1; /* rechter Mausknopf */
//NULL = ADDRESS(0);
const int MaxCache = 100;

// TYPE
typedef struct {
  int CacheId, CacheUsed, CacheInfo1, CacheInfo2;
  ADDRESS CacheBuffer;
  int CacheLength;
} CacheType;

// VAR
char Name[], Command[], ActPath[];
int FileError; /* Flag: Fehler bei Fileroutinen */
int ShowError; /* Flag: nicht fatale Fehlermeldungen anzeigen */

int NewXMin, NewXMax, NewYMin, NewYMax;

CacheType Cache[]; // ARRAY [1..MaxCache] OF CacheType;
int AnzCache, CacheCounter, ErrorResult;

/* Programmverwaltung ***********************************************/

void InitWorkstation(char *WinName);
void ExitWorkstation(int result);

void Error(char *s, int Mode);
int LoadAndRun(char *Prg, char *Arg);

void *Allocate(unsigned long Bytes);
void Deallocate(void *Ptr);
void *GetBuffer(unsigned long Bytes);

unsigned GetCache(unsigned id);
unsigned NewCache(unsigned id, unsigned long Bytes);
void FreeCache(unsigned n);

/* Bildschirmverwaltung *********************************************/

void Copy(int direction, int sx, int sy, int width, int height, int dx, int dy);
void SetPicture(unsigned width, unsigned height, void *Picture);
void SetBuffer(unsigned width, unsigned height, void *Buffer);

/* Dateiverwaltung **************************************************/

int OpenFile(char *Name);
void CloseFile(int Handle);
int CreateFile(char *Name);
void DeleteFile(char *Name);
void ReadFile(int Handle, unsigned long Bytes, void *Ptr);
void WriteFile(int Handle, unsigned long Bytes, void *Ptr);
unsigned long FileLength(char *Filename);
int FileName(char *Pattern, char *FileName);
void FileSeek(int Handle, unsigned long pos);
void RenameFile(char *s, char *d);
int SelectFile(char *msg, char *path, char *file);

/* Eingaben *********************************************************/

void WaitInput(unsigned x, unsigend y, BITSET *b, char *ch, int WarteZeit);
void WaitKey(void);
void WaitTime(unsigned t);
unsigned long GetTime();

/* Zufallszahl zwischen 1 und n *************************************/

unsigned Zufall(unsigned n);
void SetzeZufall(unsigned long n);

/* Druckerausgaben **************************************************/

void PrinterOut(char ch);
int PrinterStatus();

#endif /* HASCS_SYSTEM */ 
