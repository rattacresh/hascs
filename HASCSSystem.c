/* HASCSSystem module */
#include <SDL/SDL.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <sys/stat.h>
#include <errno.h>

#include "compat.h"
#include "HASCSSystem.h"

#include "HASCSGraphics.h"

#define StrEqual(p,q) (!strcmp(p,q))
void SplitPath(char *n,char *p,char *f)
{
	char *x = strrchr(n, '/');
	if (!x) {
		strcpy(f, n);
		strcpy(p, ".");
	} else {
		strcpy(f,x+1);
		strncpy(p,n,x-n);
		p[x-n] = '\0';
	}
}
#define Concat(x, y, z, a) (strcat(strcpy(z,x),y), memmove(&ok,&ok,0))
#define Assign(x,y,a) (strcpy(y,x), memmove(&ok,&ok,0))

int ScreenWidth, ScreenHeight; //, ScreenPlanes;

char WName[60];
static int type;
unsigned win;
		
SDL_Rect desk, work, curr, full, save;
int XOff, YOff;
		
//void *MenuAdr;

int ShowError = TRUE;
int FileError = FALSE;
unsigned NewXMin = 40, NewYMin = 25, NewXMax = 0, NewYMax = 0;
    
unsigned AnzCache = 0, CacheCounter = 0;
    
SDL_Surface *ScreenMFDBAdr, *BufferMFDBAdr, *PicMFDBAdr;

//SearchRec DTABuffer;
struct stat StatBuf;
glob_t *GlobBufAdr;
int GlobCounter;
char *LastFileName = "";

unsigned char *BufferAdr = NULL;
unsigned long BufferLen = 0;

unsigned long mousetime = 1;
int losgelassen = TRUE;

/* Min max */

static int Max(int a, int b)
{
	if (a > b) return a; else return b;
}

static int Min(int a, int b)
{
	if (a < b) return a; else return b;
}

int RcIntersect(SDL_Rect *ref_p1, SDL_Rect *ref_p2)
{
#define p1 (*ref_p1)
#define p2 (*ref_p2)
	SDL_Rect r;

	r.x = Max(p2.x, p1.x);
	r.y = Max(p2.y, p1.y);
	r.w = Min(p2.x + p2.w, p1.x + p1.w);
	r.h = Min(p2.y + p2.h, p1.y + p1.h);
	r.w = r.w - r.x;
	r.h = r.h - r.y;
	p2  = r;
	return p2.w > 0 && p2.h > 0;
#undef p1
#undef p2
}

 /* Programmverwaltung ***********************************************/

void InitWorkstation(char *WinName)
{
	extern int __argc; extern char **__argv;
	static char cwd[128], cmd[128] = {};
	extern char *program_invocation_name;

	Name = program_invocation_name;
	Command = strncpy(cmd, __argc > 1 ? __argv[1] : "", sizeof cmd - 1);
	ActPath = getcwd(cwd, sizeof cwd);

	if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "SDL konnte nicht initialisiert werden: %s\n", SDL_GetError());
		exit(1);
	}
	
	
	ScreenWidth = 640; // paramptr->rasterWidth + 1;
	ScreenHeight = 400; // paramptr->rasterHeight + 1;
	
	ScreenMFDBAdr = SDL_SetVideoMode(ScreenWidth, ScreenHeight, 16, SDL_HWSURFACE);
	if (ScreenMFDBAdr == NULL) {
		fprintf(stderr, "Ich konnte kein Fenster mit der Auflösung %ix%i öffnen: %s\n", ScreenWidth, ScreenHeight, SDL_GetError());
		exit(1);
	}
	
	SDL_WM_SetCaption(WinName, WinName);    
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
		SDL_DEFAULT_REPEAT_INTERVAL);
#if 0

	SDL_Color colors[256];
	for (i = 0; i < 256; i++)
		colors[i].r = colors[i].g = colors[i].b = i;
	colors[0].r = colors[0].g = colors[0].b  = 255;
	SDL_SetPalette(surf, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
	
	SDL_Rect  dst = {0,0,640,400};
#endif
#if 1
	// Ich zeichne einfach das gesamte Fenster neu und
	// ignoriere den Frame:
	SDL_BlitSurface(BufferMFDBAdr, NULL, ScreenMFDBAdr, NULL);
	SDL_Flip(ScreenMFDBAdr);
#endif
}

/**
 * Beendet das gesamte Programm.
 */
