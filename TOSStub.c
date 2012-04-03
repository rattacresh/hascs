#include "HASCSSystem.h" /* HASCSSystem module */

/* compat stubs begin */
#define bee 0
#define NULL 0
#define FALSE 0
#define NIL NULL
#define TRUE 1
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

DeviceHandle ScreenHandle;
int ScreenWidth, ScreenHeight, ScreenPlanes;

char WName[60];
WElementSet type;
unsigned win;
		
Rectangle desk, work, curr, full, save;
int XOff, YOff;
		
void *MenuAdr;

MemFormDef ScreenMFDB, BufferMFDB, PicMFDB;
PtrMemFormDef ScreenMFDBAdr, BufferMFDBAdr, PicMFDBAdr;

SearchRec DTABuffer;
char LastFileName[60];

unsigned char *BufferAdr;
unsigned long BufferLen;

unsigned long mousetime;
int losgelassen;

/* Min max */

int Max(int a, int b)
{
	if (a > b) { return a; } else { return b; }
}

int Min(int a, int b)
{
	if (a < b) { return a; } else { return b; }
}

int RcIntersect(Rectangle *ref_p1, Rectangle *ref_p2)
#define p1 (*ref_p1)
#define p2 (*ref_p2)
{
	Rectangle r;

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
	int ok;
	PtrDevParm paramptr;

	InitGem(2, ScreenHandle, ok); /* Rasterkoordinaten */
	if (!ok)
		Error("Applikation kann nicht angemeldet werden!", -1);
	ShellRead(Name, Command);
	GetDefaultPath(ActPath);
	GrafMouse(bee, NIL);

	paramptr = DeviceParameter(ScreenHandle);
	ScreenWidth = paramptr->rasterWidth + 1;
	ScreenHeight = paramptr->rasterHeight + 1;
	ScreenPlanes = paramptr->maxRasterPls;

	LoadResource("HASCSIII.RSC"); /* Men�zeile laden */
	if (GemError()) {
		Error("Resourcedatei HASCSIII.RSC nicht gefunden!", 1);
		MenuAdr = NIL;
	} else {
		MenuAdr = ResourceAddr(treeRsrc, 0);
		MenuBar(MenuAdr, TRUE);
	}

	XOff = 0;
	YOff = 0;

	desk = WindowSize(DeskHandle, workSize); /* Desktopgr��e */
	type = nameBar|closer|fuller|mover|sizer|upArrow
		|downArrow|vertSlider|leftArrow|rightArrow|horSlider;
	CreateWindow(type, desk, win);
	Assign(WinName, WName, ok);
	SetWindowString(win, nameStr, WName);
	work.x = 0; work.y = 0; work.w = 640; work.h = 400;
	curr = CalcWindow(calcBorder, type, work);
	curr.x = desk.x; curr.y = desk.y;
	if (curr.w > desk.w) curr.w = desk.w;
	if (curr.h > desk.h) curr.h = desk.h;
	OpenWindow(win, curr);
	work = WindowSize(win, workSize);

	ScreenMFDB.start = NULL; /* Bildschirm */
	ScreenMFDBAdr = &ScreenMFDB;

	SetDTA(&DTABuffer);
}


void ExitWorkstation(int result)
{
	GemHandle AppHandle;
	AppHandle = CurrGemHandle();
	ExitGem(AppHandle);
	TermProcess(result);
}


void *Allocate(unsigned long Bytes)
{
	void *Ptr;
	if (Bytes == 0) { return NULL; }
	Alloc(Bytes, Ptr);
	return Ptr;
}

void *GetBuffer(unsigned long Bytes)
{

/*
InOut.WriteLn();
InOut.WriteString("GetBuffer: Bytes = ");
InOut.WriteLNum(Bytes, 10, 1, ' ');
*/

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
	BufferAdr[Bytes] = 0; /* Endmarkierung */

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
	if (n == 0) { /* alles l�schen */
		for (i = 1; i <= AnzCache; i++)
			if (!Free(Cache[i].CacheBuffer))
				Error("Fehler in der Speicherverwaltung(1)!", 0);
		AnzCache = 0;
	} else {
		if (!(Free(Cache[n].CacheBuffer)))
			Error("Fehler in der Speicherverwaltung(2)!", 0);
		for (i = n; i <= AnzCache - 1; i++)
			Cache[i] = Cache[i+1];
		AnzCache--;
	}
}

