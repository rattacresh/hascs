/* Sound module */
#include "compat.h"
extern void XConcat(char *s, char *p, char *r, int *ok);
#define Concat(x, y, z, a) XConcat(x, y, z, &a)
#include <arpa/inet.h> /* byte order htons() ntohs()*/
#include <SDL/SDL.h>
#include "Sound.h"

#include "HASCSSystem.h"

/* Hardware Register für DMA Sound */
#define sndmactl (*(volatile unsigned char *)0xFF8901) /* Sound-DMA-Control Register */
#define sndbashi (*(volatile unsigned char *)0xFF8903) /* Frame-Start-Address  */
#define sndbasmi (*(volatile unsigned char *)0xFF8905)
#define sndbaslo (*(volatile unsigned char *)0xFF8907)
#define sndadrhi (*(volatile unsigned char *)0xFF8909) /* Frame-Address-Counter */
#define sndadrmi (*(volatile unsigned char *)0xFF890B)
#define sndadrlo (*(volatile unsigned char *)0xFF890D)
#define sndendhi (*(volatile unsigned char *)0xFF890F) /* Frame-End-Adress */
#define sndendmi (*(volatile unsigned char *)0xFF8911)
#define sndendlo (*(volatile unsigned char *)0xFF8913)
#define sndmode  (*(volatile unsigned char *)0xFF8921) /* Sound-Mode-Control */

#define conterm (*(volatile unsigned *)0x484)

/*static SoundType StaticSound;*/
/*char FileName[81];*/
static uint16_t *IBuffer;
static int DoLoop;
/*static ModeBuf stack;*/

static SDL_AudioSpec desired;

static int DMASound()
{
	unsigned long Value;
	if (GetCookie("_SND", Value))
		return Value / 2 % 2 == 1;
	return FALSE;
}


static int InBuffer(unsigned l, char *p, char *s)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (s[i] == '\0') return TRUE;
		if (p[l] != s[i]) return FALSE;
		l++;
	}
	return TRUE;
}

static void ChangeVorz(CharPtr Buffer, unsigned long Length)
{
	unsigned long i;
	for (i = 1; i <= Length; i++) {
		*Buffer = (*Buffer+128) % 256;
		Buffer++;
	}
}

static unsigned GetFreq(unsigned f)
{
	if (f < 9388) return 0;
	else if (f < 18775) return 1;
	else if (f < 37550) return 2;
	else return 3;
}

