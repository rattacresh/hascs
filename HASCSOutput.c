/* HASCSOutput module */
#include "HASCSOutput.h"

/* Textausgabe für HASCS */

#define TextX 50 /* Koordinaten des Textausgabebereiches */
#define TextY 16
#define TextZeilen 8
#define TextSpalten 28

typedef char TextString[TextSpalten];
typedef const char *CharSet;

void (*CharOut)(char);
void (*StringOut)(char *);

unsigned cx, cy; /* Cursorposition */

TextSTring TextFenster[TextZeilen+1]; /* letzte = "" */
int TextFensterAusgabe;

char out[256], hilf[256];

CharSet Cut;

void BWOut(char c)
{
	unsigned pos, i, n;
	if (TextMode == 0) {
		if (c != 10) {
			if (cx < MaxX) {
				SetChar(cx, cy, c);
				cx++;
			}
		} else if (cx > 0)
			cx--;
	} else if (TextMode == 1) {
		c = CAP(c);
		if (c >= "A" && c <= "Z")
			n = 80 + c - "A";
		else if (c == " ")
			n = 0;
		else if (c == "Ä" || c == "ä")
			n = 106;
		else if (c == "Ö" || c == "ö")
			n = 107;
		else if (c == "Ü" || c == "ü")
			n = 108;
		else if (c == "_")
			n = 1;
		else
			n = 0;
		if (c != 10) {
			SetSprite(cx, cy, SystemSprite[n]);
			cx++;
		} else
			cx--;
	}
}

void BWStringOut(char *s, size_t n)
{
	unsigned i;
	for (i = 0; i <= n; i++) {
		if (s[i] == '\0') return;
		CharOut(s[i]);
	}
}

void Cls()
{
	/* Bildschirm löschen */
	unsigned x, y;
	cx = 0; cy = 0;
	for (x = 0; x <= 39; x++)
		for (y = 0; y <= 24; y++)
			SetSprite(x, y, SystemSprite[0]);
}


void GotoXY(unsigned x, unsigned y)
/* Cursorpositionierung */
{
	cx = x; cy = y;
}


void GetXY(unsigned x, unsigned y)
/* Cursorposition holen */
{
	x = cx; y = cy;
}


void PrintColors(int Chars, int Background)
/* Zeichen- und Hintergrundfarbe setzen */
{
}


void CursorAn(void)
/* Cursor anschalten */
{
	CharOut("_"); CharOut(10);
}


void CursorAus(void)
/* Cursor abschalten */
{
	CharOut(" "); CharOut(10);
}


void Print(char *s)
{
	unsigned i, l; int zulang;

	/* String ausgeben */

	void FindString(void) /* Textersatz durchführen */
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
				while (i > 0 && NOT(out[i] IN Cut))
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


unsigned StringToCard(char *s);
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


/* CARDINAL Zahl mit der Länge l ausgeben */
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
	CharOut("_");
	CharOut(10);
	do {
		WaitInput(x, y, b, ch, -1);
		if (ch >= 32 && i < l) {
			CharOut(ch); s[i] = ch; i++;
			CharOut("_");
			CharOut(10);
		} else if (ch == 10 && i > 0) { /* Backspace */
			CharOut(" "); CharOut(10); CharOut(10);
			CharOut("_"); CharOut(10);
			i--;
		} else if ((ch == 33)) /* Escape */
			while (i > 0) {
				CharOut(" "); CharOut(10); CharOut(10);
				CharOut("_"); CharOut(10);
				i--;
			}
	} while (ch != 15C && MausLinks & ~b); /* CR */
	CharOut(" "); CharOut(10);
	s[i] = '\0';
}


void InputClick(char *s, unsigned l,unsigned *x, unsigned *y, BITSET *b)
{
	/* String einlesen mit maximal l Zeichen */
	unsigned i; char ch;

	i = 0;
	while (s[i] != '\0' && i < l) {
		CharOut(s[i]);
		i++;
	}
	CharOut("_");
	CharOut(10);
	do {
		WaitInput(x, y, b, ch, -1);
		if (ch >= 32 && i < l) {
			CharOut(ch); s[i] = ch; i++;
			CharOut("_");
			CharOut(10);
		} else if (ch == 10 && i > 0) { /* Backspace */
			CharOut(" "); CharOut(10); CharOut(10);
			CharOut("_"); CharOut(10);
			i--;
		} else if ((ch == 33)) /* Escape */
			while (i > 0) {
				CharOut(" "); CharOut(10); CharOut(10);
				CharOut("_"); CharOut(10);
				i--;
			}
	} while (ch != 15 && b == 0); /* CR */
	CharOut(" "); CharOut(10);
	s[i] = '\0';
}

void InputCard(unsigned *c, unsigned l)
{
	char s[21];
	CardToString(c, 1, s);
	InputString(s, l);
	c = StringToCard(s);
}

/* Textfenster Routinen *********************************************/