void ExitWorkstation(int result)
{
	if (PicMFDBAdr)
		SDL_FreeSurface(BufferMFDBAdr);
	if (BufferMFDBAdr)
		SDL_FreeSurface(BufferMFDBAdr);
	atexit(SDL_Quit);
	exit(result);
}

/**
 * Alloziert einen Speicherbereich von Bytes Größe im RAM und liefert
 * einen Pointer darauf zurück.
 */
void *Allocate(unsigned long Bytes)
{
	if (!Bytes) 
		return NULL;
	return malloc(Bytes);
}

/**
 * Liefert einen Pointer auf den HASCS-Systempuffer. Übergeben wird
 * die erwünschte Puffergröße in Bytes. Sollte der Systempuffer zu
 * klein sein, so wird er vergrößert. Dabei werden jedoch die
 * bisherigen Inhalte gelöscht.
 */
void *GetBuffer(unsigned long Bytes)
{    
	printf("\n");
	printf("GetBuffer: Bytes = %lu\n", Bytes);

	if (Bytes >= BufferLen) {
		BufferLen = Bytes + 1;
		if (BufferAdr != NULL) {
			free(BufferAdr);
			BufferAdr = NULL;
		}
		BufferAdr = malloc(BufferLen);
		if (BufferAdr == NULL)
			Error("Kein Speicher mehr frei!", -1);
		else if (((unsigned long)BufferAdr % 2) != 0)
			Error("Ungerade Pufferadresse!", -1);
	}
	BufferAdr[Bytes] = 0; // Endmarkierung 

	printf("  BufferAdr = %p\n", BufferAdr);
    
	return BufferAdr;
}

/**
 * Liefert den index des Caches mit der übergebenen id. Gibt es keinen
 * Cache mit dieser id, so wird 0 zurückgegeben.
 */
unsigned GetCache(unsigned id)
{
	unsigned i;
    
	printf("\n");
	printf("GetCache: id = %u\n", id);
    
	for (i = 1; i <= AnzCache; i++)
		if (id == Cache[i].CacheId) {
			CacheCounter++;
			Cache[i].CacheUsed = CacheCounter;

			printf("  index = %u\n", i);
			printf("  buffer = %p\n", Cache[i].CacheBuffer);
			printf("  bytes = %i\n", Cache[i].CacheLength);
			
			return i;
		}
	return 0;
}

/**
 * Ein Cache wird gelöscht. Der übergebene Parameter ist der index,
 * nicht die id des Caches.
 */
