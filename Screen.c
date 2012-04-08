/* Screen module */
#include "compat.h"
#include "Screen.h"

#include "HASCSGraphics.h"
#include "HASCSOutput.h"
#include "HASCSGlobal.h"
#include "HASCSSystem.h"


/************************************************************************/

void NewScreen(unsigned sx, unsigned sy, unsigned sw, unsigned sh, char *t)
{
	DScreen.x = sx; DScreen.y = sy; DScreen.w = sw; DScreen.h = sh;
	Assign(DScreen.Title, t);
	DScreen.n = 0;
}

unsigned AddObject(unsigned ox, unsigned oy, unsigned ow, unsigned oh, char *t, BITSET f)
{
	if (DScreen.n >= MaxObjects) return 0;
	DScreen.n += 1;
	DScreen.Obj[DScreen.n].x = ox; DScreen.Obj[DScreen.n].y = oy; DScreen.Obj[DScreen.n].w = ow; DScreen.Obj[DScreen.n].h = oh;
	Assign(DScreen.Obj[DScreen.n].Text, t); Assign(DScreen.Obj[DScreen.n].Input, "");
	DScreen.Obj[DScreen.n].Flags = f;
	return DScreen.n;
}

/************************************************************************/

void SetInputString(unsigned i, char *s)
{
	Assign(DScreen.Obj[i].Input, s);
}

void GetInputString(unsigned i, char *s)
{
	Assign(s, DScreen.Obj[i].Input);
}

void SetInputCard(unsigned i, unsigned c)
{
	CardToString(c, 1, DScreen.Obj[i].Input);
}

unsigned GetInputCard(unsigned i)
{
	return StringToCard(DScreen.Obj[i].Input);
}

unsigned GetFlagSelected(unsigned i)
{
	if (Inverted & DScreen.Obj[i].Flags)
		return 1;
	else
		return 0;
}

void SetFlagSelected(unsigned i, unsigned f)
{
	if (f == 0)
		DScreen.Obj[i].Flags &= ~Inverted;
	else
		DScreen.Obj[i].Flags |= Inverted;
}

/************************************************************************/

void DrawObject(unsigned i)
{
	unsigned ox, oy, j, k;
	ox = DScreen.x + DScreen.Obj[i].x;
	oy = DScreen.y + DScreen.Obj[i].y;
	if ((Editable|Selectable|Exit) & DScreen.Obj[i].Flags)
		FillRectangle(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1, &SystemSprite[0]);
	
	if (SpriteFill & DScreen.Obj[i].Flags) {
		j = StringToCard(DScreen.Obj[i].Text);
		if (j < MaxSprites)
			FillRectangle(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1, &FelderSprite[j]);
		else if (j < 2 * MaxSprites)
			FillRectangle(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1, &MonsterSprite[j-MaxSprites]);
		else if (j < 3 * MaxSprites)
			FillRectangle(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1, &SystemSprite[j-2*MaxSprites]);
		else if (j < 4 * MaxSprites)
			FillRectangle(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1, &GegenSprite[j-3*MaxSprites]);
	} else if (BigText & DScreen.Obj[i].Flags) {
		TextMode = 1;
		if (Centered & DScreen.Obj[i].Flags)
			PrintAt(ox + DScreen.Obj[i].w / 2 - Length(DScreen.Obj[i].Text) / 2, oy, DScreen.Obj[i].Text);
		else
			PrintAt(ox, oy , DScreen.Obj[i].Text);
		TextMode = 0;
	} else {
		if (Centered & DScreen.Obj[i].Flags)
			PrintAt(2 * ox + DScreen.Obj[i].w - Length(DScreen.Obj[i].Text) / 2, oy, DScreen.Obj[i].Text);
		else
			PrintAt(2 * ox, oy, DScreen.Obj[i].Text);
	}
	if (Editable & DScreen.Obj[i].Flags) {
		if (BigText & DScreen.Obj[i].Flags)
			TextMode = 1;
		Print(DScreen.Obj[i].Input);
		if (Edited & DScreen.Obj[i].Flags)
			Print("_");
		if (BigText & DScreen.Obj[i].Flags)
			TextMode = 0;
	} else if (Length(DScreen.Obj[i].Input) != 0) {
		if (BigText & DScreen.Obj[i].Flags) TextMode = 1;
		Print(DScreen.Obj[i].Input);
		TextMode = 0;
	}
	if (Inverted & DScreen.Obj[i].Flags)
		for (j = 1; j <= DScreen.Obj[i].w; j++)
			for (k = 1; k <= DScreen.Obj[i].h; k++)
				InvertFeld(ox + j - 1, oy + k - 1);
	else if (Outlined & DScreen.Obj[i].Flags && Exit & DScreen.Obj[i].Flags)
		DOutlineBar(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1);
	else if (Outlined & DScreen.Obj[i].Flags)
		OutlineBar(ox, oy, ox + DScreen.Obj[i].w - 1, oy + DScreen.Obj[i].h - 1);
}

