/* Dialog Module */
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

#include "Dialog.h"


#define MaxCommand (1 + sizeof Command / sizeof *Command) /* Kommandos */
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
typedef unsigned CharSet[(1 << CHAR_BIT) / sizeof (unsigned)];
typedef char *CharPtr;

typedef (*ActionProc)(CharPtr);
typedef struct {
	char name[21]
	ActionProc action;
} CommandType;
typedef struct {
	char name[3];
	unsigned operation;
} OperatorType;
typedef struct {
	char name[21]
	void *loc;
	unsigned number;
	unsigned type;
} VariableType;
typedef struct {
	unsigned LabelNumber;
	void *LabelAddress;
} LabelType;
                    
CommandType Command[] = {
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
	{"COPY", Copy},
	{"AIM", Aim},
	{"TELEPORT", Teleport},
	{"DELETE", Delete},
	{"SOUND", Sound},
	{"INVERT", Invert},
	{"OUTPUT", Output},
	{"GOSUB", Gosub},
	{"RETURN", Return},
	{"CALL", Call},
};

OperatorType Operator[] = {
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


VariableType Variable[] = {
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
};

unsigned fx, fy, fw, fh, i; /* Fensterausmaße in Sprites */
unsigned ZeilenAus, ZeilenPos = 1, Redraw,
    DialogBreite, DialogHoehe, DialogNummer = 9999,
    x, y, w, h;
unsigned long DialogLaenge;
CharPtr DialogBuffer; /* Dialogpuffer */
int Continue, OpenWindow; /* Dialogausführung */
String80Type ein;
unsigned long DebugLevel;
CharSet VarSet = "abcdefghijklmnopqrstuvwxyz"
		 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		 ".0123456789";
	OpSet = ":=+-|&<>*/~#";
	NumberSet = "0123456789";

MonsterType *SelectedMonster = NULL;
GegenstandTyp *SelectedGegenstand = NULL;
ParameterTyp *SelectedParameter = NULL;

LabelType ReturnAddress[MaxGosub+1]; /* GOSUB Stack */
unsigned ReturnLevel;

LabelType Labels[MaxLabel+1]; /* Label Adressen */
unsigned LabelAnzahl;
    
String80Type DialogText[MaxZeilen + 1]; /* Texte für Hyperclick */

unsigned LocalVar[26];
char LocalVarName[2];
    
/* Forward Deklarationen ************************************************/

unsigned GetNumber(CharPtr *p);

void *GetItem(unsigned n, void **p, unsigned *r; char **s);
unsigned EvalTerm(CharPtr *p, unsigned n);
void EvalString(CharPtr *p, char **s);
void NewLabel(unsigned l, CharPtr p);

/* Hilfprozeduren *******************************************************/

void DialogFehler(char *s, *q, unsigned c)
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

unsigned Max(unsigned x, unsigned y)
{
	if (x > y) return x else return y;
}

unsigned Min(unsigned x, unsigned y)
{
	if (x < y) return x else return y;
}

unsigned GetToken(CharPtr *ref_p, unsigned *ref_n, char *s)
{
	CharPtr p = *ref_p;
	unsigned n = *ref_n

	unsigned i, t;

	t = *p;
	switch (t) {
	case OperatorToken:
	case VariableToken:
	case CommandToken:
		*p++;
		n = *p++;
		break;
	case ByteToken:
		*p++;
		n = *p++;
		t = NumberToken;
		break;
	case NumberToken:
		*p++;
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
	*ref_p = p, *ref_n = n;
	return t;
}

int NextLine(CharPtr *ref_p)
{
	CharPtr *p = *ref_p;
	unsigned t, i;
	String80Type s;

	do {
		t = GetToken(&p, i, s);
		if (t == TextEndeToken) return FALSE;
	while (t != ZeilenEndeToken);
	p++;
	*ref_p = p;
	return *p != TextEndeToken;
}


int Tokenize(CharPtr p, CharPtr q, unsigned long *ref_l)
{
	unsigned long l = *ref_l;
	unsigned t, n, i, Line;
	char s[80];
	int ok = TRUE, label = FALSE, ende;
	CharPtr start;

	static int LastLine(CharPtr *ref_p)
	{
		CharPtr p = *ref_p;
		while (*p >= ' ') p++;
		while (*p != '\0' && *p != 10) p++;
		if (*p == 10) p++;
		Line++;
		*ref_p = p;
		return *p == '\0';
	}

	static void GetLine(CharPtr *ref_p, char *s, size_t n)
	{
		CharPtr p = *ref_p;
		unsigned i;
		for (i = 0; i <= n; i++) {
			if (*p < ' ') { s[i] = '\0'; break; }
			s[i] = *p++;
		}
		*ref_p = p;
	}

	unsigned GetToken(CharPtr *ref_p, char *s)
	{
		CharPtr p = ref_p;
		unsigned i = 0, t = 0;
		while (*p == ' ' || *p = 8) p++;
		if (*p IN OpSet) { /* Operator */
			t = OperatorToken;
			while (*p IN OpSet)
				s[i++] = *p++;
		} else if (*p == '"') { /* String */
			t = StringToken;
			p++;
			while (*p >= " " && *p != '"')
				s[i++] = *p++;
			if (*p == '"') p++;
		} else if (*p IN NumberSet) {
			t = NumberToken;
			while (*p IN NumberSet)
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
		} else if (*p IN VarSet) {
			t:= VariableToken;
			while (*p IN VarSet)
				s[i++] = *p++;
		}
		s[i] = '\0';
		*ref_p = p;
		return t;
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
			if (COMPARE(Command[i].name, s)) {
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
			while (t != 0)
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
						NewLabel(n);
					}
				case StringToken:
					n = LENGTH(s);
					Out(t); Out(n);
					if (n > 0)
						for (i = 0; i <= n - 1; i++)
							OutC(s[i]);
				}
				t = GetToken(p, s);
			}
			ende = LastLine(p);
			if (!ende) Out(ZeilenEndeToken); /* LF */
		} else if (*p !=  '*') /* Ausgabezeile */
			GetLine(p, s);
			n = LENGTH(s);
			Out(AusgabeToken);
			if (n > 0)
				for (i = 0; i <= n - 1 ; i++)
					OutC(s[i]);
			
			if (n > DialogBreite) DialogBreite = n;
			DialogHoehe++;
			ende = LastLine(p);
			if (!ende) Out(ZeilenEndeToken); /* LF */
		} else /* Kommentar */
			ende = LastLine(p);
	} while (!ende && ok);
	Out(0); /* Endekennzeichnung */
	l = q - start;
	return ok;
}