void FreeCache(unsigned n)
{
	unsigned i;
	if (n == 0) { /* alles löschen */
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

/**
 * Legt einen neuen Cache mit der ID id und der Größe Bytes an.
 * Liefert den index des neues Caches zurück.
 */
unsigned NewCache(unsigned id, unsigned long Bytes)
{
	unsigned i;
	void *adr;
    
	unsigned LRUCache(void)
	{
		unsigned i, j, Min; 
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

		return 0; // Der Rückgabewert wird vom Aufrufer (unten) gar nicht verwendet.
	}
    
	printf("NewCache: id = %u\n", id);
	printf("  Bytes = %lu\n", Bytes);

	i = GetCache(id);
	if (i != 0) 
		FreeCache(i);
	adr = NULL;
	do {
		if (AnzCache < MaxCache) 
			adr = malloc(Bytes);
		if (adr == NULL) {
			i = LRUCache();
			FreeCache(i);
		}
	} while (adr == NULL);
	++CacheCounter;
	++AnzCache;
	Cache[AnzCache].CacheId = id;
	Cache[AnzCache].CacheBuffer = adr;
	Cache[AnzCache].CacheLength = Bytes;
	Cache[AnzCache].CacheUsed = CacheCounter;

	return AnzCache;
}

void Deallocate(void **ref_Ptr)
{
#define Ptr (*ref_Ptr)
	if (Ptr != NULL) {
		free(Ptr);
		Ptr = NULL;
	}
#undef Ptr
}

/**
 * Startet ein externes Programm. Wird momentan nur von HASCSEditor
 * zum Starten eines externen Texteditors genutzt. IMHO brauchen wir
 * das nicht, da man heutzutage in einer Multitasking-Umgebung lebt,
 * in der man sich den Texteditor auch anderweitig starten kann.
 *
 * Ich ignoriere die Args, da sie nirgends in HASCS übergeben werden.
 */
int LoadAndRun(char *Prg, char *Arg)
{    
	long result;
	char path[128], file[128];
	/*int i, l;*/
	int ok;
	/*SDL_Rect save;*/


	if (StrEqual(Prg, "EDITOR.PRG") || StrEqual(Prg, "HASCSSPR.PRG")
		|| StrEqual(Prg, "HASCSIII.PRG"))
	{
		SplitPath(Name, path, file);
		Concat(path, Prg, file, ok);
	} else
		Assign(Prg, file, ok);
   
	result = system(Prg);

	if (result < 0) {
		/* FIXME overlap --rtc */
		Concat("Programmstart nicht möglich: ",file, file, ok);
		Error(file, 1);
	}
    
	return result;
}

/* Bildschirmverwaltung *********************************************/

/**
 * Kopiert einen rechteckigen Bereich auf dem Bildschirmpuffer oder
 * zwischen Pic-Buffer und Bildschirmpuffer.
 *
 * TODO: Scheint noch nicht richtig zu funktionieren. Es wird zwar ein
 * Bildbereich kopiert, aber nicht der, der angegeben wurde.
 */
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
	  if (direction == 4) {     /* Pic    -> Buffer */
		  copy_err = SDL_BlitSurface(PicMFDBAdr, &sourceRect, BufferMFDBAdr, &destRect);	  
	  } else {                  /* Buffer -> Buffer  */
		  copy_err = SDL_BlitSurface(BufferMFDBAdr, &sourceRect, BufferMFDBAdr, &destRect);	  
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
	if (PicMFDBAdr)
		SDL_FreeSurface(BufferMFDBAdr);
	PicMFDBAdr = SDL_CreateRGBSurfaceFrom(Picture, width, height, 1, width/8, 0, 0, 0, 0);
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
	if (BufferMFDBAdr)
		SDL_FreeSurface(BufferMFDBAdr);
	BufferMFDBAdr = SDL_CreateRGBSurfaceFrom(Buffer, width, height, 1, pitch, 0, 0, 0, 0);
	printf("BufferMFDB wurde gesetzt\n");
}


/* Speicherverwaltung ***********************************************/

int FileName(char *Pattern, char *FileName)
{
	int result;
	int ok;
	if (StrEqual(LastFileName, Pattern)) {
		result = 0;
		if (GlobCounter < GlobBufAdr->gl_pathc - 1)
			GlobCounter++;
		else
			result = -1;
	} else {
		if (GlobBufAdr)
			globfree(GlobBufAdr);
		result = glob(Pattern, 0, NULL, GlobBufAdr);
		GlobCounter = 0;
		Assign(Pattern, LastFileName, ok);
	}
	Assign(GlobBufAdr->gl_pathv[GlobCounter], FileName, ok);
	return result >= 0;
}

/**
 * Liefert die Größe einer Datei in Bytes zurück. Falls die Datei
 * nicht existiert, so wird 0 zurückgegeben.
 */
unsigned long FileLength(char *Filename)
{
	int result;

	result = stat(Filename, &StatBuf);

	FileError = result < 0;
	if (FileError) /* nicht gefunden */
		return 0;
	else
		return StatBuf.st_size;
}

int OpenFile(char *Name)
{
	int Handle;
	
	Handle = open(Name, 0);
	FileError = Handle < 0;
	return Handle;
}

void CloseFile(int Handle)
{
	FileError = close(Handle) < 0;
	//	if (FileError)
	//	Error("FileLength: Fehler beim Schließen einer Datei!", 1);	
}

void DeleteFile(char *Name)
{
	FileError = remove(Name) < 0;
}

/**
 * Erzeugt eine neue Datei -- oder überschreibt eine
 * existierende. TODO: soll auch Pfade erzeugen, wenn die noch nicht
 * da sind.
 */
int CreateFile(char *Name)
{
	int Handle;
	char path[128], file[128];
	Handle = creat(Name, 0666);
	if (Handle < 0 && errno == ENOENT) { /* Path not found */
		SplitPath(Name, path, file);
		path[LENGTH(path)-1] = '\0';
		if (mkdir(path, 0777) < 0) {
		}
		Handle = creat(Name, 0666);
	}
	FileError = Handle < 0;
	return Handle;
}

void ReadFile(int Handle, unsigned long Bytes, void *Ptr)
{
	unsigned long Count;
	Count = read(Handle, Ptr, Bytes);
	FileError = Bytes != Count;
}

void WriteFile(int Handle, unsigned long Bytes, void *Ptr)
{
	unsigned long Count;
	Count = write(Handle, Ptr, Bytes);
	FileError = Bytes != Count;
}

void FileSeek(int Handle, unsigned long pos)
{
	unsigned long ret;
	ret = lseek(Handle, pos, SEEK_SET);
	FileError = ret != pos;
}

void RenameFile(char *s, char *d)
{
	rename(s, d);
}

/**
 * Soll eine File-Select-Box öffnen. Die gibt es im SDL nicht -- wir
 * müssen sie selbst schreiben (TODO). Wird hauptsächlich im
 * HASCSEditor verwendet, so dass wir auch erst die Engine fertig
 * machen können.
 */
int SelectFile(char *msg, char *path, char *file)
{
	int ok;
	/*int result;*/
	char pathandfile[128];
    

	/*Assign(path, EasyGEM1.SelectMask, ok);*/
	Assign(file, pathandfile, ok);
	/*GrafMouse(arrow, NIL);
	EasyGEM1.SelectFile(msg, pathandfile, ok);
	GrafMouse(bee, NIL);*/
	printf("Select [%s] [%s]: ", path, pathandfile);
	fgets(pathandfile, sizeof pathandfile, stdin);
	ok = 1;

	if (!ok) return FALSE;
	SplitPath(pathandfile, ActPath, file);
	/*result = */chdir(ActPath);
	return 1;
}


/* Eingaberoutinen **************************************************/

void WaitInput(unsigned *ref_x, unsigned *ref_y, BITSET *ref_b, char *ref_ch, int WarteZeit)
{
#define xx (*ref_x)
#define yy (*ref_y)
#define ch (*ref_ch)
#define b (*ref_b)

#define keyboard (1<<SDL_KEYDOWN)
#define mouseButton ((1<<SDL_MOUSEBUTTONDOWN)|(1<<SDL_MOUSEBUTTONDOWN))
#define message ((1<<SDL_VIDEORESIZE)|(1<<SDL_VIDEOEXPOSE)\
		|(1<<SDL_QUIT)|(1<<SDL_SYSWMEVENT)|(1<<SDL_ACTIVEEVENT))
#define timer (1<<SDL_USEREVENT)
#define msBut1 SDL_BUTTON_LEFT
#define msBut2 SDL_BUTTON_RIGHT
#define lookForEntry SDL_MOUSEMOTION

	typedef BITSET EventSet;
	typedef struct {
		unsigned x, y;
	} Point;
	EventSet flags, events;
	unsigned mouse;
	SDL_Event msg;
	SDL_Rect rect;
	Point mLoc;
	unsigned mButtons;
	SDLMod keyState;
	SDL_keysym key;
	unsigned doneClicks;
	int ok;
	unsigned long time;
	
	void RedrawWindow(SDL_Rect frame)
	{
		SDL_Rect r, s;
		/*UpdateWindow(TRUE);*/
		r.x = r.y = 0;
		r.w = ScreenMFDBAdr->w;
		r.h = ScreenMFDBAdr->h;
		if (RcIntersect(&frame, &r)) {
			/*GrafMouse(mouseOff, NIL);*/
			/* Pufferkoordinaten */
			s.x = r.x - work.x + XOff;
			s.y = r.y - work.y + YOff;
			s.w = r.w; s.h = r.h;
			SDL_BlitSurface(BufferMFDBAdr, &s, ScreenMFDBAdr, &r);
			printf("Update %d %d %d %d", r.x, r.y, r.w, r.h);
			SDL_UpdateRect(ScreenMFDBAdr, r.x, r.y, r.w, r.h);
			/*GrafMouse(mouseOn, NIL);*/
		}
		/*UpdateWindow(FALSE);*/
	}

	void SetSlider(void)
	{
#if 0
		int pos, size, oldpos, oldsize;
		size = 1000 * (long)work.w / 640;
		pos = 0;
		if (work.w < 640)
			pos = 1000 * (long)XOff / (640 - work.w);
		oldpos = WindowSliderValue(win, horPosition);
		oldsize = WindowSliderValue(win, horSize);
		if (oldpos != pos)
			SetWindowSlider(win, horPosition, pos);
		if (oldsize != size)
			SetWindowSlider(win, horSize, size);
		size = 1000 * (long)work.h / 400;
		pos = 0;
		if (work.h < 400)
			pos = 1000 * (long)YOff / (400 - work.h);
		oldpos = WindowSliderValue(win, vertPosition);
		oldsize = WindowSliderValue(win, vertSize);
		if (oldpos != pos)
			SetWindowSlider(win, vertPosition, pos);
		if (oldsize != size)
			SetWindowSlider(win, vertSize, size);
#endif
	}

	void VollBild(void)
	{
		if (type == 0) { /* Fenster wieder normal */
			ScreenMFDBAdr = SDL_SetVideoMode(0, 0, 0, 
				ScreenMFDBAdr->flags & ~SDL_FULLSCREEN);
			type = 1;
		} else {
			save.w = ScreenMFDBAdr->w;
			save.h = ScreenMFDBAdr->h;
			type = 0;
			ScreenMFDBAdr = SDL_SetVideoMode(0, 0, 0,
				ScreenMFDBAdr->flags | SDL_FULLSCREEN);
			XOff = 0; YOff = 0;
		}
		work.x = 0;
		work.y = 0;
		work.w = ScreenMFDBAdr->w;
		work.h = ScreenMFDBAdr->h;
		RedrawWindow(work);
	}

	void Ende(void)
	{
		/*int dummy;*/
		/*Error("HASCS III wirklich beenden?", 0);*/
		ExitWorkstation(0);
		exit(0);
	}

	void Correct(int *ref_x, int *ref_y, Uint16 *ref_w, Uint16 *ref_h)
	{
#define x (*ref_x)
#define y (*ref_y)
#define w (*ref_w)
#define h (*ref_h)
		w = Min(w, 640);
		h = Min(h, 400);
		x = Min(x, 640-w); x = Max(0, x); /* XOff */
		y = Min(y, 400-h); y = Max(0, y); /* YOff */
#undef x
#undef y
#undef w
#undef h
	}

	void Button(void)
	{
		ok = mLoc.x >= work.x && mLoc.y >= work.y
			&& mLoc.x < work.x + work.w && mLoc.y < work.y + work.h;
		if (ok) {
			xx = mLoc.x - work.x + XOff;
			yy = mLoc.y - work.y + YOff;
			if (msBut1 & mButtons) b |= (1<<0);
			if (msBut2 & mButtons) b |= (1<<1);
		}
	}

	void Keyboard(void)
	{
		int Redraw;
		Redraw = FALSE;
		switch (key.sym) {
		case SDLK_LEFT : XOff -= 16; Redraw = TRUE; break;
		case SDLK_RIGHT : XOff += 16; Redraw = TRUE; break;
		case SDLK_UP : YOff -= 16; Redraw = TRUE; break;
		case SDLK_DOWN : YOff += 16; Redraw = TRUE; break;
		case SDLK_q : if (key.mod & KMOD_CTRL) { /* Control Q */
				Ende();  key.sym = '\0';
			}
			break;
		case SDLK_f : if (key.mod & KMOD_CTRL) { /* Control F */
				VollBild(); key.sym = '\0';
			}
			break;
		case SDLK_l : if (key.mod & KMOD_CTRL) { /* Control L */
				FreeCache(0); key.sym = '\0';
			}
		default:
			break;
		}
		if (Redraw) {
			Correct(&XOff, &YOff, &work.w, &work.h);
			RedrawWindow(work);
			SetSlider();
		}
		ch = key.sym;
		if (ch >= '1' && ch <= '9' && timer & flags)
			ch = '\0'; /* Richtungstasten löschen */
		ok = ch != '\0';
	}
	void Message(void)
	{
#if 0
		void SelectMenu(int item)
		{
			unsigned exit;
			void *DialogAdr;
			Rectangle startrec;

			startrec.x = 0; startrec.y = 0; startrec.w = 0; startrec.h = 0;
			switch (item) {
			case  7: DialogAdr = ResourceAddr(treeRsrc, 1);
				DoSimpleBox(DialogAdr, startrec, exit);
				 break;
			case 19: Ende(); break;
			case 17: FreeCache(0); break;
			case 16: VollBild(); break;
			}
		}

		void ArrowWindow(ArrowedMode a)
		{
			switch (a) {
			case pageUp      : YOff -= work.h; break;
			case pageDown    : YOff += work.h; break;
			case rowUp       : YOff -= 16; break;
			case rowDown     : YOff += 16; break;
			case pageLeft    : XOff -= work.w; break;
			case pageRight   : XOff += work.w; break;
			case columnLeft  : XOff -= 16; break;
			case columnRight : XOff += 16; break;
			}
			Correct(XOff, YOff, work.w, work.h);
			RedrawWindow(work);
		}
#endif

		switch (msg.type) {
			SDL_Rect rdrwFrame;
		case SDL_VIDEOEXPOSE :
			rdrwFrame.x = 0;
			rdrwFrame.y = 0;
			rdrwFrame.w = ScreenMFDBAdr->w;
			rdrwFrame.h = ScreenMFDBAdr->h;
			RedrawWindow(rdrwFrame);
			break;
		case SDL_ACTIVEEVENT :
			/*SetTopWindow(win);*/
			break;
		case SDL_VIDEORESIZE :
#if 0
			work = CalcWindow(calcWork, type, msg.moveFrame);
			Correct(XOff, YOff, work.w, work.h);
			curr = CalcWindow(calcBorder, type, work);
			SetWindowSize(win, curr);
			break;
#endif
		case SDL_QUIT :
			Ende();
			break;
#if 0
		case windFulled :
			curr = WindowSize(win, borderSize);
			full = WindowSize(win, fullSize);
			if (full.w == curr.w && full.h == curr.h)
				full = WindowSize(win, previousSize)
			work = CalcWindow(calcWork, type, full);
			Correct(XOff, YOff, work.w, work.h);
			curr = CalcWindow(calcBorder, type, work);
			SetWindowSize(win, curr);
			break;
		case windArrowed :
			ArrowWindow(msg.arrwMode);
			break;
		case windHSlid :
			XOff = (long)msg.horPos * (640 - work.w) / 1000;
			RedrawWindow(work);
			break;
		case windVSlid :
			YOff = (long)msg.vertPos * (400 - work.h) / 1000;
			RedrawWindow(work);
			break;
		case menuSelected :
			NormalTitle(MenuAdr, msg.selTitle, TRUE);
			SelectMenu(msg.selItem);
			break;
#endif
		}
		if (type != 0) SetSlider();
	}

	int SDL_WaitEventTimeout(SDL_Event *event, int timeout)
	{
		Uint32 expiration = 0;
		if (timeout > 0)
			expiration = SDL_GetTicks() + timeout;

		for (;;) {
			SDL_PumpEvents();
			switch (SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_ALLEVENTS)) {
			case -1:
				/* fixed from SDL 2 beta to make it
				 * distinguishable from timeout*/
				return -1;
			case 1:
				return 1;
			case 0:
				if (!timeout 
				 || (timeout > 0 
				  && SDL_GetTicks() >= expiration))
					return 0;
				SDL_Delay(10);
			}
		}
	}
	void MultiEvent(EventSet flags, unsigned mouse, unsigned buttons,
		unsigned clicks, unsigned mm1flags, SDL_Rect *rect1,
		unsigned mm2flags, SDL_Rect *rect2, 
		SDL_Event *msg, unsigned long time,
		Point *mLoc, unsigned *mButtons,
		SDLMod *keyState, SDL_keysym *key, unsigned *doneClicks,
		EventSet *events)
	{
		int buttonevent = (1<<SDL_MOUSEBUTTONUP)
			|(1<<SDL_MOUSEBUTTONDOWN);
		static struct {
			Point mLoc;
			unsigned mButtons;
			SDLMod keyState;
			SDL_keysym key;
		} Save;

		int Motion(unsigned flags,
			unsigned mm1flags, SDL_Rect *rect1,
			unsigned mm2flags, SDL_Rect *rect2,
			Point *oldmLoc,
			Point *newmLoc)
		{
			int Inside(Point *p, SDL_Rect *r)	
			{
				return p->x >= r->x
					&& p->y >= r->y
					&& p->x < r->x + r->w
					&& p->y < r->y + r->h;
			}

			return flags & (1<<SDL_MOUSEMOTION)
				&& ((mm1flags & (1<<SDL_MOUSEMOTION)
					&& Inside(oldmLoc, rect1)
					   != Inside(newmLoc, rect1))
				 || (mm2flags & (1<<SDL_MOUSEMOTION)
					&& Inside(oldmLoc, rect2)
					   != Inside(newmLoc, rect2)));
		}

		*events = 0;
		*doneClicks = ((Save.mButtons & mouse) == mouse >> 8);
		printf("MultiEvent() %d %d %d %d\n", *doneClicks, Save.mButtons, mouse, clicks);


		while (!(flags & (SDL_MOUSEBUTTONUP|SDL_MOUSEBUTTONDOWN))
		    || *doneClicks <= clicks)
		{
			printf("calling SDL_WaitEventTimeout %d %d %d %d\n",
				flags, timer, flags&timer, (int)time);
			switch (SDL_WaitEventTimeout(msg,
					flags & timer ? time : -1))
			{
				Point oldmLoc;
			case 1: /* SDL Event */
				switch (msg->type) {
				case SDL_KEYDOWN:
					printf("The %s key was pressed (code %i)!\n",
					       SDL_GetKeyName(msg->key.keysym.sym), msg->key.keysym.sym);		
					if (msg->key.keysym.sym == SDLK_RSHIFT
					 || msg->key.keysym.sym == SDLK_LSHIFT
					 || msg->key.keysym.sym == SDLK_RCTRL
					 || msg->key.keysym.sym == SDLK_LCTRL
					 || msg->key.keysym.sym == SDLK_RALT
					 || msg->key.keysym.sym == SDLK_LALT
					 || msg->key.keysym.sym == SDLK_RMETA
					 || msg->key.keysym.sym == SDLK_LMETA
					 || msg->key.keysym.sym == SDLK_LSUPER
					 || msg->key.keysym.sym == SDLK_RSUPER)
						continue;
					Save.keyState = msg->key.keysym.mod;
					Save.key = msg->key.keysym;
					if (~flags & (1<<SDL_KEYDOWN))
						continue;

					*events |= (1<<SDL_KEYDOWN);
					break;
				case SDL_MOUSEBUTTONUP:
					buttonevent = (1<<SDL_MOUSEBUTTONUP);
					Save.mButtons
						&= ~(1<<msg->button.button);
					goto CountClicks;
				case SDL_MOUSEBUTTONDOWN:
					printf("Mouse button %d pressed at (%d,%d)\n", msg->button.button, msg->button.x, msg->button.y);		
					buttonevent = (1<<SDL_MOUSEBUTTONUP);
					Save.mButtons
						|= (1<<msg->button.button);
				CountClicks:
					*doneClicks += (Save.mButtons & mouse)
								== mouse >> 8;

					Save.mLoc.x = msg->button.x;
					Save.mLoc.y = msg->button.y;

					goto TestMotion;
				case SDL_MOUSEMOTION:
					Save.mLoc.x = msg->motion.x;
					Save.mLoc.y = msg->motion.y;
				TestMotion:
					oldmLoc = Save.mLoc;
					if (!Motion(flags,
							mm1flags, rect1,
							mm2flags, rect2,
							&oldmLoc, &Save.mLoc))
						continue;

					*events |= (1<<SDL_MOUSEMOTION);
					break;
				case SDL_VIDEORESIZE:
				case SDL_VIDEOEXPOSE:
				case SDL_QUIT:
				case SDL_SYSWMEVENT:
				case SDL_ACTIVEEVENT:
					if (~flags & (1<<msg->type))
						continue;

					*events |= (1<<msg->type);
					break;
				default:
					printf("uncatched event type %d\n", 
						msg->type);
					continue;
				}
				break;
			case -1: /* Error */
				printf("Fehler beim Warten auf Events!\n");
				continue;
			case 0: /* Timeout */
				printf("Timeout\n");
				if (~flags & (1<<SDL_USEREVENT))
					continue;

				*events |= (1<<SDL_USEREVENT);
			}
			break;
		}
		*mLoc = Save.mLoc;
		*mButtons = Save.mButtons;
		*keyState = Save.keyState;
		*key = Save.key;

		if (*doneClicks > 0)
			(*doneClicks)--;
		if (*doneClicks >= clicks)
			*events |= buttonevent & flags;

		return;
	}
	printf("WaitInput()");

	ok = FALSE; b = 0; ch = '\0';
	if (NewXMin < 40) { /* Teilbereich aktualisieren */
		rect.x = (int)NewXMin * 16 - XOff + work.x;
		rect.y = (int)NewYMin * 16 - YOff + work.y;
		rect.w = ((int)(NewXMax - NewXMin) + 1) * 16;
		rect.h = ((int)(NewYMax - NewYMin) + 1) * 16;
		RedrawWindow(rect);
		NewXMin = 40; NewYMin = 25; NewXMax = 0; NewYMax = 0;
	}
	mouse = 1; /* zuerst auf loslassen warten */
	if (WarteZeit < 0) {
		/*GrafMouse(arrow, NIL);*/
		time = mousetime; /* Wartezeit bis losgelassen */
		flags = keyboard |mouseButton | message | timer;
	} else {
		time = ABS(WarteZeit);
		flags = keyboard | message | timer;
	}
	do {
		MultiEvent(flags, mouse, msBut1|msBut2, 0,
			lookForEntry, &rect, lookForEntry, &rect,
			&msg, time,
			&mLoc, &mButtons,
			&keyState, &key, &doneClicks,
			&events);
		if (message & events) Message();
		if (mouseButton & events) {
			if (mouse == msBut1) { /* Maus losgelassen */
				losgelassen = TRUE;
				mouse = msBut1 | (msBut1 << 8); /* auf Klick warten */
				mousetime = 1;
			} else { /* Maus gedrückt */
				Button();
				if (losgelassen)
					mousetime = 1000;
				else
					mousetime = 1;
				losgelassen = FALSE;
			}
		}
		if (keyboard & events) Keyboard();
		if (timer & events) {
			mouse = msBut1 | (msBut1 << 8); /* auf gedrückte Maus warten */
			ok = WarteZeit >= 0;
			flags = keyboard | mouseButton | message;
		}
	} while (!ok);
	/*GrafMouse(bee, NIL);*/

#undef xx
#undef yy
#undef ch
#undef b
}


