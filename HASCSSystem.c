/* HASCSSystem module */

#include <SDL/SDL.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include "HASCSSystem.h"

int ScreenWidth, ScreenHeight; //, ScreenPlanes;

char WName[60];
//WElementSet type;
unsigned win;
		
SDL_Rect desk, work, curr, full, save;
int XOff, YOff;
		
//void *MenuAdr;

SDL_Surface *ScreenMFDB, *BufferMFDB, *PicMFDB;
int CompatBufferMFDB_set = 0;
int CompatPicMFDB_set = 0;
unsigned char *MonoScreen, *MonoBuffer, *MonoPic;
unsigned int MonoBuffer_w, MonoBuffer_h;
unsigned int MonoPic_w, MonoPic_h;

//SearchRec DTABuffer;
char *LastFileName;

unsigned char *BufferAdr;
unsigned long BufferLen;

unsigned long mousetime;
int losgelassen;

/* Min max */

int Max(int a, int b)
{
	if (a > b) return a; else return b;
}

int Min(int a, int b)
{
	if (a < b) return a; else return b;
}

int RcIntersect(SDL_Rect *ref_p1, SDL_Rect *ref_p2)
#define p1 (*ref_p1)
#define p2 (*ref_p2)
{
	SDL_Rect r;

	r.x = Max(p2.x, p1.x);
	r.y = Max(p2.y, p1.y);
	r.w = Min(p2.x + p2.w, p1.x + p1.w);
	r.h = Min(p2.y + p2.h, p1.y + p1.h);
	r.w = r.w - r.x;
	r.h = r.h - r.y;
	p2  = r;
	return p2.w > 0 && p2.h > 0;
}
#undef p1
#undef p2


 /* Programmverwaltung ***********************************************/