void DrawScreen(void)
{
	unsigned i;
	ReserveScreen(DScreen.x, DScreen.y, DScreen.x + DScreen.w - 1, DScreen.y + DScreen.h - 1);
	PrintAt(2 * DScreen.x + DScreen.w - Length(DScreen.Title) / 2, DScreen.y, DScreen.Title);
	for (i = 1; i <= DScreen.n; i++)
		DrawObject(i);
}

/************************************************************************/

static unsigned FindObject(unsigned mx, unsigned my)
{
	unsigned i;
	if (mx < DScreen.x || my < DScreen.y) return 0;
	mx -= DScreen.x; my -= DScreen.y;
	for (i = 1; i <= DScreen.n; i++)
		if (mx >= DScreen.Obj[i].x && my >= DScreen.Obj[i].y && mx < DScreen.Obj[i].x + DScreen.Obj[i].w && my < DScreen.Obj[i].y + DScreen.Obj[i].h)
			return i;
	return 0;
}

static unsigned FindFlags(unsigned start, BITSET f)
{
	unsigned i;
	for (i = start; i <= DScreen.n; i++)
		if ((DScreen.Obj[i].Flags & f) != 0)
			return i;
	return 0;
}

static void EditObject(unsigned e, char ch)
{
	int ok;
	if (ch >= ' ') {
		if (BigText & DScreen.Obj[e].Flags)
			ok = Length(DScreen.Obj[e].Text) + Length(DScreen.Obj[e].Input)  < DScreen.Obj[e].w - 1;
		else
			ok = (Length(DScreen.Obj[e].Text) + Length(DScreen.Obj[e].Input) + 1) / 2 < DScreen.Obj[e].w;
		if (ok) {
			DScreen.Obj[e].Input[Length(DScreen.Obj[e].Input)+1] = '\0';
			DScreen.Obj[e].Input[Length(DScreen.Obj[e].Input)] = ch;
		}
	} else if (ch == '\010' && Length(DScreen.Obj[e].Input) > 0) /* Backspace */
		DScreen.Obj[e].Input[Length(DScreen.Obj[e].Input)-1] = '\0';
	else if (ch == '\033')
		*DScreen.Obj[e].Input = *"";
}

unsigned HandleScreen(void)
{
	unsigned i, mx, my, edit, newedit, ende;
	BITSET mb;
	char mch;

	ende = 0;
	edit = FindFlags(1, Editable);
	newedit = edit;
	if (edit != 0) {
		DScreen.Obj[edit].Flags |= Edited;
		DrawObject(edit);
	}

	while (ende == 0) {
		WaitInput(&mx, &my, &mb, &mch, -1); mx = mx / 16; my = my / 16;
		if (mb) {
			i = FindObject(mx, my);
			if (i != 0) {
				if (Selectable & DScreen.Obj[i].Flags) {
					DScreen.Obj[i].Flags = DScreen.Obj[i].Flags & ~Inverted;
					DrawObject(i);
				}
				if (Editable & DScreen.Obj[i].Flags)
					newedit = i;
				if (Exit & DScreen.Obj[i].Flags) {
					ende = i;
					DScreen.Obj[i].Flags |= Inverted;
					DrawObject(i);
					WaitTime(0);
				}
			}
		} else if (mch > '\0' && mch != '\015' && edit != 0) {
			EditObject(edit, mch);
			DrawObject(edit);
		} else if (mch == '\015') {
			newedit = FindFlags(edit + 1, Editable);
			if (newedit == 0)
				ende = edit;
		}
		if (newedit != edit && newedit != 0) {
			DScreen.Obj[edit].Flags &= ~Edited;
			DrawObject(edit);
			edit = newedit;
			DScreen.Obj[edit].Flags |= Edited;
			DrawObject(edit);
		}
	}
	return ende;
}