/**
 * Wartet auf einen Tastendruck. Zeichnet auch den Fensterinhalt neu.
 */
void WaitKey(void)
{
	unsigned x, y; BITSET s; char ch;
	WaitInput(&x, &y, &s, &ch, -1);
#if 0
	RedrawWindow(NULL);

	SDL_Event event;
	while (event.type != SDL_KEYDOWN) {
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT) {
			ExitWorkstation(0);
			exit(0);
		}
	}       
#endif
}

/**
 * Wartet t Einheiten und zeichnet vorher den Fensterinhalt neu. Die
 * (von Alexander gedachten) Einheiten von t sind mit unbekannt! Ich
 * übergebe das mal einfach so an usleep.
 */
void WaitTime(unsigned t)
{
	unsigned mx, my; BITSET mb; char mch;
	WaitInput(&mx, &my, &mb, &mch, t); /* Redraw! */
#if 0
	RedrawWindow(NULL);
	if (t > 0)
		usleep(t);
#endif
}


unsigned long GetTime()
{
	return time(NULL);
}

unsigned Zufall(unsigned n)
{
	if (n == 0) return 0;
	return rand();
}

void SetzeZufall(unsigned long n)
{
	srand(n);
}

/**
 * Behandelt fatale und nichtfatale Fehler. Gibt eine Meldung aus und
 * lässt, je nach Mode, den User eine Entscheidung über Fortführung,
 * Abbruch oder Programmabbruch treffen. So ist es zumindest
 * gedacht. Momentan sind keine Userentscheidungen möglich, bei
 * schwerwiegenden Fehlern wird immer das Programm beendet. HASCS
 * denkt, dass der User immer auf den ersten Button geklickt hat.
 *
 * Mode == -1 bedeutet einen fatalen Fehler (alles Ende)
 * Mode == 0 bedeutet schwerwiegender Fehler, aber die Frage nach dem 
 *           Programmabbruch ist dem User überlassen. Momentan wird aber
 *           immer abgebrochen (außer ShowError ist FALSE).
 * Mode == 1 gibt nur eine Meldung aus und das Programm geht weiter
 * Mode == 2 fragt den User nach Abbruch oder Weiter. Momentan "klickt"
 *           er immer auf Abbruch.
 * Mode == 3 gibt nur eine Meldung aus und das Programm geht weiter
 * 
 */