/************************************************************************/

void *GetLabel(unsigned l)
{
	unsigned i = 1;
	while (i <= LabelAnzahl) {
		if (Labels[i].LabelNumber == l)
			return Labels[i].LabelAddress;
		i++;
	}
	return NULL;
}

void NewLabel(unsigned l, CharPtr p)
{
	if (LabelAnzahl < MaxLabel) {
		LabelAnzahl++;
		Labels[LabelAnzahl].LabelNumber = l;
		Labels[LabelAnzahl].LabelAddress = p;
	}
}

int FindLabel(CharPtr *p; unsigned l)
{
	unsigned cmd, t; String80Type s;

	*p = GetLabel(l); /* Schon in Liste? */
	if (*p) return TRUE;
	*p = DialogBuffer;
	do {
		t = GetToken(p, cmd, s);
		if (t == CommandToken && cmd == 4) /* Label */
			if (GetNumber(p) == l) {/* Label gefunden */
				NewLabel(l, p);
				return TRUE;
			}
	} while (NextLine(p));
	DialogFehler("Label nicht gefunden ", "", l);
	return FALSE;
}

/************************************************************************/

void PrinterLine(char *s, size_t n)
{
	unsigned i;
	if (!PrinterStatus())
		WaitTime(1000);

	for (i = 0; i <= n ; i++) {
		if (s[i] == '\0') {
			PrinterOut(13); PrinterOut(10);
			return;
		}
		PrinterOut(s[i]);
	}
	PrinterOut(13); PrinterOut(10);
}


