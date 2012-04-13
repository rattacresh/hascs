/* Dialog module */
#include "compat.h"
#include <arpa/inet.h> /* byte order htons() ntohs()*/
#include "Dialog.h"
/*
   Version 0.1   06.03.93
           0.2   19.03.93 FindLabel
           0.3   25.04.93 InString
           0.4   06.08.93 Fensterausgabe
           0.5   09.08.93 Umstellung auf Befehlsworte, Monster-Variablen
           0.6   10.08.93 Spielervariablen, Bilder, Flags, Feldänderung
           0.7   11.08.93 Umstellung auf Adressen
           0.8   13.08.93 Zufall, Select
           0.9   17.08.93 Aim
           0.91  19.08.93 Copy, Debug
           0.92  20.08.93 Delete
           0.95  21.98.93 CodeDialog
           0.96  24.08.93 Invert, Sound, Output, Gosub, Kommentar
           0.97  27.08.93 Teleport
           0.98  23.10.93 Druckerausgabe
           0.99  23.01.94 Sound
           1.00  24.04.94 Variablenumstellung
           1.10  07.06.94 Stringvariablen, Labelarray
           1.20  12.06.94 Operatoren Priorität, CFG Datei Umstellung
           1.30  13.06.94 Ausdrücke, Call
           2.00  24.06.94 Klammern, Stringterme
           2.01  17.07.94 Levelvariablen
           3.00  19.08.94 lokale Variablen (A-Z), Time, Binäre Suche

           4.00  20.09.94 Megamax Umstellung, Tokenisierung
*/

#include "HASCSGlobal.h"
#include "HASCSGraphics.h"
#include "HASCSOutput.h"
#include "HASCSSystem.h"
#include "HASCSDisk.h"
#include "HASCSGame.h"
#include "Image.h"
#include "Sound.h"


#define MaxCommand (1 + sizeof XCommand / sizeof *XCommand) /* Kommandos */
#define MaxOperator (1 + sizeof Operator / sizeof *Operator) /* Operatoren */
#define MaxVariable (1 + sizeof Variable / sizeof *Variable) /* Variablen */

#define MaxGosub 10     /* Gosub Ebenen */
#define MaxZeilen 25    /* Textzeilen */
#define MaxLabel 50     /* Labels */

#define TraceDebug 0
#define VarDebug 1
#define TokenDebug 2
#define NoText 3
#define NoWait 4

#define TextEndeToken 0
#define NumberToken 1     /* Konstante */
#define VariableToken 2   /* Variable */
#define StringToken 3     /* Zeichenkette */
#define OperatorToken 4   /* Operator */
#define KommaToken 5      /* Komma */
#define ByteToken 6       /* Konstante <256 */
#define CommandToken 7    /* Befehl */
#define KlammerAufToken 8
#define KlammerZuToken 9
#define ZeilenEndeToken 10
#define AusgabeToken 11

typedef char String80Type[80];
typedef unsigned *CardPtr;
typedef String80Type *StringPtr;
/*typedef unsigned CharSet[(1 << CHAR_BIT) / sizeof (unsigned)];*/
typedef char *CharSet;
#ifndef SOUND_H
typedef char *CharPtr;
#endif

typedef void (*ActionProc)(CharPtr *);
typedef struct {
	char name[21];
	ActionProc action;
} CommandType;
typedef struct {
	char name[3];
	unsigned operation;
} OperatorType;
typedef struct {
	char name[21];
	void *loc;
	unsigned number;
	unsigned type;
} VariableType;
typedef struct {
	unsigned LabelNumber;
	void *LabelAddress;
} LabelType;

static unsigned fx, fy, fw, fh/*, i*/; /* Fensterausmaße in Sprites */
static unsigned ZeilenAus, ZeilenPos = 1, Redraw,
    DialogBreite, DialogHoehe, DialogNummer = 9999,
    x, y, w, h;
static unsigned long DialogLaenge;
static CharPtr DialogBuffer; /* Dialogpuffer */
static int Continue, OpenWindow; /* Dialogausführung */
static String80Type ein;
static unsigned long DebugLevel;
static CharSet VarSet = "abcdefghijklmnopqrstuvwxyz"
		 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		 ".0123456789",
	OpSet = ":=+-|&<>*/~#",
	NumberSet = "0123456789";

static MonsterTyp *SelectedMonster = NULL;
static GegenstandTyp *SelectedGegenstand = NULL;
static ParameterTyp *SelectedParameter = NULL;

static LabelType ReturnAddress[MaxGosub+1]; /* GOSUB Stack */
static unsigned ReturnLevel;

static LabelType Labels[MaxLabel+1]; /* Label Adressen */
static unsigned LabelAnzahl;
    
static String80Type DialogText[MaxZeilen + 1]; /* Texte für Hyperclick */

static unsigned LocalVar[26];
    
static void Nothing(CharPtr *p), Goto(CharPtr *p), Input(CharPtr *p),
	End(CharPtr *p), Label(CharPtr *p), Window(CharPtr *p),
	If(CharPtr *p), Picture(CharPtr *p), Wait(CharPtr *p),
	Select(CharPtr *p), XCopy(CharPtr *p), Aim(CharPtr *p),
	XTeleport(CharPtr *p), Delete(CharPtr *p), Sound(CharPtr *p),
	Invert(CharPtr *p), Output(CharPtr *p), Gosub(CharPtr *p),
	Return(CharPtr *p), Call(CharPtr *p);

static CommandType XCommand[] = {
	/* Kommandos */
	{"", Nothing},
	{"GOTO", Goto},
	{"INPUT", Input},
	{"END", End},
	{"LABEL", Label},
	{"WINDOW", Window},
	{"IF", If},
	{"LET", Nothing},
	{"PICTURE", Picture},
	{"WAIT", Wait},
	{"SELECT", Select},
	{"THEN", Nothing},
	{"COPY", XCopy},
	{"AIM", Aim},
	{"TELEPORT", XTeleport},
	{"DELETE", Delete},
	{"SOUND", Sound},
	{"INVERT", Invert},
	{"OUTPUT", Output},
	{"GOSUB", Gosub},
	{"RETURN", Return},
	{"CALL", Call},
};

static OperatorType Operator[] = {
	{"", 0}, /* Dummy for start index 1 */

	/* Operatoren */
	{":=", 1},
	{"+", 2},
	{"-", 3},
	{"|", 4},
	{"&", 5},
	{"=", 6},
	{"<", 7},
	{">", 8},
	{"#", 9},
	{"*", 10},
	{"/", 11},
	{"<=", 12},
	{">=", 13},
	{"~", 14},
};


