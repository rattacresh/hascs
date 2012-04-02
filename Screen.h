#ifndef SCREEN_H
#define SCREEN_H

const int MaxObjects = 100;

const int Exit = 0; /* Flags */
const int Editable = 1;
const int Selectable = 2;

const int Outlined = 8;
const int Inverted = 9;
const int Edited = 10;
const int Centered = 11;
const int BigText = 12;
const int SpriteFill = 13;

struct ObjectType = {
  int x, y, w, h; /* relativ Position */
  char Text[81];
  char Input[81];
  BITSET Flags; /* Special Properties */
};
  
struct ScreenType = {
  int x, y, w, h;
  char Title[81];
  ObjectType Obj[MaxObjects];
  int n; /* Number of Objects */
};
    

ScreenType DScreen;


/************************************************************************/

void NewScreen(unsigned sx, unsigned sy, unsigned sw, unsigned sh, char *t);

unsigned AddObject(unsigned ox, unsigned oy, unsigned ow, unsigned, char *t, BITSET f);

/************************************************************************/

void SetInputString(unsigned i, char *s);

void GetInputString(unsigned i, char *s);

void SetInputCard(unsigned i, unsigned c);

unsigned GetInputCard(unsigned i);

unsigned GetFlagSelected(unsigned i);

void SetFlagSelected(unsigned i, unsigned f);

/************************************************************************/

void DrawObject(unsigned i);

void DrawScreen(void);

/************************************************************************/

unsigned HandleScreen(void);

#endif /* SCREEN_H */