void BeginOutput(void)
{
	/* Bereitet Ausgabe im Textfenster vor */
	unsigned i, j;
	for (j = 0; j <= TextZeilen - 1; j++) /* Textbereich hochschieben */
		Assign(TextFenster[j], TextFenster[j+1]);
	ScrollUp(TextX / 2, TextY, TextSpalten / 2, TextZeilen);
	GotoXY(TextX, TextY + TextZeilen - 1); /* Cursor auf Anfang letzte Zeile */
	TextFensterAusgabe = TRUE;
}

void EndOutput(void)
/* Schließt Ausgabe im Textfenster ab */
{
	TextFensterAusgabe = FALSE;
}

void OutputText(char *s);
{
	BeginOutput;
	Print(s);
	EndOutput;
}

void PrintOutput()
{
	unsigned i, j;
	for (j = 0; j <= TextZeilen - 1; j++) {/* Textbereich löschen */
		for (i = 0; i <= TextSpalten / 2 - 1; i++)
			SetSprite(TextX / 2 + i, TextY + j, SystemSprite[0]);
		PrintAt(TextX, TextY + j, TextFenster[j]);
	}
}


/* Sonstiges und Stringfunktionen ***********************************/

int Compare(char *s, char *p, size_t ns, size_t np)
{
	unsigned i;
	for (i = 0; i <= ns; i++) {
		if (i > np) return s[i] == '\0';
		if (s[i] != p[i]) return FALSE;
		if (s[i] == '\0') return TRUE;
	}
	if (ns == np) return TRUE;
	return p[ns+1] == '\0';
}

int COMPARE(char *s, char *p, size_t ns, size_t np)
{
	unsigned i;
	for (i = 0; i <= ns; i++) {
		if (i > np) return s[i] == '\0';
		if (CAP(s[i]) != CAP(p[i])) return FALSE;
		if (s[i] == '\0') return TRUE;
	}
	if (ns == np) return TRUE;
	return p[ns+1] == '\0';
}

int SMALLER(char *s, char *p, size_t ns, size_t np)
{
	/* Liefert s < p */
	unsigned i;
	for (i = 0; i <= ns; i++) {
		if (i > np)
			return FALSE;
		else if (cap(s[i]) != cap(p[i]))
			return cap(s[i]) < cap(p[i]);
		else if (s[i] == '\0')
			return FALSE;
	}
	return TRUE;
}

unsigned Length(char *s, size_t ns)
{
	unsigned i;
	for (i = 0; i <= ns; i++)
		if (s[i] == '\0') return i;
	return ns+1;
}

void Assign(char *s, char *p, size_t ns, size_t np)
{
	unsigned i;
	for (i = 0; i <= ns; i++) {
		if (i > np) {
			s[i] = '\0';
			return;
		}
		s[i] = p[i];
		if (s[i] == '\0')
			return;
	}
}

void Concat(char *s, char *p, char *r, size_t ns, size_t nr)
{
	unsigned i, j;
	Assign(s, p);
	i = Length(s);
	for (j = 0; j <= nr; j++) {
		if (i > ns)
			return;
		s[i] = r[j];
		if (s[i] == '\0')
			return;
		i++;
	}
	if (i <= ns)
		s[i] = '\0';
}

void Split(char *p, char *r, char *s, size_t np, size_t nr, size_t ns, unsigned i)
{
	unsigned j;
	for (j = 0; j <= np; j++)
		if (j < i && j <= ns)
			p[j] = s[j];
		else
			p[j] = '\0';
	for (j = 0; j <= nr; j++)
		if (j + i <= ns) 
			r[j] = s[j+i];
		else
			r[j] = '\0';
}

unsigned FindC(char *s, size_t ns, char c)
{
	unsigned i;
	for (i = 0; i <= ns; i++) {
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
	else if (ch == 'ä') return 'Ä';
	else if (ch == 'ü') return 'Ü';
	else if (ch == 'ö') return 'Ö';
	else return ch;
}

int StringToInt(char *s, size_t ns)
{
	unsigned i;
	int f, z;
	z = 0;
	f = 1;
	for (i = 0; i <= ns; i++)
		if (s[i] == '-' && f == 1) { /* ein Minuszeichen */
			f = -1;
		else if (s[i] >= '0' && s[i] <= '9')
			z = z * 10 + (s[i] - '0');
		else if (s[i] != ' ')
			return z * f;
	return z * f;
}

void AssignC(char *s, char c, size_t ns)
{
	s[0] = c;
	if (ns >= 1)
		s[1] = '\0';
}

void OutputInit(void)
{ /* Initialisierung */
	TextFensterAusgabe = FALSE;
	TextFenster[TextZeilen] = "";
	TextMode = 0;
	AnzahlTexte = 0;
	GotoXY(0, 0);
	CharOut = BWOut;
	StringOut = BWStringOut;
	PrintChar = BWOut;
	Cut = "' :.,\\/";
	MaxX = 80;
}
