#ifndef COMPAT_H
#define COMPAT_H
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#define CAP(c) toupper(c)
#define HIGH(s) 1024 /* greater than any string */
#define TRUE 1
#define FALSE 0
#define INSet(a,b) strchr(b,a)
#define BITSET unsigned
#define LENGTH(s) strlen(s)
#if 0
/* compat stubs begin */
#define bee 0
#define NIL NULL
typedef unsigned  DeviceHandle;
typedef unsigned GemHandle;
typedef unsigned WElementSet;
typedef struct {int x, y, w, h;} Rectangle;
typedef struct {void *start;int w, h, words, standardForm, planes;} MemFormDef;
typedef MemFormDef *PtrMemFormDef;
typedef struct {char *name;unsigned long size;} SearchRec;
typedef struct {
	int rasterWidth, rasterHeight, maxRasterPls;
} *PtrDevParm;
void InitGem(int x, DeviceHandle y, int z) {}
void ShellRead(char *x, char *y) {}
void GetDefaultPath(char *x) {}
void GrafMouse(int x, void *y) {}
PtrDevParm DeviceParameter(DeviceHandle x) {return NULL;}
void LoadResource(char *x) {}
int GemError() {return 0;}
void *ResourceAddr(void *x, int y) { return NULL; }
void *treeRsrc = NULL;
void MenuBar(void *x, int y) {}
Rectangle WindowSize(DeviceHandle x, int y) { Rectangle r; return r; }
int workSize = 0;
DeviceHandle DeskHandle = 0;
void CreateWindow(WElementSet x, Rectangle y, unsigned z) {}
Rectangle CalcWindow(unsigned x, unsigned y, Rectangle z) {return z;}
void OpenWindow(unsigned z, Rectangle y) {}
void SetDTA(void *x) {}
GemHandle CurrGemHandle(void) {return 0;}
void ExitGem(GemHandle x) {}
void TermProcess(int x) {}
void Alloc(unsigned long x, void *y) {}
int Free(void *x) {return 0;}

#define calcBorder 0
#define nameBar 0
#define closer 0
#define fuller 0
#define mover 0
#define sizer 0
#define upArrow 0
#define downArrow 0
#define vertSlider 0
#define leftArrow 0
#define rightArrow 0
#define horSlider 0
void Assign(char *x, char *y, int z) {}
void SetWindowString(unsigned x, char *y, char *z) {}
char *nameStr = "";
#define UINT_MAX 0
int StrEqual(char *x, char *y) {return 1;}
void SplitPath(char *x, char *y, char *z) {}
void Concat(char *x, char *y, char *z, int a) {}
#define borderSize 0
void CloseWindow(unsigned x) {}
void Pexec(int x, char *y, char *z, void *a, long b) {}
#define loadExecute 0
void CopyOpaque(DeviceHandle x, PtrMemFormDef y, PtrMemFormDef z,
	Rectangle a, Rectangle b, int f) {}
#define onlyS 0
void SearchNext(int x) {}
void SearchFirst(char *x, int y, int z) {}
int Open(char *x, int y, int z) {return 0;}
int Close(int x) {return 0;}
int Delete(char *x) {return 0;}
void Create(char *x, int y, int z) {}
int DirCreate(char *x) {return 0;}
void Read(int x, unsigned long y, void *z) {}
void Write(int x, unsigned long y, void *z) {}
void Seek(unsigned long x, int y, int z, long a) {}
#define beginning 0
void Rename(char *s, char *d) {}
struct {
	void (*SelectFile)(char *x, char *y, int z);
	char *SelectMask;
} EasyGEM1;
#define arrow 0
void SetDefaultPath(char *x, int y) {}
typedef unsigned EventSet;
/* compat stubs end */
#endif
#endif