void InitWorkstation(char *WinName)
{
	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL konnte nicht initialisiert werden: %s\n", SDL_GetError());
		exit(1);
	}
	
	
	ScreenWidth = 640; // paramptr->rasterWidth + 1;
	ScreenHeight = 400; // paramptr->rasterHeight + 1;
	
	ScreenMFDB = SDL_SetVideoMode(ScreenWidth, ScreenHeight, 16, SDL_HWSURFACE);
	if (ScreenMFDB == NULL) {
		fprintf(stderr, "Ich konnte kein Fenster mit der Auflösung 640*400 öffnen: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_WM_SetCaption(WinName, WinName);    


#if 0

	SDL_Color colors[256];
	for (i = 0; i < 256; i++)
		colors[i].r = colors[i].g = colors[i].b = i;
	colors[0].r = colors[0].g = colors[0].b  = 255;
	SDL_SetPalette(surf, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
	
	SDL_Rect  dst = {0,0,640,400};
#endif


}


void ExitWorkstation(int result)
{
	atexit(SDL_Quit);
}


void *Allocate(unsigned long Bytes)
{
	void *Ptr;
	if (Bytes == 0) 
		return NULL;
	Ptr = malloc(Bytes);
	return Ptr;
}

void *GetBuffer(unsigned long Bytes)
{
    
	/*
	  InOut.WriteLn();
	  InOut.WriteString("GetBuffer: Bytes = ");
	  InOut.WriteLNum(Bytes, 10, 1, ' ');
	*/
	/*

	if (Bytes >= BufferLen) {
		BufferLen = Bytes + 1;
		if (BufferAdr != NULL) {
			if (!Free(BufferAdr))
				Error("Fehler in der Speicherverwaltung(3)", -1);
			BufferAdr = NULL;
		}
		Alloc(BufferLen, BufferAdr);
		if (BufferAdr == NULL)
			Error("Kein Speicher mehr frei!", -1);
		else if (((unsigned long)BufferAdr % 2) != 0)
			Error("Ungerade Pufferadresse!", -1);
	}
	BufferAdr[Bytes] = 0; // Endmarkierung 
	*/
	/*
	  InOut.WriteString("  BufferAdr = ");
	  InOut.WriteLNum(BufferAdr, 10, 1, ' ');
	*/
    
	return BufferAdr;
}

unsigned GetCache(unsigned id)
{
	unsigned i;
    
	/*
	  InOut.WriteLn();
	  InOut.WriteString("GetCache: id = "); InOut.WriteCard(id, 1);
	*/
    
	for (i = 1; i <= AnzCache; i++)
		if (id == Cache[i].CacheId) {
			CacheCounter++;
			Cache[i].CacheUsed = CacheCounter;
	    
			/*
			  InOut.WriteString("  index = "); InOut.WriteCard(i, 1);
			  InOut.WriteString("  buffer = "); InOut.WriteLNum(CacheBuffer, 10, 1, ' ');
			  InOut.WriteString("  bytes = "); InOut.WriteLNum(CacheLength, 10, 1, ' ');
			*/
	    
			return i;
		}
	return 0;
}

void FreeCache(unsigned n)
{
	unsigned i;
	if (n == 0) { // alles löschen 
		for (i = 1; i <= AnzCache; i++)
			free(Cache[i].CacheBuffer);
		AnzCache = 0;
	} else {
		free(Cache[n].CacheBuffer);
		for (i = n; i <= AnzCache - 1; i++)
			Cache[i] = Cache[i+1];
		--AnzCache;
	}
}

unsigned NewCache(unsigned id, unsigned long Bytes)
{
	//unsigned i;
	//void *adr;
    
	unsigned LRUCache(void)
	{
		unsigned i, j, Min; 
		//int ok;
		Min = UINT_MAX; 
		j = 0;
		for (i = 1; i <= AnzCache; i++)
			if (Cache[i].CacheUsed <= Min) {
				j = i;
				Min = Cache[i].CacheUsed;
			}
		if (j != 0)
			return j;
		else
			Error("Kein Speicher mehr frei!", -1);
	}
    
    
	/*
	  InOut.WriteLn();
	  InOut.WriteString("NewCache: id = "); InOut.WriteCard(id, 1);
	  InOut.WriteString("  Bytes = "); InOut.WriteLNum(Bytes, 10, 1, ' ');
	*/

	/*
	i = GetCache(id);
	if (i != 0) FreeCache(i);
	adr = NULL;
	do {
		if (AnzCache < MaxCache) Alloc(Bytes, adr);
		if (adr == NULL) {
			i = LRUCache();
			FreeCache(i);
		}
	} while (adr == NULL);
	CacheCounter++;
	AnzCache++;
	Cache[AnzCache].CacheId = id;
	Cache[AnzCache].CacheBuffer = adr;
	Cache[AnzCache].CacheLength = Bytes;
	Cache[AnzCache].CacheUsed = CacheCounter;
	*/
	return AnzCache;
}

void Deallocate(void *Ptr)
{
	if (Ptr) {
		free(Ptr);
		Ptr = NULL;
	}
}


int LoadAndRun(char *Prg, char *Arg)
{
    
	long result = 0;
	/*
	char path[128], file[128];
	int i, l;
	int ok;
	//Rectangle save;
    
	
	if (StrEqual(Prg, "EDITOR.PRG") || StrEqual(Prg, "HASCSSPR.PRG")
	    || StrEqual(Prg, "HASCSIII.PRG"))
		{
			SplitPath(Name, path, file);
			Concat(path, Prg, file, ok);
		} else
		Assign(Prg, file, ok);
	*/
    
	/* Arg umrechnen in PASCAL String */
	/*
	i = 0; while (Arg[i] != 0) i++; l = i;
	while (i > 0) {
		Arg[i] = Arg[i-1]; i--;
	}
	Arg[0] = l;
	*/
    
	//if (type != 0 && MenuAdr != NIL)
	//MenuBar(MenuAdr, FALSE);
	//save = WindowSize(win, borderSize);
	//CloseWindow(win);
    
	//Pexec(loadExecute, file, Arg, NULL, result);
	/*
	if (result < 0) {
		Concat("Programmstart nicht möglich: ",file, file, ok);
		Error(file, 1);
	}
	*/
    
	//OpenWindow(win, save);
	//if (type != 0 && MenuAdr != NIL)
	//	MenuBar(MenuAdr, TRUE);
	return result;
}

/* Bildschirmverwaltung *********************************************/


void Copy(int direction, int sx, int sy, int width, int height, int dx, int dy)
{
	  SDL_Rect sourceRect, destRect;

	  sourceRect.x = sx; 
	  sourceRect.y = sy;
	  sourceRect.w = width; 
	  sourceRect.h = height;

	  destRect.x = dx; 
	  destRect.y = dy;
	  destRect.w = width; 
	  destRect.h = height;

	  int copy_err;
	  if (direction == 4) {     // Pic    -> Buffer 
		  copy_err = SDL_BlitSurface(PicMFDB, &sourceRect, BufferMFDB, &destRect);	  
	  } else {                  // Buffer -> Buffer 
		  copy_err = SDL_BlitSurface(BufferMFDB, &sourceRect, BufferMFDB, &destRect);	  
	  }
	  
	  if (copy_err)
		  printf("Copy Error: %i\n", copy_err);
}

/**
 * Übergibt ein existierendes monochrome-Bitmap an die Verwaltung von
 * HASCSSystem.
 */
void SetPicture(unsigned width, unsigned height, void *Picture)
{
	PicMFDB = SDL_CreateRGBSurfaceFrom(Picture, width, height, 1, width/8, 0, 0, 0, 0);
	MonoPic = Picture;
	MonoPic_w = width;
	MonoPic_h = height;
	printf("PicMFDB wurde gesetzt\n");
}

/**
 * Übergibt ein existierendes monochrome-Bitmap als Puffer an die
 * Verwaltung von HASCSSystem.
 */
void SetBuffer(unsigned width, unsigned height, void *Buffer)
{
	unsigned pitch = width/8;
	printf("width %i height %i pitch %i\n", width, height, pitch);
	BufferMFDB = SDL_CreateRGBSurfaceFrom(Buffer, width, height, 1, pitch, 0, 0, 0, 0);
	MonoBuffer = Buffer;
	MonoBuffer_w = width;
	MonoBuffer_h = height;
	printf("BufferMFDB wurde gesetzt\n");
}


/* Speicherverwaltung ***********************************************/

int FileName(char *Pattern, char *FileName)
{
	/*
	int result;
	int ok;
	if (StrEqual(LastFileName, Pattern))
		SearchNext(result);
	else {
		SearchFirst(Pattern, 0, result);
		Assign(Pattern, LastFileName, ok);
	}
	//Assign(DTABuffer.name, FileName, ok);
	return result >= 0;
	*/
	return 0;
}

unsigned long FileLength(char *Filename)
{
	/*
	int result;
	SearchFirst(Filename, 0, result);
	FileError = result < 0;
	if (FileError) // nicht gefunden 
		return 0;
	else
		return 0; //DTABuffer.size;
	*/
	return 0;		
}

int OpenFile(char *Name)
{
	/*
	int Handle;
	Open(Name, 0, Handle);
	FileError = Handle < 0;
	return Handle;
	*/
	return 0;
}

void CloseFile(int Handle)
{
	//FileError = !Close(Handle);
}

void DeleteFile(char *Name)
{
	//FileError = Delete(Name);
}

int CreateFile(char *Name)
{
	/*
	int Handle;
	char path[128], file[128];
	Create(Name, 0, Handle);
	if (Handle == -34) { // Path not found 
		SplitPath(Name, path, file);
		path[(sizeof path)-1] = '\0';
		if (!DirCreate(path));
		Create(Name, 0, Handle);
	}
	FileError = Handle < 0;
	return Handle;
	*/
	return 0;
}

void ReadFile(int Handle, unsigned long Bytes, void *Ptr)
{
	/*
	unsigned long Count;
	Count = Bytes;
	Read(Handle, Bytes, Ptr);
	FileError = Bytes != Count;
	*/
}

void WriteFile(int Handle, unsigned long Bytes, void *Ptr)
{
	/*
	unsigned long Count;
	Count = Bytes;
	Write(Handle, Bytes, Ptr);
	FileError = Bytes != Count;
	*/
}

void FileSeek(int Handle, unsigned long pos)
{
	//long ret;
	//Seek(pos, Handle, beginning, ret);
	//FileError = ret != pos;
}

void RenameFile(char *s, char *d)
{
	//Rename(s, d);
}

int SelectFile(char *msg, char *path, char *file)
{
	/*
	int ok;
	int result;
	char pathandfile[128];
    

	  Assign(path, EasyGEM1.SelectMask, ok);
	  Assign(file, pathandfile, ok);
	  GrafMouse(arrow, NIL);
	  EasyGEM1.SelectFile(msg, pathandfile, ok);
	  GrafMouse(bee, NIL);
	  if (!ok) return FALSE;
	  SplitPath(pathandfile, ActPath, file);
	  SetDefaultPath(ActPath, result);
	*/
	return 1;
}


/* Eingaberoutinen **************************************************/

void RedrawWindow(void* frame)
{
	// Ich zeichne einfach das gesamte Fenster neu und
	// ignoriere den Frame:
	SDL_BlitSurface(BufferMFDB, NULL, ScreenMFDB, NULL);
	SDL_Flip(ScreenMFDB);
}

void WaitInput(unsigned *ref_x, unsigned *ref_y, BITSET *ref_b, char *ref_ch, int WarteZeit)
{
	RedrawWindow(NULL);

	SDL_Event event;
	do {
		int wait_err = SDL_WaitEvent(&event);
		if (wait_err == 0)
			printf("Fehler beim Warten auf Events!\n");
	} while ((event.type == SDL_ACTIVEEVENT) || (event.type == SDL_MOUSEMOTION));

	
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		printf("Mouse button %d pressed at (%d,%d)\n", event.button.button, event.button.x, event.button.y);		
		*ref_x = event.button.x;
		*ref_y = event.button.y;
		switch (event.button.button) {
		case 1: 
			*ref_b = MausLinks;
			break;
		case 3: 
			*ref_b = MausRechts;
			break;
		default:
			*ref_b = MausRechts;
		}
                break;
        case SDL_KEYDOWN:
		printf("The %s key was pressed (Code %i)!\n",
		       SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.sym);		
		*ref_ch = event.key.keysym.sym;
		break;
        case SDL_QUIT:
		ExitWorkstation(0);
		exit(0);
		break;
	}

	if (WarteZeit > 0)
		usleep(WarteZeit);
}


/**
 * Wartet auf einen Tastendruck. Zeichnet auch den Fensterinhalt neu.
 */
void WaitKey(void)
{
	RedrawWindow(NULL);

	SDL_Event event;
	while (event.type != SDL_KEYDOWN) {
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT) {
			ExitWorkstation(0);
			exit(0);
		}
	}       
}

