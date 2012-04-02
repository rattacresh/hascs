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

struct SoundType = {
  // Sound-Routinen müssen komplett neu geschrieben werden, weswegen ich den folgenden Code
  // nicht übrsetze - Lew
  //CASE : BOOLEAN OF
  //TRUE : Buffer : CharPtr; /* Zeiger auf Sample */
  //| FALSE: b3, b2, b1, b0 : BYTE;
  //END;
 unsigned Length;
 int Frequency;  /* Frequenz in Herz */
};
  

SoundType SoundOff;

char SoundPath[128];
int SoundAusgabe;

void (*PlaySound) (*SoundType);

int LoadSound(unsigned n, SoundType s);

int LoadSoundFile(char *f, unsigned id, SoundType s);

void PlaySoundN(unsigned n);

void LoopSoundN(unsigned n);

#endif /* SOUND_H  */