unsigned NewCache(unsigned id, unsigned long Bytes)
{
	unsigned i;
	void *adr;

	unsigned LRUCache(void)
	{
		unsigned i, j, Min; /*int ok;*/
		Min = UINT_MAX; j = 0;
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
	return AnzCache;
}

void Deallocate(void *Ptr)
{
	if (Ptr)
		if (Free(Ptr))
			Ptr = NULL;
}


int LoadAndRun(char *Prg, char *Arg)
{

	long result;
	char path[128], file[128];
	int i, l;
	int ok;
	Rectangle save;

	if (StrEqual(Prg, "EDITOR.PRG") || StrEqual(Prg, "HASCSSPR.PRG")
	 || StrEqual(Prg, "HASCSIII.PRG"))
	{
		SplitPath(Name, path, file);
		Concat(path, Prg, file, ok);
	} else
		Assign(Prg, file, ok);

	/* Arg umrechnen in PASCAL String */
	i = 0; while (Arg[i] != 0) i++; l = i;
	while (i > 0) {
		Arg[i] = Arg[i-1]; i--;
	}
	Arg[0] = l;

	if (type != 0 && MenuAdr != NIL)
		MenuBar(MenuAdr, FALSE);
	save = WindowSize(win, borderSize);
	CloseWindow(win);

	Pexec(loadExecute, file, Arg, NULL, result);
	if (result < 0) {
		Concat("Programmstart nicht m�glich: ",file, file, ok);
		Error(file, 1);
	}

	OpenWindow(win, save);
	if (type != 0 && MenuAdr != NIL)
		MenuBar(MenuAdr, TRUE);
	return result;
}

/* Bildschirmverwaltung *********************************************/

void Copy(int direction, int sx, int sy, int width, int height, int dx, int dy)
{
	Rectangle sourceRect, destRect;

	sourceRect.x = sx; sourceRect.y = sy;
	sourceRect.w = width; sourceRect.h = height;
	destRect.x = dx; destRect.y = dy;
	destRect.w = width; destRect.h = height;
	if (direction == 4) /* Pic -> Buffer */
		CopyOpaque(ScreenHandle, PicMFDBAdr, BufferMFDBAdr,
			  sourceRect, destRect, onlyS);
	else /* Buffer -> Buffer */
		CopyOpaque(ScreenHandle, BufferMFDBAdr, BufferMFDBAdr,
			  sourceRect, destRect, onlyS);
}

void SetPicture(unsigned width, unsigned height, void *Picture)
{
	PicMFDB.start = Picture;
	PicMFDB.w = width;
	PicMFDB.h = height;
	PicMFDB.words = (PicMFDB.w + 15) / 16;
	PicMFDB.standardForm = FALSE;
	PicMFDB.planes = 1;
	PicMFDBAdr = &PicMFDB;
}

void SetBuffer(unsigned width, unsigned height, void *Buffer)
{
	BufferMFDB.start = Buffer;
	BufferMFDB.w = width;
	BufferMFDB.h = height;
	BufferMFDB.words = (BufferMFDB.w+15) / 16;
	BufferMFDB.standardForm = FALSE;
	BufferMFDB.planes = 1;
	BufferMFDBAdr = &BufferMFDB;
}


/* Speicherverwaltung ***********************************************/

int FileName(char *Pattern, char *FileName)
{
	int result;
	int ok;
	if (StrEqual(LastFileName, Pattern))
		SearchNext(result);
	else {
		SearchFirst(Pattern, 0, result);
		Assign(Pattern, LastFileName, ok);
	}
	Assign(DTABuffer.name, FileName, ok);
	return result >= 0;
}

unsigned long FileLength(char *Filename)
{
	int result;
	SearchFirst(Filename, 0, result);
	FileError = result < 0;
	if (FileError) /* nicht gefunden */
		return 0;
	else
		return DTABuffer.size;
}

int OpenFile(char *Name)
{
	int Handle;
	Open(Name, 0, Handle);
	FileError = Handle < 0;
	return Handle;
}

void CloseFile(int Handle)
{
	FileError = !Close(Handle);
}

void DeleteFile(char *Name)
{
	FileError = Delete(Name);
}

int CreateFile(char *Name)
{
	int Handle;
	char path[128], file[128];
	Create(Name, 0, Handle);
	if (Handle == -34) { /* Path not found */
		SplitPath(Name, path, file);
		path[(sizeof path)-1] = '\0';
		if (!DirCreate(path));
		Create(Name, 0, Handle);
	}
	FileError = Handle < 0;
	return Handle;
}

void ReadFile(int Handle, unsigned long Bytes, void *Ptr)
{
	unsigned long Count;
	Count = Bytes;
	Read(Handle, Bytes, Ptr);
	FileError = Bytes != Count;
}

void WriteFile(int Handle, unsigned long Bytes, void *Ptr)
{
	unsigned long Count;
	Count = Bytes;
	Write(Handle, Bytes, Ptr);
	FileError = Bytes != Count;
}

void FileSeek(int Handle, unsigned long pos)
{
	long ret;
	Seek(pos, Handle, beginning, ret);
	FileError = ret != pos;
}

void RenameFile(char *s, char *d)
{
	Rename(s, d);
}

int SelectFile(char *msg, char *path, char *file)
{
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
	return TRUE;
}


/* Eingaberoutinen **************************************************/

BITSET WaitInput(unsigned x, unsigned y, BITSET *b, char *ch, int WarteZeit)
{
	EventSet flags, events;
	unsigned mouse;
	MessageBuffer msg;
	Rectangle rect;
	Point mLoc;
	MButtonSet mButtons;
	SpecialKeySet keyState;
	GemChar key;
	unsigned doneClicks;
	int ok;
	unsigned long time;


	void RedrawWindow(Rectangle frame)
	{
		Rectangle r, s;
		UpdateWindow(TRUE);
		r = WindowRectList(win, firstElem);
		while (r.w > 0 && r.h > 0) {
			if (RcIntersect(frame, r)) {
				GrafMouse(mouseOff, NIL);
				/* Pufferkoordinaten */
				s.x = r.x - work.x + XOff;
				s.y = r.y - work.y + YOff;
				s.w = r.w; s.h = r.h;
				if (ScreenPlanes == 1)
					CopyOpaque(ScreenHandle, BufferMFDBAdr, ScreenMFDBAdr,
						s, r, onlyS);
				else
					CopyTrans(ScreenHandle, BufferMFDBAdr, ScreenMFDBAdr,
						s, r, replaceWrt, 1, 0);
				GrafMouse(mouseOn, NIL);
			}
			r = WindowRectList(win, nextElem);
		}
		UpdateWindow(FALSE);
	}

	void SetSlider(void)
	{
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
	}

	void VollBild(void)
	{
		if (type == 0) { /* Fenster wieder normal */
			CloseWindow(win); DeleteWindow(win);
			type = nameBar | closer | fuller | mover | sizer | upArrow
				| downArrow | vertSlider | leftArrow | rightArrow
				| horSlider;
			CreateWindow(type, desk, win);
			SetWindowString(win, nameStr, &WName);
			OpenWindow(win, save);
			if (MenuAdr) MenuBar(MenuAdr, TRUE);
		} else {
			save = WindowSize(win, borderSize);
			CloseWindow(win); DeleteWindow(win);
			if (MenuAdrL) MenuBar(MenuAdr, FALSE);
			type = 0;
			CreateWindow(type, desk, win);
			work.x = Max(ScreenWidth / 2 - 320, 0);
			work.y = Max(ScreenHeight / 2 - 200, 0);
			work.w = Min(640, ScreenWidth);
			work.h = Min(400, ScreenHeight);
			curr = CalcWindow(calcBorder, type, work);
			OpenWindow(win, curr);
			XOff = 0; YOff = 0;
		}
		work = WindowSize(win, workSize);
		RedrawWindow(work);
	}

	void Ende(void)
	{
		int dummy;
		Error("HASCS III wirklich beenden?", 0);
	}

	void Correct(int x, int y, int w, int h)
	{
		w = Min(w, 640);
		h = Min(h, 400);
		x = Min(x, 640-w); x = Max(0, x); /* XOff */
		y = Min(y, 400-h); y = Max(0, y); /* YOff */
	}

	void Button(void);
	{
		ok = mLoc.x >= work.x && mLoc.y >= work.y
			&& mLoc.x < work.x + work.w && mLoc.y < work.y + work.h;
		if (ok) {
			x = mLoc.x - work.x + XOff;
			y = mLoc.y - work.y + YOff;
			if (msBut1 & mButtons) b |= (1<<0);
			if (msBut2 & mButtons) b |= (1<<1);
		}
	}

	void Keyboard(void)
	{
		int Redraw;
		Redraw = FALSE;
		switch (key.scan) {
		case 0x4B : XOff += 16; Redraw = TRUE; break;
		case 0x4D : XOff += 16; Redraw = TRUE; break;
		case 0x48 : YOff += 16; Redraw = TRUE; break;
		case 0x50 : YOff += 16; Redraw = TRUE; break;
		case 0x10 : if (key.ascii == 0x11) { /* Control Q */
				Ende();  key.ascii = 0C;
			}
			break;
		case 0x21 : if (key.ascii == 0x06) { /* Control F */
				VollBild(); key.ascii = 0C;
			}
			break;
		case 0x26 : if (key.ascii == 0x0C) { /* Control L */
				FreeCache(0); key.ascii = 0C;
			}
			break;
		}
		if (Redraw) {
			Correct(XOff, YOff, work.w, work.h);
			RedrawWindow(work);
			SetSlider();
		}
		ch = key.ascii;
		if (ch >= '1' && ch <= '9' && timer & flags)
			ch = '\0'; /* Richtungstasten l�schen */
		ok = ch != '\0';
	}


	void Message(void)
	{

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

		switch (msg.msgType) {
		case windRedraw :
			RedrawWindow(msg.rdrwFrame);
			break;
		case windTopped :
			SetTopWindow(win);
			break;
		case windMoved, windSized :
			work = CalcWindow(calcWork, type, msg.moveFrame);
			Correct(XOff, YOff, work.w, work.h);
			curr = CalcWindow(calcBorder, type, work);
			SetWindowSize(win, curr);
			break;
		case windClosed :
			Ende();
			break;
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
		}
		if (type != 0) SetSlider();
	}

	ok = FALSE; b = 0; ch = '\0';
	if (NewXMin < 40) { /* Teilbereich aktualisieren */
		rect.x = INT(NewXMin) * 16 - XOff + work.x;
		rect.y = INT(NewYMin) * 16 - YOff + work.y;
		rect.w = (INT(NewXMax - NewXMin) + 1) * 16;
		rect.h = (INT(NewYMax - NewYMin) + 1) * 16;
		RedrawWindow(rect);
		NewXMin = 40; NewYMin = 25; NewXMax = 0; NewYMax = 0;
	}
	mouse = 1; /* zuerst auf loslassen warten */
	if (WarteZeit < 0) {
		GrafMouse(arrow, NIL);
		time = mousetime; /* Wartezeit bis losgelassen */
		flags = EventSet{keyboard, mouseButton, message, timer};
	} else {
		time = LONG(ABS(WarteZeit));
		flags = EventSet{keyboard, message, timer};
	}
	
	do {
		MultiEvent(flags, mouse, msBut1|msBut2, 0,
			lookForEntry, rect, lookForEntry, rect,
			msg, time,
			mLoc, mButtons,
			keyState, key, doneClicks,
			events);
		if (message & events) Message();
		if (mouseButton & events) {
			if (mouse == 1) { /* Maus losgelassen */
				losgelassen = TRUE;
				mouse = 257; /* auf Klick warten */
				mousetime = 1;
			} else { /* Maus gedr�ckt */
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
			mouse = 257; /* auf gedr�ckte Maus warten */
			ok = WarteZeit >= 0;
			flags = keyboard | mouseButton | message;
		}
	} while (!ok);
	GrafMouse(bee, NIL);
}


void WaitKey(void)
{
	unsigned x, y; BITSET s; char ch;
	WaitInput(x, y, s, ch, -1);
}


void WaitTime(unsigned t)
{
	unsigned mx, my; BITSET mb; char mch;
	WaitInput(mx, my, mb, mch, t); /* Redraw! */
}


unsigned long GetTime()
{
	return SuperLPeek(0x04BA);
}

unsigned Zufall(unsigned n)
{
	if (n == 0) return 0;
	return RandomCard(1, n);
}

void SetzeZufall(unsigned long n)
{
	Randomize(n);
}


void Error(char *s, int Mode)
{
	char q[256];
	unsigned default;
	int ok;
	
	if (Mode >= 0 && !ShowError)
		 return; /* keine Fehlermeldung */
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
}


void PrinterOut(char ch)
{
	do { } while (!PrnOS());
	PrnOut(ch);
}

int PrinterStatus()
{
	return PrnOS();
}

void SystemInit(void)
{
	ShowError = TRUE;
	FileError = FALSE;
	LastFileName = "";
	NewXMin = 40; NewYMin = 25; NewXMax = 0; NewYMax = 0;

	BufferLen = 0;
	BufferAdr = NULL;

	AnzCache = 0; CacheCounter = 0;

	losgelassen = TRUE;
	mousetime = 1;
}