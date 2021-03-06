/* HASCSOutput module */
#include "compat.h"
#include "HASCSOutput.h" 

#include "HASCSGraphics.h"
#include "HASCSSystem.h"
#include "Sound.h"

/* Textausgabe f�r HASCS */

#define TextX 50 /* Koordinaten des Textausgabebereiches */
#define TextY 16
#define TextZeilen 8
#define TextSpalten 28

typedef char TextString[TextSpalten];
typedef const char *CharSet;

static void (*CharOut)(char);
static void (*StringOut)(char *);

static unsigned cx, cy; /* Cursorposition */

static TextString TextFenster[TextZeilen+1]; /* letzte = "" */
static int TextFensterAusgabe;

static char out[256], hilf[256];

static CharSet Cut;

static void BWOut(char c)
{
	unsigned /*pos, i,*/ n;
	if (TextMode == 0) {
		if (c != '\010') {
			if (cx < MaxX) {
				SetChar(cx, cy, c);
				cx++;
			}
		} else if (cx > 0)
			cx--;
	} else if (TextMode == 1) {
		c = CAP(c);
		if (c >= 'A' && c <= 'Z')
			n = 80 + c - 'A';
		else if (c == ' ')
			n = 0;
		else if (c == '\x8e' || c == '\x84')
			n = 106;
		else if (c == '\x99' || c == '\x94')
			n = 107;
		else if (c == '\x9a' || c == '\x81')
			n = 108;
		else if (c == '_')
			n = 1;
		else
			n = 0;
		if (c != '\010') {
			SetSprite(cx, cy, &SystemSprite[n]);
			cx++;
		} else
			cx--;
	}
}

static void BWStringOut(char *s)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (s[i] == '\0') return;
		CharOut(s[i]);
	}
}

void Cls()
{
	/* Bildschirm l�schen */
	unsigned x, y;
	cx = 0; cy = 0;
	for (x = 0; x <= 39; x++)
		for (y = 0; y <= 24; y++)
			SetSprite(x, y, &SystemSprite[0]);
}


void GotoXY(unsigned x, unsigned y)
/* Cursorpositionierung */
{
	cx = x; cy = y;
}


void GetXY(unsigned *ref_x, unsigned *ref_y)
/* Cursorposition holen */
{
#define x (*ref_x)
#define y (*ref_y)
	x = cx; y = cy;
#undef x
#undef y
}


static void PrintColors(int Chars, int Background)
/* Zeichen- und Hintergrundfarbe setzen */
{
}


static void CursorAn(void)
/* Cursor anschalten */
{
	CharOut('_'); CharOut('\010');
}


static void CursorAus(void)
/* Cursor abschalten */
{
	CharOut(' '); CharOut('\010');
}


void Print(char *s)
{
	unsigned i, l; int zulang;

	/* String ausgeben */

	void FindString(void) /* Textersatz durchf�hren */
	{
		unsigned i, n;
		i = 1; n = 0;
		while (s[i] <= '9' && s[i] >= '0') {
			n = n * 10 + s[i] - '0';
			i++;
		}
		for (i = 1; i <= AnzahlTexte; i++)
			if (n == Text[i].Nummer) {
				Assign(out, Text[i].String);
				if (Text[i].Sample != 0)
					PlaySoundN(Text[i].Sample);
				return;
			}
		Assign(out, s);
	}

	if (s[0] == '#')
		FindString();
	else
		Assign(out, s);

	if (TextFensterAusgabe) { /* mit Wortumbruch */
		do {
			l = Length(TextFenster[TextZeilen-1]);
			zulang = l + Length(out) > TextSpalten;
			if (zulang) {
				i = TextSpalten - l;
				while (i > 0 && !INSet(out[i],Cut))
					i--;
				if (i == 0 && l == 0) i = TextSpalten - l;
				Split(out, hilf, out, i);
			}
			Concat(TextFenster[TextZeilen-1], TextFenster[TextZeilen-1], out);
			StringOut(out);
			if (zulang) {
				EndOutput(); BeginOutput();
				Assign(out, hilf);
			}
		} while (zulang);
	} else
		StringOut(out);

}


void PrintAt(unsigned x, unsigned y, char *s)
{
	GotoXY(x,y); Print(s);
}


void CardToString(unsigned c, unsigned l, char *s)
{
	unsigned i, k, j;
	for (i = 0; i <= l-1; i++)
		s[i] = ' ';
	k = l-1;
	j = c;
	do {
		k++;
		l = k;
		c = j;
		do {
			l--;
			s[l] = c % 10 + '0';
			c = c / 10;
		} while (c != 0 && l != 0);
	} while (c != 0);
	s[k] = '\0';
}