/* Variablenbearbeitung *************************************************/

void GetVariable(CharPtr *p, unsigned *c, char *s)
{
	unsigned type, i, r;
	char q[80];
	CharPtr v;

	*c = 0;
	
	switch ((type = GetToken(p, i, q)))
	case VariableToken:
		v = GetItem(i, p, r, q);
		*c = r;
		Assign(s, q);
		return Variable[i].type;
	case KlammerAufToken: /* ( */
		switch ((type = GetVariable(p, i, q))) {
		case StringToken:
			EvalString(p, q);
			Assign(s, q);
			break;
		case NumberToken:
			*c = EvalTerm(p, i);
		}
		break;
	case StringToken:
		Assign(s, q);
		break;
	case NumberToken:
		*c = i;
		/* FALLTHROUGH */
	case ZeilenEndeToken:
		break;
	default:
		DialogFehler("Fehlerhafter Ausdruck!", "", 65535);
	}
	return type;
}

unsigned GetNumber(CharPtr *p)
{
	unsigned c, t;
	Strin80Type s;

	t = GetVariable(p, c, s);
	if (t == StringToken) {
		c = StringToCard(s);
		t = NumberToken;
	}
	return t == NumberToken ? EvalTerm(p, c) : 0;
}

unsigned GetArgument(CharPtr *p)
{
	Strin80Type s;
	unsigned n;
	if (GetToken(p, n, s) == KlammerAufToken)
		return GetNumber(p);
	DialogFehler("Klammer auf '(' erwartet", "", 65535);
	Continue = FALSE;
	return 0;
}

void GetString(CharPtr *p, char *s)
{
	unsigned c;
	if (GetVariable(p, c, s) == NumberToken)
		CardToString(c, 1, s);
	EvalString(p, s);
}

void *GetAddress(CharPtr *p; unsigned *t, unsigned *c, char *s)
{
	void *v;
	unsigned i, k;
	char q[80];
	
	t = GetToken(p, i, s);
	if (t = VariableToken)
		v = GetItem(i, p, c, s);
	if (v != NULL) {
		t = GetToken(p, k, q); /* Komma überlesen */
		t = Variable[i].type;
		return v;
	}
	DialogFehler("Variable erwartet ", s, 65535);
	Continue = FALSE;
	return NULL;
}

unsigned FindText(unsigned n)
{
	unsigned i;

	for (i = 1; i <= AnzahlTexte; i++)
		if (Text[i].Nummer == n)
			return i;
	AnzahlTexte++
	Text[AnzahlTexte].Nummer = n;
	return AnzahlTexte;
}