int LoadSoundFile(char *f, unsigned id, SoundType *ref_s)
{
#define s (*ref_s)
	int h;
	char header[128];
	int ok/*, free*/;

	int LoadSMP(void)
	{
		unsigned long HeaderLength;
		unsigned i;

		HeaderLength = (long)ntohs(IBuffer[2])+ntohs(IBuffer[3]);
		s.Length = 65536*(long)ntohs(IBuffer[4])+ntohs(IBuffer[5]);
		i = /*HASCSSystem.*/NewCache(id, s.Length);
		s.Buffer = /*HASCSSystem.*/Cache[i].CacheBuffer;
		/*HASCSSystem.*/FileSeek(h, HeaderLength + 8);
		/*HASCSSystem.*/ReadFile(h, s.Length, /*HASCSSystem.*/Cache[i].CacheBuffer);
		s.Frequency = 12500; /* 12500 Hz */
		/*HASCSSystem.*/Cache[i].CacheInfo1 = s.Frequency;
		return TRUE;
	}

	int LoadWAV(void)
	{
		unsigned i;
		s.Length = (long)(256*header[41]+header[40])
			 + 65536*(long)(256*header[43]+header[42]);
		s.Frequency = header[24] + 256*header[25];
		i = /*HASCSSystem.*/NewCache(id, s.Length);
		s.Buffer = /*HASCSSystem.*/Cache[i].CacheBuffer;
		/*HASCSSystem.*/Cache[i].CacheInfo1 = s.Frequency;
		/*HASCSSystem.*/FileSeek(h, 44);
		/*HASCSSystem.*/ReadFile(h, s.Length, s.Buffer);
		ChangeVorz(s.Buffer, s.Length);
		return TRUE;
	}

	int LoadHSN(unsigned Version)
	{
		unsigned i;
		s.Length = 65536*(long)ntohs(IBuffer[10])+ntohs(IBuffer[11]);
		s.Frequency = ntohs(IBuffer[12]) * 10;
		i = /*HASCSSystem.*/NewCache(id, s.Length);
		s.Buffer = /*HASCSSystem.*/Cache[i].CacheBuffer;
		/*HASCSSystem.*/Cache[i].CacheInfo1 = s.Frequency;
		if (Version == 0)
			/*HASCSSystem.*/FileSeek(h, 49);
		else
			/*HASCSSystem.*/FileSeek(h, 90);
		/*HASCSSystem.*/ReadFile(h, s.Length, s.Buffer);
		return TRUE;
	}

	int LoadUnknown(void)
	{
		unsigned i;
		s.Length = /*HASCSSystem.*/FileLength(f);
		i = /*HASCSSystem.*/NewCache(id, s.Length);
		s.Buffer = /*HASCSSystem.*/Cache[i].CacheBuffer;
		/*HASCSSystem.*/ReadFile(h, s.Length, s.Buffer);
		s.Frequency = 12500; /* 12500 Hz */
		/*HASCSSystem.*/Cache[i].CacheInfo1 = s.Frequency;
		return TRUE;
	}

	h = /*HASCSSystem.*/OpenFile(f);
	if (/*HASCSSystem.*/FileError) return FALSE; /* File Not Found */
	/*HASCSSystem.*/ReadFile(h, 128, &header);
	/*HASCSSystem.*/FileSeek(h, 0);
	IBuffer = (uint16_t *)&header;
	if (InBuffer(0, header, "\x7e\x81\x7e\x81")) ok = LoadSMP();
	else if (InBuffer(8, header, "WAVEfmt")) ok = LoadWAV();
	else if (InBuffer(0, header, "HSND1.0")) ok = LoadHSN(0);
	else if (InBuffer(0, header, "HSND1.1")) ok = LoadHSN(1);
	else ok = LoadUnknown();
	/*HASCSSystem.*/CloseFile(h);
	return ok;
#undef s
}

int LoadSound(unsigned n, SoundType *ref_s)
{
#define s (*ref_s)
	char FileName[128];
	int ok;
	unsigned i, id;

	id = 2048 + n;
	i = /*HASCSSystem.*/GetCache(id);
	if (i != 0) {
		s.Buffer = /*HASCSSystem.*/Cache[i].CacheBuffer;
		s.Length = /*HASCSSystem.*/Cache[i].CacheLength;
		s.Frequency = /*HASCSSystem.*/Cache[i].CacheInfo1;
		return TRUE;
	} else { /* neu laden */
		strcpy(FileName, "SAMPLE.000");
		FileName[7] = n / 100 + '0';
		FileName[8] = n / 10 % 10 + '0';
		FileName[9] = n % 10 + '0';
		Concat(SoundPath, FileName, FileName, ok);
		return LoadSoundFile(FileName, id, &s);
	}
#undef s
}

static struct {
	unsigned char *start, *b, *ende;
	int loop, mode;
} SoundCB;

void SoundCallback(void *Buffer, Uint8 *stream, int len)
{
	int i;
	if (SoundCB.b == SoundCB.ende) {
		SDL_PauseAudio(1);
		return;
	}
	for (i = 0; i < len; i++) {
		switch (SoundCB.mode) {
		case 0:
			stream[i++] = *SoundCB.b;
			stream[i++] = *SoundCB.b;
			/* FALLTHROUGH */
		case 1:
			stream[i++] = *SoundCB.b;
			/* FALLTHROUGH */
		case 2:
			stream[i] = *SoundCB.b++;
			break;
		case 3:
			stream[i] = *SoundCB.b++;
			if (SoundCB.b == SoundCB.ende)
				break;
			SoundCB.b++;
		}
		if (SoundCB.b == SoundCB.ende) {
			if (!SoundCB.loop)
				break;
			else
				SoundCB.b = SoundCB.start;
		}
	}
	for (; i < len; i++)
		stream[i] = 0;
	ChangeVorz(stream, len);
}