static VariableType Variable[] = {
	{"DUMMY", NULL, 0, NumberToken}, /* Dummy for start index 1 */
	
	/* Variablen */
	{"SPIELER.TP", &Spieler.TP, 0, NumberToken},
	{"SPIELER.GOLD", &Spieler.Gold, 0, NumberToken},
	{"SPIELER.NAHRUNG", &Spieler.Nahrung, 0, NumberToken},
	{"SPIELER.GRAD", &Spieler.Grad, 0, NumberToken},
	{"SPIELER.ST", &Spieler.St, 0, NumberToken},
	{"SPIELER.KO", &Spieler.Ko, 0, NumberToken},
	{"SPIELER.GE", &Spieler.Ge, 0, NumberToken},
	{"SPIELER.IN", &Spieler.In, 0, NumberToken},
	{"SPIELER.ZT", &Spieler.Zt, 0, NumberToken},
	{"SPIELER.CH", &Spieler.Ch, 0, NumberToken},
	{"SPIELER.MUSTER", &Spieler.Sprite, 0, NumberToken},
	{"SPIELER.SICHTWEITE", &Spieler.Sichtweite, 0, NumberToken},
	{"SPIELER.LEVEL", &Spieler.LevelNumber, 0, NumberToken},
	{"SPIELER.X", &Spieler.x, 0, NumberToken},
	{"SPIELER.Y", &Spieler.y, 0, NumberToken},
	{"SPIELER.STATUS", &Spieler.Status, 0, NumberToken},
	{"SPIELER.KLASSE", &Spieler.Typ, 0, NumberToken},
	{"SPIELER.EP", &Spieler.EP+2, 0, NumberToken},
	{"SPIELER.ZUEGE", &Spieler.Moves+2, 0, NumberToken},
	{"SPIELER.LERNEN", &Spieler.Lernen, 0, NumberToken},
	{"SPIELER.TPMAX", &Spieler.TPMax, 0, NumberToken},
	{"SPIELER.PERMANENT", &Spieler.Permanent, 0, NumberToken},
	{"SPIELER.NAME", &Spieler.Name, 0, StringToken},

	{"SPIELER.RECHTEHAND", NULL, 60001, NumberToken},
	{"SPIELER.LINKEHAND", NULL, 60002, NumberToken},
	{"SPIELER.RUESTUNG", NULL,  60003, NumberToken},
	{"SPIELER.RING", NULL, 60004, NumberToken},
	{"SPIELER.REITTIER", NULL, 60005, NumberToken},
	{"SPIELER.RUCKSACK", NULL, 60000, NumberToken},
	{"MONSTER", NULL, 60006, NumberToken},
	{"GEGENSTAND", NULL, 60007, NumberToken},
	{"PARAMETER", NULL, 60008, NumberToken},

	{"SCHALTER", NULL, 50, NumberToken},
	{"ZUFALL", NULL, 51, NumberToken},
	{"FELD", NULL, 52, NumberToken},
	{"EINGABE", &ein, 0, StringToken}, /* Eingabezeichenkette */

	{"MONSTER.STATUS", NULL, 1, NumberToken},
	{"MONSTER.DIALOG", NULL, 2, NumberToken},
	{"MONSTER.TP", NULL, 3, NumberToken},
	{"MONSTER.X", NULL, 4, NumberToken},
	{"MONSTER.Y", NULL, 5, NumberToken},
	{"MONSTER.SPEZIAL", NULL, 6, NumberToken},
	{"MONSTER.TYP", NULL, 7, NumberToken},
	{"MONSTER.NAME", NULL, 8, StringToken},

	{"GEGENSTAND.MUSTER", NULL, 11, NumberToken},
	{"GEGENSTAND.SPEZIAL", NULL, 12, NumberToken},
	{"GEGENSTAND.TYP", NULL, 13, NumberToken},
	{"GEGENSTAND.PAR1", NULL, 14, NumberToken},
	{"GEGENSTAND.PAR2", NULL, 15, NumberToken},
	{"GEGENSTAND.PAR3", NULL, 16, NumberToken},
	{"GEGENSTAND.FLAGS", NULL, 17, NumberToken},
	{"GEGENSTAND.DIALOG", NULL, 18, NumberToken},
	{"GEGENSTAND.NAME", NULL, 19, StringToken},

	{"PARAMETER.ART", NULL, 21, NumberToken},
	{"PARAMETER.PAR1", NULL, 22, NumberToken},
	{"PARAMETER.PAR2", NULL, 23, NumberToken},
	{"PARAMETER.PAR3", NULL, 24, NumberToken},
	{"PARAMETER.PAR4", NULL, 25, NumberToken},
	{"PARAMETER.PAR5", NULL, 26, NumberToken},
	{"PARAMETER.PAR6", NULL, 27, NumberToken},
	{"REDRAW", &Redraw, 0, NumberToken},
	{"DEBUG", &DebugLevel, 0, NumberToken},

	{"LEVEL.BREITE", &LevelBreite, 0, NumberToken},
	{"LEVEL.HOEHE", &LevelHoehe, 0, NumberToken},
	{"LEVEL.SICHTWEITE", &LevelSichtweite, 0, NumberToken},
	{"LEVEL.FLAGS", &LevelFlags, 0, NumberToken},
	{"LEVEL.DIALOG", &LevelDialog, 0, NumberToken},
	{"LEVEL.MAXMONSTER", &LevelMaxMonster, 0, NumberToken},
	{"LEVEL.NAME", &LevelName, 0, StringToken},

	{"FELD.NAME", NULL, 31, StringToken},
	{"FELD.SPEZIAL", NULL, 32, NumberToken},
	{"TEXT.STRING", NULL, 33, StringToken},
	{"TEXT.SOUND", NULL, 34, NumberToken},

	{"DIALOGPFAD", &DiaPath, 0, StringToken},
	{"KARTENPFAD", &MapPath, 0, StringToken},
	{"SPIELERPFAD", &PlaPath, 0, StringToken},
	{"SYSTEMPFAD", &PrgPath, 0, StringToken},
	{"SOUNDPFAD", &SoundPath, 0, StringToken},
	{"TEXTEDITOR", &TextEditor, 0, StringToken},
	{"INIDATEI", &IniFile, 0, StringToken},

	{"TIME", NULL, 60, NumberToken},
	
	{"A", &LocalVar[0], 0, NumberToken},
	{"B", &LocalVar[1], 0, NumberToken},
	{"C", &LocalVar[2], 0, NumberToken},
	{"D", &LocalVar[3], 0, NumberToken},
	{"E", &LocalVar[4], 0, NumberToken},
	{"F", &LocalVar[5], 0, NumberToken},
	{"G", &LocalVar[6], 0, NumberToken},
	{"H", &LocalVar[7], 0, NumberToken},
	{"I", &LocalVar[8], 0, NumberToken},
	{"J", &LocalVar[9], 0, NumberToken},
	{"K", &LocalVar[10], 0, NumberToken},
	{"L", &LocalVar[11], 0, NumberToken},
	{"M", &LocalVar[12], 0, NumberToken},
	{"N", &LocalVar[13], 0, NumberToken},
	{"O", &LocalVar[14], 0, NumberToken},
	{"P", &LocalVar[15], 0, NumberToken},
	{"Q", &LocalVar[16], 0, NumberToken},
	{"R", &LocalVar[17], 0, NumberToken},
	{"S", &LocalVar[18], 0, NumberToken},
	{"T", &LocalVar[19], 0, NumberToken},
	{"U", &LocalVar[20], 0, NumberToken},
	{"V", &LocalVar[21], 0, NumberToken},
	{"W", &LocalVar[22], 0, NumberToken},
	{"X", &LocalVar[23], 0, NumberToken},
	{"Y", &LocalVar[24], 0, NumberToken},
	{"Z", &LocalVar[25], 0, NumberToken},

	{"MONSTER.TREFFERWURF", NULL, 101, NumberToken},
	{"MONSTER.SCHADEN", NULL, 102, NumberToken},
	{"MONSTER.BONUS", NULL, 103, NumberToken},

	{"GEGENSTAND.X", NULL, 111, NumberToken}, 
	{"GEGENSTAND.Y", NULL, 112, NumberToken},

	{"PARAMETER.X", NULL, 121, NumberToken},
	{"PARAMETER.Y", NULL, 122, NumberToken},
};

/* Forward Deklarationen ************************************************/

static unsigned GetNumber(CharPtr *p);

static void *GetItem(unsigned n, CharPtr *p, unsigned *r, char *s);
static unsigned EvalTerm(CharPtr *p, unsigned n);
static void EvalString(CharPtr *p, char *s);
static void NewLabel(unsigned l, CharPtr p);

/* Hilfprozeduren *******************************************************/

static void DialogFehler(char *s, char *q, unsigned c)
{
	char err[256], num[256];
	err[0] = '\0';
	if (DialogNummer < 1000) {
		CardToString(DialogNummer, 1, num);
		Concat(err, "Dialog ", num);
		Concat(err, err, ": ");
	}
	Concat(err, err, s); Concat(err, err, q);
	if (c != 65535) {
		CardToString(c, 1, num);
		Concat(err, err, num);
	}
	Error(err, 3);
}