void *GetItem(unsigned n, void **p, unsigned *r, char **s);
{
	unsigned x, y; String80Type *str; CardPtr v;
	v = NULL;
	r = 0;
	if  (Variable[n].loc != NULL)
		v = Variable[n].loc;
	else if (Variable[n].number < 10) {
		if (SelectedMonster) /* Monster */
			switch (Variable[n].number) {
			case 1: v = &SelectedMonster->Status; break;
			case 2: v = &SelectedMonster->Sprich; break;
			case 3: v = &SelectedMonster->TP; break;
			case 4: v = &SelectedMonster->x; break;
			case 5: v = &SelectedMonster->y; break;
			case 6: v = &SelectedMonster->Spezial; break;
			case 7: v = &SelectedMonster->Typ; break;
			case 8: v = &SelectedMonster->Name; break;
			}
	} else if (Variable[n].number < 20) {
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
			case 19 : v = S&electedGegenstand->Name; break;
			}
	} else if (Variable[n].number < 30) {
		if (SelectedParameter)
			switch (Variable[n].number) {
			case 21 : v = &SelectedParameter->Art; break;
			case 22 : v = &SelectedParameter->xhoch; break;
			case 23 : v = &SelectedParameter->yhoch; break;
			case 24 : v = &SelectedParameter->Levelhoch; break;
			case 25 : v = &SelectedParameter->xrunter; break;
			case 26 : v = &SelectedParameter->yrunter; break;
			case 27 : v = &SelectedParameter->Levelrunter; break;
		}

	} else if (Variable[n].number >= 60000) {
		r = Variable[n].number;
	} else {
		switch (Variable[n].number) {
		case 31:
			x = GetArgument(p);
			if (x < MaxSprites)
				v = &Felder[x].Name;
			break;
		case 32:
			x = GetArgument(p);
			if (x < MaxSprites)
				v = &Felder[x].Spezial;
			break;
		case 33:
			x = GetArgument(p);
			v = &Text[FindText(x)].String;
			break;
		case 34:
			x = GetArgument(p);
			v = &Text[FindText(x)].Sample;
			break;
		case 50:
			x = GetArgument(p);
			if (x >= 1 && x <= MaxFlags)
				v = &Spieler.Flags[x];
			break;
		case 51:
			r = Zufall(GetArgument(p));
			break;
		case 52:
			x = GetArgument(p);
			y = GetNumber(p);
			v = &Level[x,y].Feld;
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
			str = ADDRESS(v);
			Assign(s, *str);
		}
	}
	return v;
}

/* Zahlausdruck auswerten ***********************************************/

unsigned EvalTerm(CharPtr *p, unsigned n)
{
	unsigned v, op; String80Type h;

	while (GetToken(p, op, h) == OperatorToken) {
		if (GetVariable(p, v, h) == StringToken)
			v = StringToCard(h);

		switch (op) {
		case  1 : n = v; break; /* Zuweisung */
		case  2 : n += v; break; /* Addition */
		case  3 : n = n > v ? n - v : 0; break; /* Subtraktion */
		case  4 : n = n | v; break; /* Vereinigungsmenge */
		case  5 : n = n & v; break; /* Schnittmenge */
		case  6 : n = n == v; break; /* Gleichheit */
		case  7 : n = n < v; break;
		case  8 : n = n > v; break;
		case  9 : n = n != v; break;
		case 10 : n = n * v; break; /* Multiplikation */
		case 11 : n = n / v; break; /* Division */
		case 12 : n = n <= v break;
		case 13 : n = n >= v; break;
		case 14 : n = n & ~v; break;/* Mengendifferenz */
		}
	}
	return n;
}

/* Zeichenkettenausdruck auswerten **************************************/

void EvalString(CharPtr *p, char *s)
{
	unsigned t, op, c; String80Type h;

	while (GetToken(p, op, h) == OperatorToken) {
		if (GetVariable(p, c, h) == NumberToken)
			CardToString(c, 1, h);

		switch (op) {
		case  1: Assign(s, h); break; /* = */
		case  2: Concat(s, s, h); break; /* sp */
		case  4: Assign(s, COMPARE(s, "1") || COMPARE(h, "1")
				? "1" : "0"); break;
		case  5: Assign(s, COMPARE(s, "1") && COMPARE(h, "1")
				? "1" : "0"); break;
		case  6: Assign(s, COMPARE(s, h) ? "1" : "0"); break;
		case  9: Assign(s, !COMPARE(s, h) ? "1" : "0"); break;
		case 12: Assign(s, InString(s, h) ? "1" : "0"); break;
		case 13: Assign(s, InString(h, s) ? "1" : "0"); break;
		default
			DialogFehler("Falscher Operator in Stringausdruck!", "", 65535);
			break;
		}
	}
}

/* Textzeile aus-/eingeben **********************************************/