unsigned StringToCard(char *s)
{
	unsigned i, c;
	i = 0;
	c = 0;
	while (s[i] >= '0' && s[i] <= '9') {
		c = 10 * c + (s[i] - '0');
		i++;
	}
	return c;
}


/* CARDINAL Zahl mit der L�nge l ausgeben */
void PrintCard(unsigned c, unsigned l)
{
	char s[21];
	CardToString(c, l, s); Print(s);
}


void PrintLongCard(unsigned long x, unsigned z)
{
	unsigned i, n, c;
	if (x < 65535)
		PrintCard(x, z);
	else {
		PrintCard(x / 10000, z-4);
		c = x % 10000;
		n = 0;
		while (c > 0) {
			n++;
			c = c / 10;
		}
		for (i = 1; i <= 4-n; i++)
			Print("0");
		PrintCard(x % 10000, 1);
	}
}


void InputString(char *s, unsigned l)
{
	/* String einlesen mit maximal l Zeichen */
	unsigned i, x, y; char ch; BITSET b;

	i = 0;
	while (s[i] != '\0' && i < l) {
		CharOut(s[i]);
		i++;
	}
	CharOut('_');
	CharOut('\010');
	do {
		WaitInput(&x, &y, &b, &ch, -1);
		if (ch >= 32 && i < l) {
			CharOut(ch); s[i] = ch; i++;
			CharOut('_');
			CharOut('\010');
		} else if (ch == '\010' && i > 0) { /* Backspace */
			CharOut(' '); CharOut('\010'); CharOut('\010');
			CharOut('_'); CharOut('\010');
			i--;
		} else if (ch == '\033') /* Escape */
			while (i > 0) {
				CharOut(' '); CharOut('\010'); CharOut('\010');
				CharOut('_'); CharOut('\010');
				i--;
			}
	} while (ch != '\015' && MausLinks & ~b); /* CR */
	CharOut(' '); CharOut('\010');
	s[i] = '\0';
}


void InputClick(char *s, unsigned l,unsigned *ref_x, unsigned *ref_y, BITSET *ref_b)
{
#define x (*ref_x)
#define y (*ref_y)
#define b (*ref_b)
	/* String einlesen mit maximal l Zeichen */
	unsigned i; char ch;

	i = 0;
	while (s[i] != '\0' && i < l) {
		CharOut(s[i]);
		i++;
	}
	CharOut('_');
	CharOut('\010');
	do {
		WaitInput(&x, &y, &b, &ch, -1);
		if (ch >= 32 && i < l) {
			CharOut(ch); s[i++] = ch;
			CharOut('_');
			CharOut('\010');
		} else if (ch == '\010' && i > 0) { /* Backspace */
			CharOut(' '); CharOut('\010'); CharOut('\010');
			CharOut('_'); CharOut('\010');
			i--;
		} else if (ch == '\033') /* Escape */
			while (i > 0) {
				CharOut(' '); CharOut('\010'); CharOut('\010');
				CharOut('_'); CharOut('\010');
				i--;
			}
	} while (ch != '\015' && b == 0); /* CR */
	CharOut(' '); CharOut('\010');
	s[i] = '\0';
#undef x
#undef y
#undef z
}

void InputCard(unsigned *ref_c, unsigned l)
{
#define c (*ref_c)
	char s[21];
	CardToString(c, 1, s);
	InputString(s, l);
	c = StringToCard(s);
#undef c
}

/* Textfenster Routinen *********************************************/

void BeginOutput(void)
{
	/* Bereitet Ausgabe im Textfenster vor */
	unsigned /*i,*/ j;
	for (j = 0; j <= TextZeilen - 1; j++) /* Textbereich hochschieben */
		Assign(TextFenster[j], TextFenster[j+1]);
	ScrollUp(TextX / 2, TextY, TextSpalten / 2, TextZeilen);
	GotoXY(TextX, TextY + TextZeilen - 1); /* Cursor auf Anfang letzte Zeile */
	TextFensterAusgabe = TRUE;
}

void EndOutput(void)
/* Schlie�t Ausgabe im Textfenster ab */
{
	TextFensterAusgabe = FALSE;
}

void OutputText(char *s)
{
	BeginOutput();
	Print(s);
	EndOutput();
}

void PrintOutput()
{
	unsigned i, j;
	for (j = 0; j <= TextZeilen - 1; j++) {/* Textbereich l�schen */
		for (i = 0; i <= TextSpalten / 2 - 1; i++)
			SetSprite(TextX / 2 + i, TextY + j, &SystemSprite[0]);
		PrintAt(TextX, TextY + j, TextFenster[j]);
	}
}


/* Sonstiges und Stringfunktionen ***********************************/