static unsigned Max(unsigned x, unsigned y)
{
	if (x > y) return x; else return y;
}

static unsigned Min(unsigned x, unsigned y)
{
	if (x < y) return x; else return y;
}

static unsigned GetToken(CharPtr *ref_p, unsigned *ref_n, char *s)
{
#define p (*ref_p)
#define n (*ref_n)
	unsigned i, t;

	t = *p;
	switch (t) {
	case OperatorToken:
	case VariableToken:
	case CommandToken:
		p++;
		n = *p++;
		break;
	case ByteToken:
		p++;
		n = *p++;
		t = NumberToken;
		break;
	case NumberToken:
		p++;
		n = *p++;
		n = n * 256 + *p++;
		break;
	case StringToken:
		p++;
		n = *p++;
		if (n > 0)
			for (i = 0; i <= n -1; i++)
				s[i] = *p++;
		s[n] = '\0';
		break;
	case KlammerAufToken:
	case KlammerZuToken:
	case KommaToken:
		p++;
		break;
	case AusgabeToken:
		i = 0; p++;
		while (*p >= ' ')
			s[i++] = *p++;
		s[i] = '\0';
		break;
	}
	return t;
#undef p
#undef n
}

static int NextLine(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned t, i;
	String80Type s;

	do {
		t = GetToken(&p, &i, s);
		if (t == TextEndeToken) return FALSE;
	} while (t != ZeilenEndeToken);
	p++;
	return *p != TextEndeToken;
#undef p
}


static int Tokenize(CharPtr p, CharPtr q, unsigned long *ref_l)
{
#define l (*ref_l)
	unsigned t, n, i, Line;
	char s[80];
	int ok = TRUE, label = FALSE, ende;
	CharPtr start;

	int LastLine(CharPtr *ref_p)
	{
#define p (*ref_p)
		while (*p >= ' ') p++;
		while (*p != '\0' && *p != 10) p++;
		if (*p == 10) p++;
		Line++;
		return *p == '\0';
#undef p
	}

	void GetLine(CharPtr *ref_p, char *s)
	{
#define p (*ref_p)
		unsigned i;
		for (i = 0; i <= HIGH(s); i++) {
			if (*p < ' ') { s[i] = '\0'; return; }
			s[i] = *p++;
		}
#undef p
	}

	unsigned GetToken(CharPtr *ref_p, char *s)
	{
#define p (*ref_p)
		unsigned i = 0, t = 0;
		while (*p == ' ' || *p == 8) p++;
		if (INSet(*p,OpSet)) { /* Operator */
			t = OperatorToken;
			while (INSet(*p,OpSet))
				s[i++] = *p++;
		} else if (*p == '"') { /* String */
			t = StringToken;
			p++;
			while (*p >= ' ' && *p != '"')
				s[i++] = *p++;
			if (*p == '"') p++;
		} else if (INSet(*p,NumberSet)) {
			t = NumberToken;
			while (INSet(*p,NumberSet))
				s[i++] = *p++;
		} else if (*p == ',') {
			t = KommaToken;
			s[i++] = *p++;
		} else if (*p == '(') {
			t = KlammerAufToken;
			s[i++] = *p++;
		} else if (*p == ')') {
			t = KlammerZuToken;
			s[i++] = *p++;
		} else if (INSet(*p,VarSet)) {
			t = VariableToken;
			while (INSet(*p,VarSet))
				s[i++] = *p++;
		}
		s[i] = '\0';
#if 0
		printf("Token %d <%s>\n", t, s);
#endif
		return t;
#undef p
	}

	unsigned FindOperator(char *s)
	{
		unsigned i;
		for (i = 1; i <= MaxOperator; i++)
			if (COMPARE(Operator[i].name, s))
				return Operator[i].operation;

		DialogFehler("Unbekannter Operator ", s, 65535);
		ok = FALSE;
		return 0; /* Unbekannter Operator */
	}

	unsigned FindVariable(char *s, unsigned *t, int *ok)
	{
		unsigned i;
		for (i = 1; i <= MaxCommand; i++)
			if (COMPARE(XCommand[i].name, s)) {
				*t = CommandToken;
				return i;
			}
		for (i = 1; i <= MaxVariable; i++)
			if (COMPARE(Variable[i].name, s)) {
				*t = VariableToken;
				return i;
			}
		DialogFehler("Unbekannte/r Variable/Befehl: ", s, 65535);
		*ok = FALSE;
		return 0;
	}

	void Out(unsigned a)
	{
		*q++ = a % 256;
	}
	void OutC(char a)
	{
		*q++ = a;
	}

	DialogBreite = 0; DialogHoehe = 0; Line = 1; LabelAnzahl = 0;
	ok = 1; label = 0; start = q;
	do {
		if (*p == '#') { /* Kommando */
			p++;
			t = GetToken(&p, s);
			while (t != 0) {
				switch (t) {
				case OperatorToken :
					n = FindOperator(s);
					Out(t); Out(n);
					break;
				case VariableToken:
					n = FindVariable(s, &t, &ok);
					if (!COMPARE(s, "LET")) {
						Out(t); Out(n);
						label = t == CommandToken
							&& n == 4;
					}
					break;
				case KommaToken:
				case KlammerAufToken:
				case KlammerZuToken:
					Out(t);
					break;
				case NumberToken:
					n = StringToCard(s);
					if (n < 256) {
						Out(ByteToken); Out(n);
					} else  {
						Out(t); Out(n / 256);
						Out(n % 256);
					}
					if (label) {
						label = FALSE;
						NewLabel(n, q);
					}
					break;
				case StringToken:
					n = LENGTH(s);
					Out(t); Out(n);
					if (n > 0)
						for (i = 0; i <= n - 1; i++)
							OutC(s[i]);
				}
				t = GetToken(&p, s);
			}
			ende = LastLine(&p);
			if (!ende) Out(ZeilenEndeToken); /* LF */
		} else if (*p !=  '*') { /* Ausgabezeile */
			GetLine(&p, s);
			n = LENGTH(s);
			Out(AusgabeToken);
			if (n > 0)
				for (i = 0; i <= n - 1 ; i++)
					OutC(s[i]);
			
			if (n > DialogBreite) DialogBreite = n;
			DialogHoehe++;
			ende = LastLine(&p);
			if (!ende) Out(ZeilenEndeToken); /* LF */
		} else /* Kommentar */
			ende = LastLine(&p);
	} while (!ende && ok);
	Out(0); /* Endekennzeichnung */
	l = q - start;
	return ok;
#undef l
}

/************************************************************************/

static void *GetLabel(unsigned l)
{
	unsigned i = 1;
	while (i <= LabelAnzahl) {
		if (Labels[i].LabelNumber == l)
			return Labels[i].LabelAddress;
		i++;
	}
	return NULL;
}

static void NewLabel(unsigned l, CharPtr p)
{
	if (LabelAnzahl < MaxLabel) {
		LabelAnzahl++;
		Labels[LabelAnzahl].LabelNumber = l;
		Labels[LabelAnzahl].LabelAddress = p;
	}
}

static int FindLabel(CharPtr *ref_p, unsigned l)
{
#define p (*ref_p)
	unsigned cmd, t; String80Type s;

	p = GetLabel(l); /* Schon in Liste? */
	if (p != NULL) return TRUE;
	p = DialogBuffer;
	do {
		t = GetToken(&p, &cmd, s);
		if (t == CommandToken && cmd == 4) /* Label */
			if (GetNumber(&p) == l) {/* Label gefunden */
				NewLabel(l, p);
				return TRUE;
			}
	} while (NextLine(&p));
	DialogFehler("Label nicht gefunden ", "", l);
	return FALSE;
#undef p
}

/************************************************************************/

static void PrinterLine(char *s)
{
	unsigned i;
	if (!PrinterStatus())
		WaitTime(1000);

	for (i = 0; i <= HIGH(s) ; i++) {
		if (s[i] == '\0') {
			PrinterOut(13); PrinterOut(10);
			return;
		}
		PrinterOut(s[i]);
	}
	PrinterOut(13); PrinterOut(10);
}