void OpenScreen(int MinW, int MinH)
{
	unsigned i;
	if  (w == 0 || h == 0) {/* Breite defaultmäßig ermitteln */
		w = Max(Min(DialogBreite, 76), Min(80, MinW));
		h = Max(Min(DialogHoehe, 23), Min(25, MinH));
	}
	w = Min(80, w); h = Min(25, h);
	if (x == 0 || y = 0
		|| x + w > 79 ||  y + h > 24) /* zentrieren */
	{
		x = (40 - w / 2) / 2 * 2;
		y = (12 - h / 2);
	}
	fw = (w + 1) / 2; fh = h; fx = x / 2; fy = y;
	if (fx >= 1 && fy >= 1 && fx+fw < 40 & fy+fh < 25)
		ReserveScreen(fx - 1, fy - 1, fx + fw, fy + fh);
	else
		FillRectangle(0, 0, 39, 24, SystemSprite[0]);
	
	ZeilenAus = 0;
	ZeilenPos = 0;
	for (i = 1; i <= MaxZeilen; i++) DialogText[i] = "";
	OpenWindow = FALSE;
}

void Ausgabe(char *s)
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

void Label(CharPtr *p)
{
	unsigned label;

	label = GetNumber(p);
	NewLabel(label, p);
}

void Goto(CharPtr *p)
{
	unsigned label;

	label = GetNumber(p);
	Continue = FindLabel(p, label);
}

void Gosub(CharPtr *p)
{
	unsigned label;
	
	label = GetNumber(p);
	if (ReturnLevel < MaxGosub) {
		ReturnLevel++
		ReturnAddress[ReturnLevel].LabelAddress = p;
		Continue = FindLabel(p, label);
	} else {
		DialogFehler("Zuviele verschachtelte GOSUB", "", 65535);
		Continue = FALSE;
	}
}

void Return(CharPtr *p)
{
	if (ReturnLevel > 0)
		p = ReturnAddress[--ReturnLevel].LabelAddress;
	else {
		DialogFehler("RETURN ohne GOSUB", "", 65535);
		Continue = FALSE;
	}
}

void End(CharPtr *p)
{
	Continue = FALSE;
}