int Compare(char *s, char *p)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (i > HIGH(p)) return s[i] == '\0';
		if (s[i] != p[i]) return FALSE;
		if (s[i] == '\0') return TRUE;
	}
	if (HIGH(s) == HIGH(p)) return TRUE;
	return p[HIGH(s)+1] == '\0';
}

int COMPARE(char *s, char *p)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (i > HIGH(p)) return s[i] == '\0';
		if (CAP(s[i]) != CAP(p[i])) return FALSE;
		if (s[i] == '\0') return TRUE;
	}
	if (HIGH(s) == HIGH(p)) return TRUE;
	return p[HIGH(s)+1] == '\0';
}

int SMALLER(char *s, char *p)
{
	/* Liefert s < p */
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (i > HIGH(p))
			return FALSE;
		else if (cap(s[i]) != cap(p[i]))
			return cap(s[i]) < cap(p[i]);
		else if (s[i] == '\0')
			return FALSE;
	}
	return TRUE;
}

unsigned Length(char *s)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++)
		if (s[i] == '\0') return i;
	return HIGH(s)+1;
}

void Assign(char *s, char *p)
{
#if 1
	char buf_p[HIGH(buf_p)]; strcpy(buf_p,p);p=buf_p;
#endif
	unsigned i;

	for (i = 0; i <= HIGH(s); i++) {
		if (i > HIGH(p)) {
			s[i] = '\0';
			return;
		}
		s[i] = p[i];
		if (s[i] == '\0')
			return;
	}
}

void Concat(char *s, char *p, char *r)
{
#if 1
	char buf_r[HIGH(buf_r)], buf_p[HIGH(buf_p)];
	strcpy(buf_r,r);r=buf_r;strcpy(buf_p,p);p=buf_p;
#endif
	unsigned i, j;
	Assign(s, p);
	i = Length(s);
	for (j = 0; j <= HIGH(r); j++) {
		if (i > HIGH(s))
			return;
		s[i] = r[j];
		if (s[i] == '\0')
			return;
		i++;
	}
	if (i <= HIGH(s))
		s[i] = '\0';
}

void Split(char *p, char *r, char *s, unsigned i)
{
#if 1
	char buf_s[HIGH(buf_s)];strcpy(buf_s,s);s=buf_s;
#endif
	unsigned j;
	for (j = 0; j <= HIGH(p); j++)
		if (j < i && j <= HIGH(s)) {
			p[j] = s[j];
#if 1
			if (p[j] == '\0')
				break;
#endif
	 	} else {
			p[j] = '\0';
#if 1
			break;
#endif
		}
	for (j = 0; j <= HIGH(r); j++)
		if (j + i <= HIGH(s))  {
			r[j] = s[j+i];
#if 1
			if (r[j] == '\0')
				break;
#endif
		} else {
			r[j] = '\0';
#if 1
			break;
#endif
		}
}

unsigned FindC(char *s, char c)
{
	unsigned i;
	for (i = 0; i <= HIGH(s); i++) {
		if (s[i] == c)
			return i;
		if (s[i] == '\0')
			return 0;
	}
	return 0;
}

int InString(char *s1, char *s2)
{
	/* s1 in s2 */
	unsigned i, j;
	if (Compare(s1, "*"))
		return !Compare(s2, "");
	i = 0; j = 0;
	if (s1[0] == '\0') return TRUE;
	while (s2[i] != '\0') {
		if (cap(s2[i]) == cap(s1[j])) {
			j++; i++;
			if (s1[j] == '\0')
				return TRUE;
		} else if (j != 0)
			j = 0;
		else {
			i++; j = 0;
		}
	}
	return FALSE;
}

char cap(char ch)
{
	if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 'A';
	else if (ch == '\x84') return '\x8e';
	else if (ch == '\x81') return '\x9a';
	else if (ch == '\x94') return '\x99';
	else return ch;
}

int StringToInt(char *s)
{
	unsigned i;
	int f, z;
	z = 0;
	f = 1;
	for (i = 0; i <= HIGH(s); i++)
		if (s[i] == '-' && f == 1) /* ein Minuszeichen */
			f = -1;
		else if (s[i] >= '0' && s[i] <= '9')
			z = z * 10 + (int)(s[i] - '0');
		else if (s[i] != ' ')
			return z * f;
	return z * f;
}

void AssignC(char *s, char c)
{
	s[0] = c;
	if (HIGH(s) >= 1)
		s[1] = '\0';
}

static void __attribute__ ((constructor)) at_init(void)
{ /* Initialisierung */
	TextFensterAusgabe = FALSE;
	*TextFenster[TextZeilen] = *"";
	TextMode = 0;
	AnzahlTexte = 0;
	GotoXY(0, 0);
	CharOut = BWOut;
	StringOut = BWStringOut;
	PrintChar = BWOut;
	Cut = "' :.,\\/";
	MaxX = 80;
}