/* Variablenbearbeitung *************************************************/

static unsigned GetVariable(CharPtr *ref_p, unsigned *ref_c, char *s)
{
#define p (*ref_p)
#define c (*ref_c)
	unsigned type, i, r;
	char q[80];
	/*CharPtr v;*/

	c = 0;
	
	type = GetToken(&p, &i, q);
	switch (type) {
	case VariableToken:
		/*v =*/ GetItem(i, &p, &r, q);
		c = r;
		Assign(s, q);
		return Variable[i].type;
	case KlammerAufToken: /* ( */
		type = GetVariable(&p, &i, q);
		switch (type) {
		case StringToken:
			EvalString(&p, q);
			Assign(s, q);
			break;
		case NumberToken:
			c = EvalTerm(&p, i);
		}
		break;
	case StringToken:
		Assign(s, q);
		break;
	case NumberToken:
		c = i;
		/* FALLTHROUGH */
	case ZeilenEndeToken:
		break;
	default:
		DialogFehler("Fehlerhafter Ausdruck!", "", 65535);
	}
	return type;
#undef c
#undef p
}

static unsigned GetNumber(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned c, t;
	String80Type s;

	t = GetVariable(&p, &c, s);
	if (t == StringToken) {
		c = StringToCard(s);
		t = NumberToken;
	}
	if (t == NumberToken)
	       return EvalTerm(&p, c);
	else
	       return 0;
#undef p
}

static unsigned GetArgument(CharPtr *ref_p)
{
#define p (*ref_p)
	String80Type s;
	unsigned n;
	if (GetToken(&p, &n, s) == KlammerAufToken)
		return GetNumber(&p);
	DialogFehler("Klammer auf '(' erwartet", "", 65535);
	Continue = FALSE;
	return 0;
#undef p
}

static void GetString(CharPtr *ref_p, char *s)
{
#define p (*ref_p)
	unsigned c;
	if (GetVariable(&p, &c, s) == NumberToken)
		CardToString(c, 1, s);
	EvalString(&p, s);
#undef p
}

static void *GetAddress(CharPtr *ref_p, unsigned *ref_t, unsigned *ref_c, char *s)
{
#define p (*ref_p)
#define t (*ref_t)
#define c (*ref_c)
	void *v;
	unsigned i, k;
	char q[80];
	
	t = GetToken(&p, &i, s);
	if (t == VariableToken) {
		v = GetItem(i, &p, &c, s);
		if (v != NULL) {
			t = GetToken(&p, &k, q); /* Komma überlesen */
			t = Variable[i].type;
			return v;
		}
	}
	DialogFehler("Variable erwartet ", s, 65535);
	Continue = FALSE;
	return NULL;
#undef p
#undef t
#undef c
}

static unsigned FindText(unsigned n)
{
	unsigned i;

	for (i = 1; i <= AnzahlTexte; i++)
		if (Text[i].Nummer == n)
			return i;
	Text[++AnzahlTexte].Nummer = n;
	return AnzahlTexte;
}

static void *GetItem(unsigned n, CharPtr *ref_p, unsigned *ref_r, char *s)
{
#define p (*ref_p)
#define r (*ref_r)
	unsigned x, y; String80Type *str; CardPtr v;
	v = NULL;
	r = 0;
	if  (Variable[n].loc != NULL)
		v = Variable[n].loc;
	else if (Variable[n].number < 10 
	      || (Variable[n].number >= 100 && Variable[n].number < 110))
	{
		if (SelectedMonster) /* Monster */
			switch (Variable[n].number) {
			case 1: v = &SelectedMonster->Status; break;
			case 2: v = &SelectedMonster->Sprich; break;
			case 3: v = &SelectedMonster->TP; break;
			case 4: v = &SelectedMonster->x; break;
			case 5: v = &SelectedMonster->y; break;
			case 6: v = &SelectedMonster->Spezial; break;
			case 7: v = &SelectedMonster->Typ; break;
			case 8: v = (CardPtr)&SelectedMonster->Name; break;
			case 101: v = &SelectedMonster->Trefferwurf; break;
			case 102: v = &SelectedMonster->Schaden; break;
			case 103: v = &SelectedMonster->Bonus; break;
			}
	} else if (Variable[n].number < 20
	      || (Variable[n].number >= 110 && Variable[n].number < 120))
	{
		if (SelectedGegenstand)
			switch (Variable[n].number) {
			case 11 : v = &SelectedGegenstand->Sprite; break;
			case 12 : v = &SelectedGegenstand->Spezial; break;
			case 13 : v = &SelectedGegenstand->KennNummer; break;
			case 14 : v = &SelectedGegenstand->Ring; break;
			case 15 : v = &SelectedGegenstand->RingWirkung; break;
			case 16 : v = &SelectedGegenstand->RingDauer; break;
			case 17 : v = &SelectedGegenstand->Flags; break;
			case 18 : v = &SelectedGegenstand->Dialog; break;
			case 19 : v = (CardPtr)&SelectedGegenstand->Name; break;
			case 111: v = &SelectedGegenstand->x; break;
			case 112: v = &SelectedGegenstand->y; break;
			}
	} else if (Variable[n].number < 30
	      || (Variable[n].number >= 120 && Variable[n].number < 130))
	{
		if (SelectedParameter)
			switch (Variable[n].number) {
			case 21 : v = &SelectedParameter->Art; break;
			case 22 : v = &SelectedParameter->xhoch; break;
			case 23 : v = &SelectedParameter->yhoch; break;
			case 24 : v = &SelectedParameter->Levelhoch; break;
			case 25 : v = &SelectedParameter->xrunter; break;
			case 26 : v = &SelectedParameter->yrunter; break;
			case 27 : v = &SelectedParameter->Levelrunter; break;
			case 121: v = &SelectedParameter->x; break;
			case 122: v = &SelectedParameter->y; break;
		}

	} else if (Variable[n].number >= 60000) {
		r = Variable[n].number;
	} else {
		switch (Variable[n].number) {
		case 31:
			x = GetArgument(&p);
			if (x < MaxSprites)
				v = (CardPtr)&Felder[x].Name;
			break;
		case 32:
			x = GetArgument(&p);
			if (x < MaxSprites)
				v = &Felder[x].Spezial;
			break;
		case 33:
			x = GetArgument(&p);
			v = (CardPtr)&Text[FindText(x)].String;
			break;
		case 34:
			x = GetArgument(&p);
			v = &Text[FindText(x)].Sample;
			break;
		case 50:
			x = GetArgument(&p);
			if (x >= 1 && x <= MaxFlags)
				v = &Spieler.Flags[x];
			break;
		case 51:
			r = Zufall(GetArgument(&p));
			break;
		case 52:
			x = GetArgument(&p);
			y = GetNumber(&p);
			v = &Level[x][y].Feld;
			break;
		case 60:
			r = GetTime() % 65536;
			break;
		}
	}
	if (v) {
		if (Variable[n].type == NumberToken)
			r = *v;
		else if (Variable[n].type == StringToken) {
			str = (String80Type *)v;
			Assign(s, *str);
		}
	}
	return v;
#undef r
#undef p
}

/* Zahlausdruck auswerten ***********************************************/

static unsigned EvalTerm(CharPtr *ref_p, unsigned n)
{
#define p (*ref_p)
	unsigned v, op; String80Type h;

	while (GetToken(&p, &op, h) == OperatorToken) {
		if (GetVariable(&p, &v, h) == StringToken)
			v = StringToCard(h);

		switch (op) {
		case  1 : n = v; break; /* Zuweisung */
		case  2 : n += v; break; /* Addition */
		case  3 : if (n > v) n -= v; else n = 0; break; /* Subtraktion */
		case  4 : n = n | v; break; /* Vereinigungsmenge */
		case  5 : n = n & v; break; /* Schnittmenge */
		case  6 : n = n == v; break; /* Gleichheit */
		case  7 : n = n < v; break;
		case  8 : n = n > v; break;
		case  9 : n = n != v; break;
		case 10 : n = n * v; break; /* Multiplikation */
		case 11 : n = n / v; break; /* Division */
		case 12 : n = n <= v; break;
		case 13 : n = n >= v; break;
		case 14 : n = n & ~v; break;/* Mengendifferenz */
		}
	}
	return n;
#undef p
}