void Input(CharPtr *p)
{
	unsigned i, mx, my;
	BITSET mb;

	void GetWord(unsigned mx, unsigned my, char *s)
	{
		/* Ermittelt das Dialogwort an Bildschirmposition mx, my */
		unsigned i;

		my = my / 16; mx = mx / 8;
		if (my < y || my >= y + h
			|| mx < x OR mx >= x + w)
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

	ein = "";
	if (ZeilenPos < h)
		ZeilenPos++;
	else {
		ScrollUp(fx, fy, fw, fh);
		for (i = 2; i <= ZeilenPos; i++)
			Assign(DialogText[i-1], DialogText[i]);
		
		DialogText[ZeilenPos] = "";
	}
	GotoXY(x, y + ZeilenPos - 1);
	InputClick(ein, w - 1, mx, my, mb);
	if (mb) {
		GetWord(mx, my, ein);
		PrintAt(x, y + ZeilenPos - 1, ein);
	}
	Assign(DialogText[ZeilenPos], ein);
	ZeilenAus = 0;
	if (DruckerAusgabe)
		PrinterLine(ein);
}

void Aim(CharPtr *p)
{
	unsigned i, x, y, qx, qy, mode, type;
	BITSET b : BITSET;
	char ch;
	CardPtr varx, vary;
	char s[80];
	
	varx = GetAddress(p, type, x, s);
	vary = GetAddress(p, type, y, s);
	mode = GetNumber(p);
	if (ZeilenAus > 0) {
		WaitKey(); ZeilenAus = 0;
	}
	if (!OpenWindow) { /* Dialogfenster löschen */
		RestoreScreen();
		OpenWindow = TRUE;
	}
	WaitInput(x, y, b, ch, -1);
	x = x / 16 - 1; y = y / 16 - 1;
	if  (x >= 0 && y >= 0
	  && x <= 22 && y <= 22 && MausLinks IN b)
	{
		if (mode = 1) {
			qx = MaxSichtweite; qy = MaxSichtweite;
			if (MakeShoot(qx, qy, x, y, 30, TRUE)) {/* getroffen */
				x = qx; y = qy;
			}
		}
		SichtLevelUmrechnung(x, y, x, y);
	} else {
		x = Spieler.x; y = Spieler.y;
	}
	*varx = x; *vary = y;
}


void Nothing(CharPtr *p)
{
}

void Window(CharPtr *p)
{
	if (!OpenWindow) {
		RestoreScreen();
		OpenWindow = TRUE;
	}
	x = GetNumber(p);
	y = GetNumber(p);
	w = GetNumber(p);
	h = GetNumber(p);
}

void Output(CharPtr *p)
{
	String80Type s;

	if (ZeilenAus > 0) {
		WaitKey(); ZeilenAus = 0;
	}
	if (!OpenWindow) {/* Dialogfenster löschen */
		RestoreScreen();
		OpenWindow = TRUE;
	}
	GetString(p, s);
	OutputText(s);
}

void Invert(CharPtr *p)
{
	unsigned x, y;

	x = GetNumber(p);
	y = GetNumber(p);
	if (LevelSichtUmrechnung(x, y, x, y))
		InvertFeld(x+1, y+1);
}

void Picture(CharPtr *p)
{
	unsigned n;
	n = GetNumber(p); /* Bildnummer */
	ShowPicture(n, FALSE);
}

void ShowPicture(unsigned n, int New)
{
	unsigned ys, ws, hs, i, PicW, PicH;
	if (!LoadImageN(n, PicW, PicH))
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
		DialogText[ZeilenPos] = "";
		hs = Min(PicH - ys, 16);
		HASCSSystem.Copy(4, 0, ys, ws, hs,
			8 * x, 16 * (ZeilenPos + y - 1));
		ZeilenAus++;
		ys += 16;
	while (ys < PicH);
	if (New) {
		if (ZeilenAus > 0)
			WaitKey();
		RestoreScreen();
	}
}


void Let(CharPtr *p, unsigned i)
{
	CardPtr var;
	unsgiend type, value;
	String80Type s;
	StringPtr str;

	var = GetItem(i, p, value, s);
	if (var)
		switch (Variable[i].type) {
		case NumberToken:
			*var = EvalTerm(p, value);
			break;
		case StringToken:
			str = ADDRESS(var); EvalString(p, s); Assign(*str, s);
			break;
		}
}


void If(CharPtr *p)
{
	unsigned cmd, c, type;
	String80Type h;

	type = GetVariable(p, c, h);
	if (type == NumberToken) {
		if (EvalTerm(p, c) = 0) return;
	} else if (type == StringToken) {
		EvalString(p, h);
		if (!COMPARE(h, "1")) return;
	}
	type = GetToken(p, cmd, h);
	if (type == VariableToken)
		Let(p, cmd);
	else if (type == CommandToken)
		Command[cmd].action(p);
}


void Sound(CharPtr *p)
{
	unsigned n, mode;
	SoundType snd;
	String80Type s;

	n = GetNumber(p);
	mode = GetNumber(p);
	switch (mode) {
	case 1: if (LoadSound(n, snd)); break;
	case 2: LoopSoundN(n); break;
	default:
		PlaySoundN(n); break;
	}
}

void Wait(CharPtr *p)
{
	unsigned t;

	t = GetNumber(p);
	if (t = 0) {
		WaitKey();
		ZeilenAus = 0;
	} else {
		WaitTime(t * 100);
	}
}

void Select(CharPtr *p)
{
	unsigned x, y, i;

	x = GetNumber(p);
	if (x <= MaxBreite) { /* Level */
		y = GetNumber(p);
		i = GetNumber(p);
		if (i == 60006) {
			SelectedMonster =
				LevelMonster IN Level[x,y].Spezial
				? &Monster[FindMonster(x,y)]
				: NULL;
		} else if (i == 60007) {
			SelectedGegenstand = 
				LevelGegenstand IN Level[x,y].Spezial
				? &Gegenstand[FindGegenstand(x,y)]/* im Level */
				: NULL;
		} else if (i == 60008) {
			SelectedParameter =
				LevelParameter IN Level[x,y].Spezial
				? &Parameter[FindParameter(x,y)]
				: NULL;
		} else 
			x = i;
	}
	if (x < 60000) return
	switch (x - 60000) {
	case 0: SelectedGegenstand = (i = GetNumber(p)) >= 1 && i <= MaxRuck
		? &Spieler.Rucksack[i] : NULL; break;
	case 1: SelectedGegenstand = &Spieler.rechteHand; break;
	case 2: SelectedGegenstand = &Spieler.linkeHand; break;
	case 3: SelectedGegenstand = &Spieler.Ruestung; break;
	case 4: SelectedGegenstand = &Spieler.Ring; break;
	case 5:	SelectedMonster = SReitet IN Spieler.Status
		? &Spieler.ReitTier : NULL; break;
	case 6:	SelectedMonster = (i = GetNumber(p)) >= 1 
		/* ( schon durch letztes GetNumber */
		&& i <= AnzahlMonster ? &Monster[i] : NULL; break;
	case 7: SelectedGegenstand = (i = GetNumber(p)) >= 1 
		&& i <= AnzahlGegen ? &Gegenstand[i] : NULL; break;
	case 8: SelectedParameter = (i = GetNumber(p)) >= 1
		&& i <= AnzahlParameter ? &Parameter[i] : NULL; break;
	}
}

void Copy(CharPtr *p)
{
	unsigned i, x, y;

	x = GetNumber(p);
	if (x <= MaxBreite) {/* Level */
		y = GetNumber(p);
		i = GetNumber(p);
		if (i == 60006 && SelectedMonster)
			NewMonster(x, y, *SelectedMonster);
		else if (i == 60007 && SelectedGegenstand)
			NewGegenstand(x, y, *SelectedGegenstand);
		else if (i == 60008 && SelectedParameter)
			NewParameter(x, y, *SelectedParameter);
		else
			x = i; /* vielleicht doch für den Spieler? */

	}
	if (x == 60000) { /* Rucksack */
		i = GetNumber(p);
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
}

void Delete(CharPtr *p)
{
	unsigned x, y, i;

	x = GetNumber(p);

	if (x <= MaxBreite) { /* Level */
		y = GetNumber(p);
		i = GetNumber(p);
		if (i == 60006) DeleteMonster(x, y);
		else if (i == 60007) DeleteGegenstand(x, y);
		else if (i == 60008) DeleteParameter(x, y);
		else
			x = i;
	}
	if (x == 60000) {/* Rucksack */
		i = GetNumber(p);
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
		else if (x == 60005)
			Spieler.ReitTier.Typ = 0; EXCL(Spieler.Status, SReitet);
	}
}


void Teleport(CharPtr *p)
{
	unsigned x, y, l;

	x = GetNumber(p);
	y = GetNumber(p);
	l = GetNumber(p);
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
}

void Call(CharPtr *p)
{
}


/* Dialog ausführen *****************************************************/

void ExecuteDialog(void)
{
	unsigned cmd, type, token;
	String80Type s;
	CharPtr p;

	Continue = TRUE;
	OpenWindow = TRUE;
	x = 0; w = 0;
	Redraw = 0;
	ReturnLevel = 0; /* noch kein GOSUB */
	ZeilenAus = 0;
	DebugLevel = {};
	LabelAnzahl = 0;
	p = DialogBuffer; /* Start am Pufferanfang */

	while (Continue) {
		token = GetToken(p, cmd, s);
		if (DebugLevel != 0) {
			switch (token) {
			case VariableToken: Assign(s, Variable[cmd].name); break;
			case CommandToken: Assign(s, Command[cmd].name); break;
			}
			WaitTime(0);
			Error(s, 2);
			Continue = ErrorResult = 2;
		}
		switch (token) {
		case VariableToken: Let(p, cmd); break;
		case CommandToken: Command[cmd].action(p); break;
		case AusgabeToken: Ausgabe(s); break;/* Zeichenkette ausgeben */
		default:
			DialogFehler("Befehl erwartet", "", 65535);
			Continue = FALSE;
		}
		Continue = Continue && NextLine(p);
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

void MakeFileName(int c, unsigned n, char *s)
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
		Assign(s, HASCSSystem.Command);
	else if (n < 1000) {
		Assign(s, DiaPath);
		Concat(s, s, c ? "XDIALOG.000" : "DIALOG.000");
		i = LENGTH(s) - 3;
		s[i]   = n / 100 + '0';
		s[i+1] = n % 100 / 10 + '0';
		s[i+2] = n % 10 + '0';
	}
}

void CodeDialog(unsigned long n, unsigned long l, void *b)
{
	unsigned long i;
	BITSET *p = b;

	for (i = 1; i <= l DIV 2; i++) {
		n = (n * 153 + 97) % 16777216;
		*p = *p ^ (n % 65536); /* EXOR */
		p += 2;
	}
}

int LoadDialog(unsigned n, int coded)
{
	char s[128]
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
	if (!Tokenize(DialogBuffer, Cache[i].CacheBuffer, DialogLaenge)) {
		FreeCache(i);
		return FALSE
	} /* Syntaxfehler */
	Cache[i].CacheInfo1 = DialogBreite;
	Cache[i].CacheInfo2 = DialogHoehe;
	Cache[i].DialogBuffer = CacheBuffer;
	return TRUE;
}

int SaveDialog(unsigned n, int coded)
{
	String80Type s;
	int f;
	unsigned long l;
	BITSET *p;

	MakeFileName(!coded, n, s);
	l = FileLength(s); if (l = 0) return FALSE;;
	p = GetBuffer(l);
	f = OpenFile(s); ReadFile(f, l, p); CloseFile(f);
	CodeDialog(LONG(BenutzerNummer), l, p);
	MakeFileName(coded, n, s);
	f = CreateFile(s); WriteFile(f, l, p); CloseFile(f);
	return !FileError;
}


/* Dialog ausführen *****************************************************/

void DoDialog(unsinged n)
{
	unsigned i;
	if (!LoadDialog(n, FALSE)) /* unkodierter Dialog */
		if (!LoadDialog(n, TRUE)) { /* kodierter Dialog */
			DialogFehler("Dialog nicht geladen", "", 65535);
			return;
		}
	ExecuteDialog();
}

void DoMonsterDialog(unsinged n, MonsterTyp *m)
{
	SelectedMonster = m;
	DoDialog(n);
}

void DoGegenstandDialog(unsigned n, GegenstandTyp *g)
{
	SelectedGegenstand = g;
	DoDialog(n);
}

void DoParameterDialog(unsigned n, ParameterTyp *p)
{
	SelectedParameter = p;
	DoDialog(n);
}


/* Kommandos und Variablen initialisieren *******************************/

void NewVariable(char *s, void *l, unsigned n, unsigned t)
{
	Assign(Variable[ZeilenPos].name, s);
	Variable[ZeilenPos].loc = l;
	Variable[ZeilenPos].number = n;
	Variable[ZeilenPos].type = t;
	ZeilenPos++;
}

/************************************************************************/

void DialogInit()
{
/*
	InitWorkstation("DIALOG");
	DiaPath = "";
	if (LoadDialog(111, FALSE))
		if (Tokenize(DialogBuffer))
			ExecuteDialog();
*/
}