void Error(char *s, int Mode)
{
	if (Mode >= 0 && !ShowError)
		return; // keine Fehlermeldung, da ShowError == FALSE
	
	printf("HASCS-Fehlermeldung: %s\n", s);
	printf("Buttons: ");
	switch (Mode) {
	case -1 : printf("[ ENDE ]\n"); break;
	case  0 : printf("[ ENDE | WEITER ]\n"); break;
	case  1 : printf("[ WEITER ]\n"); break;
	case  2 : printf("[ ABBRUCH | WEITER ]\n"); break;
	case -3 : printf("[ ABBRUCH ]\n"); break;
	}

	// User-Abfrage im Alert-Fenster:
	/*
	  default = 1;
	  switch (Mode) {
	  case -1 : Concat(q, "][ ENDE ]", q, ok); break;
	  case 0 : Concat(q, "][ ENDE | WEITER ]", q, ok); default = 2; break;
	  case 1 : Concat(q, "][ WEITER ]", q, ok); break;
	  case 2 : Concat(q, "][ ABBRUCH | WEITER ]", q, ok); default = 2; break;
	  case 3 : Concat(q, "][ ABBRUCH ]", q, ok); break;
	  }
	  FormAlert(default, q, ErrorResult);
	*/

	ErrorResult = 1; // Der User "hat den ersten Knopf im Alert-Fenster gedrückt"

	if (ErrorResult == 1 && Mode <= 0)
		ExitWorkstation(1);
}


void PrinterOut(char ch)
{
	printf("Ich drucke aus: %c\n", ch);
}

int PrinterStatus()
{
	return 0;
}