static void PlaySoundSDL(SoundType *ref_s)
{
#define s (*ref_s)
	if (!SoundAusgabe) return;

#if 0
	printf("PlaySoundSDL(Start = %p, Length = %ld, "
		"ende = %p, Freq = %d, Loop = %d)\n", 
		s.Buffer, s.Length,
		s.Buffer + s.Length, s.Frequency, DoLoop);
#endif
	SDL_LockAudio();
	SoundCB.loop = DoLoop;
	SoundCB.mode = GetFreq(s.Frequency);
	SoundCB.start = SoundCB.b = s.Buffer;
	SoundCB.ende = s.Buffer + s.Length;
	SDL_PauseAudio(0);
	SDL_UnlockAudio();
#undef s
}

static void PlaySoundDMA(SoundType *ref_s)
{
#define s (*ref_s)
#if 0
	union {
		unsigned long ptr;
		char e[4];
	} ende;
#endif

	if (!SoundAusgabe) return;

	EnterSupervisorMode(stack); /* Supervisormode */
#if 0
	sndmactl = 0; /* Stop */
#endif
	if (s.Buffer != /*HASCSSystem.*/NULL) {
#if 0
		ende.ptr = (unsigned long)s.Buffer + s.Length;
		sndbashi = s.b[2];
		sndbasmi = s.b[1];
		sndbaslo = s.b[0];
		sndendhi = ende.e[2];
		sndendmi = ende.e[1];
		sndendlo = ende.e[0];
		sndmode  = 128 + GetFreq(s.Frequency); /* Mono */
		if (DoLoop)
			sndmactl = 3; /* Let's go loop... */
		else
			sndmactl = 1; /* Let's go... */
#endif
	}
	LeaveSupervisorMode(stack);
#undef s
}