/* Zeichenkettenausdruck auswerten **************************************/

static void EvalString(CharPtr *ref_p, char *s)
{
#define p (*ref_p)
	unsigned /*t,*/ op, c; String80Type h;

	while (GetToken(&p, &op, h) == OperatorToken) {
		if (GetVariable(&p, &c, h) == NumberToken)
			CardToString(c, 1, h);

		switch (op) {
		case  1: Assign(s, h); break; /* = */
		case  2: Concat(s, s, h); break; /* sp */
		case  4: if (COMPARE(s, "1") || COMPARE(h, "1"))
				Assign(s, "1");
			else 
				Assign(s, "0");
			break;
		case  5: if (COMPARE(s, "1") && COMPARE(h, "1"))
				Assign(s, "1");
			else
				Assign(s, "0");
			break;
		case  6: if (COMPARE(s, h)) Assign(s, "1"); else Assign(s, "0"); break;
		case  9: if (!COMPARE(s, h))
				Assign(s, "1"); else Assign(s, "0");
			break;
		case 12: if (InString(s, h)) Assign(s, "1"); else Assign(s, "0"); break;
		case 13: if (InString(h, s)) Assign(s, "1"); else Assign(s, "0"); break;
		default:
			DialogFehler("Falscher Operator in Stringausdruck!", "", 65535);
			break;
		}
	}
#undef p
}

/* Textzeile aus-/eingeben **********************************************/

static void OpenScreen(int MinW, int MinH)
{
	unsigned i;
	if  (w == 0 || h == 0) {/* Breite defaultmäßig ermitteln */
		w = Max(Min(DialogBreite, 76), Min(80, MinW));
		h = Max(Min(DialogHoehe, 23), Min(25, MinH));
	}
	w = Min(80, w); h = Min(25, h);
	if (x == 0 || y == 0
		|| x + w > 79 ||  y + h > 24) /* zentrieren */
	{
		x = (40 - w / 2) / 2 * 2;
		y = (12 - h / 2);
	}
	fw = (w + 1) / 2; fh = h; fx = x / 2; fy = y;
	if (fx >= 1 && fy >= 1 && fx+fw < 40 && fy+fh < 25)
		ReserveScreen(fx - 1, fy - 1, fx + fw, fy + fh);
	else
		FillRectangle(0, 0, 39, 24, &SystemSprite[0]);
	
	ZeilenAus = 0;
	ZeilenPos = 0;
	for (i = 1; i <= MaxZeilen; i++) *DialogText[i] = *"";
	OpenWindow = FALSE;
}

static void Ausgabe(char *s)
{
	unsigned i;
	if (OpenWindow)
		OpenScreen(0, 0);

	if (ZeilenAus == h) {
		WaitKey();  /* mal abwarten */
		ZeilenAus = 0;
	}
	ZeilenAus++; /* Ausgegebene Zeilen */
	if (ZeilenPos < h)
		ZeilenPos++;
	else {
		ScrollUp(fx, fy, fw, fh);
		for (i = 2 ; i <= ZeilenPos; i++)
			Assign(DialogText[i-1], DialogText[i]);
	}
	PrintAt(x, y + ZeilenPos - 1, s);
	Assign(DialogText[ZeilenPos], s);
	if (DruckerAusgabe)
		PrinterLine(s);
}

/* Kommandos  ***********************************************************/

static void Label(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned label;

	label = GetNumber(&p);
	NewLabel(label, p);
#undef p
}

static void Goto(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned label;

	label = GetNumber(&p);
	Continue = FindLabel(&p, label);
#undef p
}

static void Gosub(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned label;
	
	label = GetNumber(&p);
	if (ReturnLevel < MaxGosub) {
		ReturnAddress[++ReturnLevel].LabelAddress = p;
		Continue = FindLabel(&p, label);
	} else {
		DialogFehler("Zuviele verschachtelte GOSUB", "", 65535);
		Continue = FALSE;
	}
#undef p
}

static void Return(CharPtr *ref_p)
{
#define p (*ref_p)
	if (ReturnLevel > 0)
		p = ReturnAddress[ReturnLevel--].LabelAddress;
	else {
		DialogFehler("RETURN ohne GOSUB", "", 65535);
		Continue = FALSE;
	}
#undef p
}

static void End(CharPtr *ref_p)
{
#define p (*ref_p)
	Continue = FALSE;
#undef p
}

static void Input(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned i, mx, my;
	BITSET mb;

	void GetWord(unsigned mx, unsigned my, char *s)
	{
		/* Ermittelt das Dialogwort an Bildschirmposition mx, my */
		unsigned i;

		my = my / 16; mx = mx / 8;
		if (my < y || my >= y + h
			|| mx < x || mx >= x + w)
			return;
		my = my - y + 1; mx = mx - x;
		if (mx >= LENGTH(DialogText[my])) return;
		while (mx > 0 && DialogText[my][mx] >= '0')
			mx--;
		if (DialogText[my][mx] < '0') mx++;
		i = 0;
		while (DialogText[my][mx] >= '0')
			s[i++] = DialogText[my][mx++];
		s[i] = '\0';
	}

	if  (OpenWindow)
		OpenScreen(20, 1);

	*ein = *"";
	if (ZeilenPos < h)
		ZeilenPos++;
	else {
		ScrollUp(fx, fy, fw, fh);
		for (i = 2; i <= ZeilenPos; i++)
			Assign(DialogText[i-1], DialogText[i]);
		
		*DialogText[ZeilenPos] = *"";
	}
	GotoXY(x, y + ZeilenPos - 1);
	InputClick(ein, w - 1, &mx, &my, &mb);
	if (mb) {
		GetWord(mx, my, ein);
		PrintAt(x, y + ZeilenPos - 1, ein);
	}
	Assign(DialogText[ZeilenPos], ein);
	ZeilenAus = 0;
	if (DruckerAusgabe)
		PrinterLine(ein);
#undef p
}

static void Aim(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned /*i,*/ x, y, qx, qy, mode, type;
	BITSET b;
	char ch;
	CardPtr varx, vary;
	char s[80];
	
	varx = GetAddress(&p, &type, &x, s);
	vary = GetAddress(&p, &type, &y, s);
	mode = GetNumber(&p);
	if (ZeilenAus > 0) {
		WaitKey(); ZeilenAus = 0;
	}
	if (!OpenWindow) { /* Dialogfenster löschen */
		RestoreScreen();
		OpenWindow = TRUE;
	}
	WaitInput(&x, &y, &b, &ch, -1);
	x = x / 16 - 1; y = y / 16 - 1;
	if  (x >= 0 && y >= 0
	  && x <= 22 && y <= 22 && MausLinks & b)
	{
		if (mode == 1) {
			qx = MaxSichtweite; qy = MaxSichtweite;
			if (MakeShoot(&qx, &qy, x, y, 30, TRUE)) {/* getroffen */
				x = qx; y = qy;
			}
		}
		SichtLevelUmrechnung(x, y, &x, &y);
	} else {
		x = Spieler.x; y = Spieler.y;
	}
	*varx = x; *vary = y;
#undef p
}


static void Nothing(CharPtr *ref_p)
{
#define p (*ref_p)
#undef p
}

static void Window(CharPtr *ref_p)
{
#define p (*ref_p)
	if (!OpenWindow) {
		RestoreScreen();
		OpenWindow = TRUE;
	}
	x = GetNumber(&p);
	y = GetNumber(&p);
	w = GetNumber(&p);
	h = GetNumber(&p);
#undef p
}

