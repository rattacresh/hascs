#ifndef SOUND_H
#define SOUND_H

/*
   Sound Routinen für HASCS III
   
   written by A. Kirchner
   
   Version 0.1  10.06.93
           0.5  04.06.94 WAV
           1.0  19.09.94 Interruptplayer

ACHTUNG: Am Programmende muss unbedingt "PlaySound(SoundOff)" aufgerufen
         werden, wenn der Interruptplayer läuft.
*/

const int MaxSounds = 50;

typedef char *CharPtr;

typedef struct {
	union {
		CharPtr Buffer; /* Zeiger auf Sample */
		unsigned char b[4];
	};
	unsigned Length;
	int Frequency;  /* Frequenz in Herz */
} SoundType;
  

SoundType SoundOff;

char SoundPath[128];
int SoundAusgabe;

void (*PlaySound) (*SoundType);

int LoadSound(unsigned n, SoundType s);

int LoadSoundFile(char *f, unsigned id, SoundType s);

void PlaySoundN(unsigned n);

void LoopSoundN(unsigned n);

#endif /* SOUND_H  */