/**
 * Wartet t Einheiten und zeichnet vorher den Fensterinhalt neu. Die
 * (von Alexander gedachten) Einheiten von t sind mit unbekannt! Ich
 * übergebe das mal einfach so an usleep.
 */
void WaitTime(unsigned t)
{
	RedrawWindow(NULL);
	if (t > 0)
		usleep(t);
}


unsigned long GetTime()
{
	return time(NULL);
}

unsigned Zufall(unsigned n)
{
	return rand();
}

void SetzeZufall(unsigned long n)
{
	srand(n);
}


void Error(char *s, int Mode)
{
	/*
	char q[256];
	unsigned default;
	int ok;
    
	if (Mode >= 0 && !ShowError)
		return; // keine Fehlermeldung 
	Assign(s, q, ok);
	WrapAlert(q, 0);
	Concat("[3][", q, q, ok);
 default = 1;
 switch (Mode) {
 case -1 : Concat(q, "][ ENDE ]", q, ok); break;
 case 0 : Concat(q, "][ ENDE | WEITER ]", q, ok); default = 2; break;
 case 1 : Concat(q, "][ WEITER ]", q, ok); break;
 case 2 : Concat(q, "][ ABBRUCH | WEITER ]", q, ok); default = 2; break;
 case 3 : Concat(q, "][ ABBRUCH ]", q, ok); break;
 }
 FormAlert(default, q, ErrorResult);
 if (ErrorResult == 1 && Mode <= 0)
	 ExitWorkstation(1);
	*/
}


void PrinterOut(char ch)
{
	/*
	do { } while (!PrnOS());
	PrnOut(ch);
	*/
}

int PrinterStatus()
{
	//return PrnOS();
	return 0;
}

void SystemInit(void)
{
	ShowError = 1;
	FileError = 0;
	LastFileName = "";
	NewXMin = 40; 
	NewYMin = 25; 
	NewXMax = 0; 
	NewYMax = 0;
    
	BufferLen = 0;
	BufferAdr = NULL;
    
	AnzCache = 0; CacheCounter = 0;
    
	losgelassen = 1;
	mousetime = 1;
}