static void PlaySoundInterrupt(SoundType *ref_s)
{
#define s (*ref_s)
#if 0
	unsigned long FrameStart, FrameEnd;
	unsigned f;
	if (!SoundAusgabe) return;

	FrameStart = (unsigned long)s.Buffer;
	FrameEnd = (unsigned long)s.Buffer + s.Length;
	f = s.Frequency;
	asm { /*$C- Case insensitive */
		clr.l d0
		lea volt(pc),a1       ; Adresse der Pegelwerte
		bclr  d0,(a1)         ; Flag Tabelle initialisieren
		beq start             ; ok
		subq.b  #1,d0         ; Länge der Tabelle(-1) == 255
		lea tabelle(pc),a0    ; Adresse der Soundbytes
	loope:
		move.b  #$8,(a0)    ; Umwandeln
		move.b  #$9,4(a0)   ; der
		move.b  #$A,8(a0)   ; Pegelwerte
		move  (a1)+,d1
		move  d1,d2
		and #15,d2
		move.b  d2,10(a0)   ; in auszugebende
		lsr #4,d1
		move  d1,d2
		and #15,d2
		move.b  d2,6(a0)    ; Lautstärken
		lsr #4,d1
		move.b  d1,2(a0)    ; für den Soundchip
		lea 16(a0),a0       ; immer 16 Bytes
		dbra  d0,loope       ; loopen

	start:
		lea stack, a0
		move.l a0, (a3)+
		jsr EnterSupervisorMode

		clr.b $FFFFFA19                ; Timer A Stop
		lea sam_data(pc), a1
		move.l  FrameStart(a6), (a1)+  ; Frame Address Counter
		move.l  FrameEnd(a6),(a1)+     ; Frame End Address
		move.l  FrameStart(a6),(a1)+   ; Frame Base Address
		move.b  #64,$FFFFFA17          ; AEOI
		move.l  #$07007F00,$FFFF8800   ; Soundchip-Init
		lea stack, a0
		move.l a0, (a3)+
		jsr LeaveSupervisorMode

		;Test ob nur aus
		cmpi.l #0, FrameStart(a6);
		beq.w bye

		lea play_s(pc),a0         ; Interrupt Player
		move.l  #1228800,d2       ; Frequenz
		divu.w  f(a6),d2          ; in
		addq.w  #1,d2             ; Teiler
		lsr.w #1,d2               ; umrechnen (runden)
		move.w #0, (a3)+          ; Timer A
		move.w #1, (a3)+          ; Control
		move.w d2, (a3)+          ; data
		move.l a0, (a3)+          ; vec
		jsr SetTimerInterrupt
		bra.w bye

	; Interrupt Player
		dc.l  'XBRA', 'SMPL', imret
	play_s:
		movem.l d0/a0-a2,-(a7)    ; Register retten
		lea sam_data(pc),a0       ; Adresse der Sampledaten
		move.l  (a0)+,a1          ; Aktuelle Sampleadresse
		move.l (a0), a2
		cmpa.l a1, a2             ; Endadresse erreicht ?
		beq test                  ; ja --> Hold-Test
		move.w  #128,d0           ; offset
		add.b (a1)+,d0            ; Datenbyte plus offset
		move.l  a1,-(a0)          ; Neue Sampleadresse
		lsl.w #4,d0
		lea 12(a0,d0.w),a0        ; Soundchip Daten aus Tabelle
		move.l  (a0)+,$FFFF8800 ; beep
		move.l  (a0)+,$FFFF8800 ; beep
		move.l  (a0)+,$FFFF8800 ; beep
	ende:
		movem.l (a7)+,d0/a0-a2    ; Register holen
	imret:
		rte       ; und zurück

	test:
		tst.w DoLoop
		beq aus                 ; kein Hold --> AUS
		move.l  4(a0),-4(a0)    ; Neuer Samplestart
		bra ende
	aus:
		clr.b $FFFFFA19       ; Timer A Stop
		bra ende

	volt:
		dc.w  $0100, $0200, $0210, $0310, $0410, $0510, $0600, $0610
		dc.w  $0630, $0710, $0720, $0731, $0741, $0810, $0820, $0831
		dc.w  $0841, $0900, $0910, $0930, $0940, $0950, $0951, $0953
		dc.w  $0962, $0963, $0A10, $0A30, $0A31, $0A50, $0A52, $0A53
		dc.w  $0A62, $0A70, $0A72, $0B10, $0B30, $0B31, $0B41, $0B51
		dc.w  $0B53, $0B62, $0B63, $0B72, $0B74, $0B80, $0B81, $0B83
		dc.w  $0B84, $0B85, $0B91, $0B93, $0B94, $0C20, $0C21, $0C41
		dc.w  $0C51, $0C53, $0C62, $0C70, $0C72, $0C74, $0C75, $0C82
		dc.w  $0C84, $0C85, $0C90, $0C92, $0C94, $0C95, $0C96, $0C97
		dc.w  $0C97, $0CA0, $0CA3, $0D20, $0D31, $0D50, $0D52, $0D61
		dc.w  $0D70, $0D72, $0D74, $0D75, $0D82, $0D84, $0D90, $0D91
		dc.w  $0D94, $0D95, $0D96, $0D97, $0DA0, $0DA0, $0DA3, $0DA5
		dc.w  $0DA6, $0DA7, $0DA8, $0DA8, $0DA8, $0DB1, $0DB4, $0DB6
		dc.w  $0DB7, $0DB7, $0DB8, $0DB8, $0DB9, $0DB9, $0DB9, $0DBA
		dc.w  $0DBA, $0DBA, $0DBA, $0DBA, $0DC2, $0DC5, $0DC6, $0E00
		dc.w  $0E31, $0E52, $0E62, $0E72, $0E75, $0E81, $0E85, $0E90
		dc.w  $0E93, $0E96, $0E97, $0E98, $0E98, $0EA2, $0EA6, $0EA7
		dc.w  $0EA8, $0EA9, $0EA9, $0EA9, $0EB3, $0EB7, $0EB8, $0EB9
		dc.w  $0EB9, $0EBA, $0EBA, $0EBA, $0EBA, $0EC0, $0EC0, $0EC0
		dc.w  $0EC7, $0EC7, $0EC8, $0EC8, $0EC9, $0ECA, $0ECA, $0ECA
		dc.w  $0ECA, $0ECB, $0ECB, $0ECB, $0ED0, $0ED0, $0ED0, $0ED7
		dc.w  $0ED7, $0ED7, $0ED9, $0ED9, $0ED9, $0EDA, $0EDA, $0EDA
		dc.w  $0EDB, $0EDB, $0EDB, $0F00, $0F00, $0F00, $0F70, $0F70
		dc.w  $0F90, $0F90, $0F90, $0FA0, $0FA0, $0FA9, $0FB0, $0FB0
		dc.w  $0FB0, $0FBA, $0FBA, $0FC0, $0FC0, $0FC9, $0FC9, $0FCA
		dc.w  $0FCA, $0FCA, $0FCB, $0FCB, $0FCB, $0FCC, $0FCC, $0FCC
		dc.w  $0FD0, $0FD0, $0FD0, $0FD0, $0FD0, $0FD0, $0FD0, $0FDB
		dc.w  $0FDB, $0FDB, $0FDB, $0FDB, $0FDB, $0FDB, $0FDC, $0FDC
		dc.w  $0FDC, $0FDC, $0FDC, $0FDC, $0FDD, $0FDD, $0FDD, $0FDD
		dc.w  $0FDD, $0FDD, $0FE0, $0FE0, $0FE0, $0FE0, $0FE0, $0FE0
		dc.w  $0FE0, $0FE0, $0FE0, $0FEC, $0FEC, $0FEC, $0FEC, $0FEC
		dc.w  $0FEC, $0FED, $0FED, $0FED, $0FED, $0FED, $0FED, $0FED
	sam_data:
		dc.l  0   ; Frame Address Counter
		dc.l  0   ; Frame End Address
		dc.l  0   ; Frame Base Address
	tabelle:
		ds   4096
	bye:
	} /*$C+ Case sensitive */
#endif
#undef s
}