static void Output(CharPtr *ref_p)
{
#define p (*ref_p)
	String80Type s;

	if (ZeilenAus > 0) {
		WaitKey(); ZeilenAus = 0;
	}
	if (!OpenWindow) {/* Dialogfenster löschen */
		RestoreScreen();
		OpenWindow = TRUE;
	}
	GetString(&p, s);
	OutputText(s);
#undef p
}

static void Invert(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned x, y;

	x = GetNumber(&p);
	y = GetNumber(&p);
	if (LevelSichtUmrechnung(x, y, &x, &y))
		InvertFeld(x+1, y+1);
#undef p
}

static void Picture(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned n;
	n = GetNumber(&p); /* Bildnummer */
	ShowPicture(n, FALSE);
#undef p
}

void ShowPicture(unsigned n, int New)
{
	unsigned ys, ws, hs, i, PicW, PicH;
	if (!LoadImageN(n, &PicW, &PicH))
		return;
	if (New) {
		OpenWindow = TRUE;
		DialogBreite = 0; DialogHoehe = 0; x = 0; w = 0;
	}
	if (OpenWindow)
		OpenScreen((PicW + 8) / 8, (PicH + 16) / 16);
	ys = 0;
	ws = Min(PicW, 8 * w);
	do {
		if (ZeilenAus == h) {
			WaitKey();
			ZeilenAus = 0;
		}
		if (ZeilenPos < h) 
			ZeilenPos++;
		else {
			ScrollUp(fx, fy, fw, fh);
			for (i = 2; i <= ZeilenPos; i++)
				Assign(DialogText[i-1], DialogText[i]);
		}
		*DialogText[ZeilenPos] = *"";
		hs = Min(PicH - ys, 16);
		/*HASCSSystem.*/Copy(4, 0, ys, ws, hs,
			8 * x, 16 * (ZeilenPos + y - 1));
		ZeilenAus++;
		ys += 16;
	} while (ys < PicH);
	if (New) {
		if (ZeilenAus > 0)
			WaitKey();
		RestoreScreen();
	}
}


static void Let(CharPtr *ref_p, unsigned i)
{
#define p (*ref_p)
	CardPtr var;
	unsigned /*type,*/ value;
	String80Type s;
	StringPtr str;

	var = GetItem(i, &p, &value, s);
	if (var)
		switch (Variable[i].type) {
		case NumberToken:
			*var = EvalTerm(&p, value);
			break;
		case StringToken:
			str = (StringPtr)var; EvalString(&p, s); Assign(*str, s);
			break;
		}
#undef p
}


static void If(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned cmd, c, type;
	String80Type h;

	type = GetVariable(&p, &c, h);
	if (type == NumberToken) {
		if (EvalTerm(&p, c) == 0) return;
	} else if (type == StringToken) {
		EvalString(&p, h);
		if (!COMPARE(h, "1")) return;
	}
	type = GetToken(&p, &cmd, h);
	if (type == VariableToken)
		Let(&p, cmd);
	else if (type == CommandToken)
		XCommand[cmd].action(&p);
#undef p
}


static void Sound(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned n, mode; SoundType snd; /*String80Type s;*/

	n = GetNumber(&p);
	mode = GetNumber(&p);
	switch (mode) {
	case 1: if (LoadSound(n, &snd)); break;
	case 2: LoopSoundN(n); break;
	default:
		PlaySoundN(n); break;
	}
#undef p
}

static void Wait(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned t;

	t = GetNumber(&p);
	if (t == 0) {
		WaitKey();
		ZeilenAus = 0;
	} else
		WaitTime(t * 100);
#undef p
}

static void Select(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned x, y, i;

	x = GetNumber(&p);
	if (x <= MaxBreite) { /* Level */
		y = GetNumber(&p);
		i = GetNumber(&p);
		if (i == 60006) {
			SelectedMonster = NULL;
			if (LevelMonster & Level[x][y].Spezial)
				SelectedMonster = &Monster[FindMonster(x,y)];
		} else if (i == 60007) {
			SelectedGegenstand = NULL;
			if (LevelGegenstand & Level[x][y].Spezial)/* im Level */
				SelectedGegenstand = &Gegenstand[FindGegenstand(x,y)];
		} else if (i == 60008) {
			SelectedParameter = NULL;
			if (LevelParameter & Level[x][y].Spezial)
				SelectedParameter = &Parameter[FindParameter(x,y)];
		} else 
			x = i;
	}
	if (x < 60000) return;
	switch (x - 60000) {
	case 0: i = GetNumber(&p);
		SelectedGegenstand = NULL;
		if (i >= 1 && i <= MaxRuck)
			SelectedGegenstand = &Spieler.Rucksack[i];
		break;
	case 1: SelectedGegenstand = &Spieler.rechteHand; break;
	case 2: SelectedGegenstand = &Spieler.linkeHand; break;
	case 3: SelectedGegenstand = &Spieler.Ruestung; break;
	case 4: SelectedGegenstand = &Spieler.Ring; break;
	case 5:	SelectedMonster = NULL;
		if (SReitet & Spieler.Status)
			SelectedMonster = &Spieler.ReitTier;
		break;
	case 6:	i = GetNumber(&p); /* ( schon durch letztes GetNumber */
		SelectedMonster = NULL;
		if (i >= 1 && i <= AnzahlMonster)
			SelectedMonster = &Monster[i];
		break;
	case 7: i = GetNumber(&p);
		SelectedGegenstand = NULL;
		if (i >= 1 && i <= AnzahlGegen)
			SelectedGegenstand = &Gegenstand[i];
		break;
	case 8: i = GetNumber(&p);
		SelectedParameter = NULL;
		if (i >= 1 && i <= AnzahlParameter)
			SelectedParameter = &Parameter[i];
		break;
	}
#undef p
}

static void XCopy(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned i, x, y;

	x = GetNumber(&p);
	if (x <= MaxBreite) {/* Level */
		y = GetNumber(&p);
		i = GetNumber(&p);
		if (i == 60006 && SelectedMonster)
			NewMonster(x, y, SelectedMonster);
		else if (i == 60007 && SelectedGegenstand)
			NewGegenstand(x, y, SelectedGegenstand);
		else if (i == 60008 && SelectedParameter)
			NewParameter(x, y, SelectedParameter);
		else
			x = i; /* vielleicht doch für den Spieler? */

	}
	if (x == 60000) { /* Rucksack */
		i = GetNumber(&p);
		if (i >= 1 && i <= MaxRuck && SelectedGegenstand)
			Spieler.Rucksack[i] = *SelectedGegenstand;
	} else {
		if (x == 60001 && SelectedGegenstand)
			Spieler.rechteHand = *SelectedGegenstand;
		else if (x == 60002 && SelectedGegenstand)
			Spieler.linkeHand = *SelectedGegenstand;
		else if  (x == 60003 && SelectedGegenstand)
			Spieler.Ruestung = *SelectedGegenstand;
		else if (x == 60004 && SelectedGegenstand)
			Spieler.Ring = *SelectedGegenstand;
		else if (x == 60005 && SelectedMonster)
			Spieler.ReitTier = *SelectedMonster;
	}
#undef p
}

static void Delete(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned x, y, i;

	x = GetNumber(&p);

	if (x <= MaxBreite) { /* Level */
		y = GetNumber(&p);
		i = GetNumber(&p);
		if (i == 60006) DeleteMonster(x, y);
		else if (i == 60007) DeleteGegenstand(x, y);
		else if (i == 60008) DeleteParameter(x, y);
		else
			x = i;
	}
	if (x == 60000) {/* Rucksack */
		i = GetNumber(&p);
		if (i >= 1 && i <= MaxRuck)
			Spieler.Rucksack[i].KennNummer = 0;
	} else {
		if (x == 60001)
			Spieler.rechteHand.KennNummer = 0;
		else if (x == 60002)
			Spieler.linkeHand.KennNummer = 0;
		else if (x == 60003)
			Spieler.Ruestung.KennNummer = 0;
		else if (x == 60004)
			Spieler.Ring.KennNummer = 0;
		else if (x == 60005) {
			Spieler.ReitTier.Typ = 0; Spieler.Status &= ~SReitet;
		}
	}
#undef p
}


