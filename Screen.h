#ifndef SCREEN_H
#define SCREEN_H

#define MaxObjects 100

#define Exit  0 /* Flags */
#define Editable  1
#define Selectable  2

#define Outlined  8
#define Inverted  9
#define Edited  10
#define Centered  11
#define BigText  12
#define SpriteFill  13

typedef struct {
	unsigned x, y, w, h; /* relativ Position */
	char Text[81], Input[81];
	BITSET Flags; /* Special Properties */
} ObjectType;
  
typedef struct {
	unsigned x, y, w, h;
	char Title[81];
	ObjectType Obj[MaxObjects+1];
	int n; /* Number of Objects */
} ScreenType;
    

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