void PlaySoundN(unsigned n)
{
	/*unsigned i;*/ SoundType s;
	if (!SoundAusgabe) return;
	if (n == 0)
		PlaySound(&SoundOff);
	else if (LoadSound(n, &s))
		PlaySound(&s);
}

void LoopSoundN(unsigned n)
{
	DoLoop = TRUE;
	PlaySoundN(n);
	DoLoop = FALSE;
}


static void __attribute__ ((constructor)) at_init(void)
{
	SoundOff.Buffer = /*HASCSSystem.*/NULL;
	DoLoop = FALSE;
	SoundAusgabe = TRUE;
	*SoundPath = *"";
#if 1
	PlaySound = PlaySoundSDL;

	/* AUDIO_S8 würde zu Endlosschleife wegen Bug in SDL führen... */
	desired.format = AUDIO_U8;
	desired.freq = /*22050*/ 25033;
	desired.samples = 1024;
	desired.channels = 1; /* Mono */
	desired.callback = SoundCallback;
	desired.userdata = NULL;

	if (SDL_OpenAudio(&desired, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
#endif
#if 0
	if (DMASound())
		PlaySound = PlaySoundDMA;
	else {
		PlaySound = PlaySoundInterrupt;
		EnterSupervisorMode(stack); /* Supervisormode */
		conterm &= ~ ((1<<8)|(1<<10)); /* Tastaturklick und Bell aus */
		LeaveSupervisorMode(stack);
	}
#endif
}