static void XTeleport(CharPtr *ref_p)
{
#define p (*ref_p)
	unsigned x, y, l;

	x = GetNumber(&p);
	y = GetNumber(&p);
	l = GetNumber(&p);
	if (l) {
		Spieler.x = x; Spieler.y = y;
		if (l != Spieler.LevelNumber) { /* neues Level laden */
			SaveLevel(Spieler.LevelNumber);
			Spieler.LevelNumber = l;
			LoadOrSavePlayer(FALSE);
			LoadLevel(Spieler.LevelNumber);
		}
	} else {
		Spieler.LevelNumber = 0;
	}
	Redraw = 1; /* Levelausschnitt neu zeichnen */
#undef p
}

static void Call(CharPtr *ref_p)
{
#define p (*ref_p)
#undef p
}


/* Dialog ausführen *****************************************************/

void ExecuteDialog(void)
{
	unsigned cmd, /*type, */token;
	String80Type s;
	CharPtr p;

	Continue = TRUE;
	OpenWindow = TRUE;
	x = 0; w = 0;
	Redraw = 0;
	ReturnLevel = 0; /* noch kein GOSUB */
	ZeilenAus = 0;
	DebugLevel = 0;
	LabelAnzahl = 0;
	p = DialogBuffer; /* Start am Pufferanfang */

	while (Continue) {
		token = GetToken(&p, &cmd, s);
		if (DebugLevel != 0) {
			switch (token) {
			case VariableToken: Assign(s, Variable[cmd].name); break;
			case CommandToken: Assign(s, XCommand[cmd].name); break;
			}
			WaitTime(0);
			Error(s, 2);
			Continue = ErrorResult = 2;
		}
		switch (token) {
		case VariableToken: Let(&p, cmd); break;
		case CommandToken: XCommand[cmd].action(&p); break;
		case AusgabeToken: Ausgabe(s); break;/* Zeichenkette ausgeben */
		default:
			DialogFehler("Befehl erwartet", "", 65535);
			Continue = FALSE;
		}
		Continue = Continue && NextLine(&p);
		/* XXX should NextLine always be invoked? even if Continue
		 * is FALSE? --rtc */
	} /* while (Continue) */

	if (!OpenWindow) {
		if (ZeilenAus > 0)
			WaitKey();
		RestoreScreen();
	}
	if (Redraw == 1) {
		MakeSichtBereich(TRUE);
		PrintCharakter(0);
	}
}


/* Dialog laden *********************************************************/

static void MakeFileName(int c, unsigned n, char *s)
{
	unsigned i;

	Assign(s, "");
	if (n == IniDialog) {
		if (c) Assign(s, "X");
		Concat(s, s, IniFile);
		Concat(s, PrgPath, s);
	} else if (n == FelderDialog) {
		if (c) Assign(s, "X");
		Concat(s, s, LevelSprites);
		Concat(s, s, ".DAT");
		Concat(s, PrgPath, s);
	} else if (n == ConfigDialog)
		Assign(s, /*HASCSSystem.*/Command);
	else if (n < 1000) {
		Assign(s, DiaPath);
		if (c)
			Concat(s, s, "XDIALOG.000");
		else
			Concat(s, s, "DIALOG.000");
		i = LENGTH(s) - 3;
		s[i]   = n / 100 + '0';
		s[i+1] = n % 100 / 10 + '0';
		s[i+2] = n % 10 + '0';
	}
}

static void OldCodeDialog(int code, unsigned long l, CharPtr p)
{
	unsigned i;
	if (code >= 0)
		SetzeZufall(code);
	else
		SetzeZufall(1);

	for (i = 0; i <= l - 1; i++)
		*p++ ^= Zufall(256) - 1;

	SetzeZufall(0L);
}
int OldSaveDialog(unsigned n, int coded, unsigned long l, CharPtr p)
{
	String80Type s;
	int f;

	if (coded)
		OldCodeDialog(BenutzerNummer, l, p);
	MakeFileName(coded, n, s);
	f = CreateFile(s);
	if (!FileError) {
		WriteFile(f, l, p);
		CloseFile(f);
	}
	if (coded)
		OldCodeDialog(BenutzerNummer, l, p);
	return !FileError;
}


int OldLoadDialog(unsigned n, int coded)
{
	unsigned long l;
	CharPtr p;
	String80Type s;
	int f;

	void BruteTest(CharPtr p)
	{
		if (p[0] >= 32 && p[1] >= 32 
		 && p[2] >= 32 && p[3] >= 32
		 && p[4] >= 32 && p[5] >= 32
		 && p[6] >= 32 && p[7] >= 32
		 && p[0] < 128 && p[1] < 128
		 && p[2] < 128 && p[3] < 128
		 && p[4] < 128 && p[5] < 128
		 && p[6] < 128 && p[7] < 128)
			printf("%d: %.8s\n", BenutzerNummer, p);
	}
	if (coded == -1) { /* Versuche es mit roher Gewalt... */
		unsigned i;
		unsigned old = BenutzerNummer;
		for (i = 0; i < 65536; i++) {
			BenutzerNummer = i;
			OldLoadDialog(n, 2);
		}
		BenutzerNummer = old;
		return TRUE;
	}

	MakeFileName(coded, n, s);
	l = FileLength(s); if (l == 0) return FALSE;
	p = GetBuffer(l);
	f = OpenFile(s); ReadFile(f, l, p); CloseFile(f);
	if (coded)
		OldCodeDialog(BenutzerNummer, l, p);

	if (coded == 2)
		BruteTest(p);
	else
		return OldSaveDialog(n, !coded, l, p);
	return TRUE;
}

void V1LoadOrSaveDialog(unsigned n, int Load, unsigned size)
{
	int f;
	unsigned DialogLength;
	unsigned DialogNumber;
	String80Type s;
	CharPtr p;
	unsigned long l;
	char Header[4];

	void MakeHeader(void)
	{
		Header[0] = DialogLength / 256;
		Header[1] = DialogLength % 256;
		Header[2] = DialogNumber / 256;
		Header[3] = DialogNumber % 256;
	}

	void AuswertHeader(void)
	{
		DialogLength = Header[0] * 256 + Header[1];
		DialogNumber = Header[2] * 256 + Header[3];
	}

	extern void V1DecodeBuffer(unsigned code, 
		unsigned long size, CharPtr Buffer);
	extern void V1CodeBuffer(unsigned code, 
		unsigned long size, CharPtr Buffer);

	if (Load) {
		MakeFileName(1, n, s);
		l = FileLength(s); if (l == 0) return;
		f = OpenFile(s);
		if (FileError)
			return;
		p = GetBuffer(DialogLength);
		ReadFile(f, 4, Header);
		AuswertHeader();

		/* wurden im original HASCS I unterschieden durch
		 * zwei verschiedene Ordner HASCS.INS und HASCS.DIA,
		 * mit Dateien TEXT.XXX vs. DIALOG.XXX
		 */
		if (size)
			l = 4+size*DialogLength;
		else {
			if (4+74*DialogLength == l)
				l = 74*DialogLength; /* Dialog */
			else if (4+61*DialogLength == l)
				l = 61*DialogLength; /* Text */
			else {
				Concat(s, "Falsche Dateigröße: ", s);
				Error(s, -1);
			}
		}

		ReadFile(f, l, p);
		CloseFile(f);
		V1DecodeBuffer(0, l, p);
		OldSaveDialog(n, 0, l, p);

		if (DialogNumber != n) {
			Concat(s, "Falsche Dialognummer: ", s);
			Error(s, -1);
		}
	} else {
		MakeFileName(0, n, s);
		l = FileLength(s); if (DialogLength == 0) return;
		p = GetBuffer(DialogLength);
		f = OpenFile(s); ReadFile(f, l, p); CloseFile(f);
		MakeFileName(1, n, s);
		DialogNumber = n;

		if (size)
			DialogLength = l / size;
		else {
			if (l % 74 == 0 && l % 61 == 0) {
				Concat(s, "Ambige Dateigröße: ", s);
				Error(s, -1);
			} else if (l % 74 == 0)
				DialogLength = l / 74;
			else if (l % 61 == 0)
				DialogLength = l / 61;
			else {
				Concat(s, "Falsche Dateigröße: ", s);
				Error(s, -1);
			}
		}
		f = CreateFile(s);

		MakeHeader();

		V1CodeBuffer(0, l, p);

		WriteFile(f, 4, Header);
		WriteFile(f, DialogLength, p);
	}

	CloseFile(f);
	
}
void V2LoadOrSaveDialog(unsigned n, int Load)
{
	/* Wird benutzt für kodierte Dialoge DIALOG.XXX 
	 * (vs unkodierte T_DIA.XXX) und für Strings HASCS_II.INI
	 * (vs. unkodiert HASCS_II.TXT)
	 */
	int f;
#if 0
	CharPtr end;
	unsigned long count;
#endif
	unsigned DialogLength;
	unsigned DialogNumber;
	String80Type s;
	CharPtr p;
	char Header[4];

	void MakeHeader(void)
	{
		Header[0] = DialogLength / 256;
		Header[1] = DialogLength % 256;
		Header[2] = DialogNumber / 256;
		Header[3] = DialogNumber % 256;
	}

	void AuswertHeader(void)
	{
		DialogLength = Header[0] * 256 + Header[1];
		DialogNumber = Header[2] * 256 + Header[3];
	}

	extern void ReadBlock(int handle, unsigned anzahl, CharPtr a);
	extern void WriteBlock(int handle, unsigned anzahl, CharPtr a);

	if (Load) {
#if 0
		if (n == CurrentDialogNumber)
			return
		Concat(s, DiaPath, "DIALOG.000");
		MakeNumber(s, n);
#else
		MakeFileName(1, n, s);
#endif
		f = OpenFile(s);
		if (FileError) {
#if 0
			V2LoadTextDialog();
			/* V2LoadTextDialog:
			 * ..."T_DIA.000" ... CurrentDialogNumber = n;
			 */
#endif
			return;
		}
#if 0
		LoadedDialogNumber = n;
		count = 2;
		ReadFile(f, count, &DialogLength);
		ReadFile(f, count, &DialogNumber);
		ReadBlock(f, DialogLength, Buffer + 20000);
		end = Buffer + 20000 + DialogLength;
		*end = '\0';
		Tokenize(Buffer + 20000);
#else
		p = GetBuffer(DialogLength);
		ReadFile(f, 4, Header);
		AuswertHeader();
		ReadBlock(f, DialogLength, p);
		OldSaveDialog(n, 0, DialogLength, p);
#endif

		if (DialogNumber != n) {
			Concat(s, "Falsche Dialognummer: ", s);
			Error(s, -1);
		}
	} else {
#if 0
		DialogNumber = CurrentDialogNumber;
		Concat(s, DiaPath, "DIALOG.000");
		MakeNumber(s, n)
		DeleteFile(s);
#else
		MakeFileName(0, n, s);
		DialogLength = FileLength(s); if (DialogLength == 0) return;
		p = GetBuffer(DialogLength);
		f = OpenFile(s); ReadFile(f, DialogLength, p); CloseFile(f);
		MakeFileName(1, n, s);
		DialogNumber = n;
#endif
		f = CreateFile(s);
#if 0
		count = 2;
		WriteFile(f, count, &DialogLength);
		WriteFile(f, count, &DialogNumber);
		WriteBlock(f, DialogLength, Buffer + 20000);
#else
		MakeHeader();
		WriteFile(f, 4, Header);
		WriteBlock(f, DialogLength, p);
#endif
	}

	CloseFile(f);
	
}
static void CodeDialog(unsigned long n, unsigned long l, void *b)
{
	unsigned long i;
	uint16_t *p = b;

	for (i = 1; i <= l / 2; i++) {
		n = (n * 153 + 97) % 16777216;
		*p = htons(ntohs(*p) ^ (n % 65536)); /* EXOR */
		p++;
	}
}

int LoadDialog(unsigned n, int coded)
{
	char s[128];
	int f; /* Filehandle */
	unsigned id, i;

	DialogNummer = n;
	id = n + 1; i = GetCache(id);
	if (i) {
		DialogBreite = Cache[i].CacheInfo1;
		DialogHoehe  = Cache[i].CacheInfo2;
		DialogBuffer = Cache[i].CacheBuffer;
		return TRUE;
	}

	MakeFileName(coded, n, s);
	DialogLaenge = FileLength(s);
	if (FileError) return FALSE;

	DialogBuffer = GetBuffer(DialogLaenge);
	f = OpenFile(s); ReadFile(f, DialogLaenge, DialogBuffer); CloseFile(f);

	if (coded)
		CodeDialog(BenutzerNummer, DialogLaenge, DialogBuffer);

	i = NewCache(id, DialogLaenge + 16);
	if (!Tokenize(DialogBuffer, Cache[i].CacheBuffer, &DialogLaenge)) {
		FreeCache(i);
		return FALSE;
	} /* Syntaxfehler */
	Cache[i].CacheInfo1 = DialogBreite;
	Cache[i].CacheInfo2 = DialogHoehe;
	DialogBuffer = Cache[i].CacheBuffer;
	return TRUE;
}

int SaveDialog(unsigned n, int coded)
{
	String80Type s;
	int f;
	unsigned long l;
	BITSET *p;

	MakeFileName(!coded, n, s);
	l = FileLength(s); if (l == 0) return FALSE;
	p = GetBuffer(l);
	f = OpenFile(s); ReadFile(f, l, p); CloseFile(f);
	CodeDialog(BenutzerNummer, l, p);
	MakeFileName(coded, n, s);
	f = CreateFile(s); WriteFile(f, l, p); CloseFile(f);
	return !FileError;
}


/* Dialog ausführen *****************************************************/

void DoDialog(unsigned n)
{
	/*unsigned i;*/
	if (!LoadDialog(n, FALSE)) /* unkodierter Dialog */
		if (!LoadDialog(n, TRUE)) { /* kodierter Dialog */
			DialogFehler("Dialog nicht geladen", "", 65535);
			return;
		}
	ExecuteDialog();
}

void DoMonsterDialog(unsigned n, MonsterTyp *ref_m)
{
#define m (*ref_m)
	SelectedMonster = &m;
	DoDialog(n);
#undef m
}

void DoGegenstandDialog(unsigned n, GegenstandTyp *ref_g)
{
#define g (*ref_g)
	SelectedGegenstand = &g;
	DoDialog(n);
#undef g
}

void DoParameterDialog(unsigned n, ParameterTyp *ref_p)
{
#define p (*ref_p)
	SelectedParameter = &p;
	DoDialog(n);
#undef p
}

#if 0
/* Kommandos und Variablen initialisieren *******************************/

static void NewVariable(char *s, void *l, unsigned n, unsigned t)
{
	Assign(Variable[ZeilenPos].name, s);
	Variable[ZeilenPos].loc = l;
	Variable[ZeilenPos].number = n;
	Variable[ZeilenPos].type = t;
	ZeilenPos++;
}
#endif

/************************************************************************/

#if 0
int main(void)
{
/*
	InitWorkstation("DIALOG");
	DiaPath = "";
	if (LoadDialog(111, FALSE))
		if (Tokenize(DialogBuffer))
			ExecuteDialog();
*/
}
#endif
