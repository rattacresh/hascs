CC = gcc
CFLAGS = -g3 -O2 -Wall -DBITSET=unsigned -Dsize_t=unsigned -DCHAR_BIT=8
LIBS = -lefence
HASCSEDITOR_SOURCE = \
	Dialog.c HASCSGame.c HASCSMagic.c HASCSSprite.c Sound.c \
	HASCSCharakter.c HASCSGlobal.c HASCSMonster.c HASCSSystem.c Start.c \
	HASCSDisk.c HASCSGraphics.c HASCSOutput.c Image.c \
	HASCSEditor.c HASCSSpieler.c Screen.c
HASCSEDITOR_OBJS := $(HASCSEDITOR_GEN:.c=.o) $(HASCSEDITOR_SOURCE:.c=.o)
HASCSIII_SOURCE = \
	Dialog.c HASCSGame.c HASCSMagic.c HASCSSprite.c Sound.c \
	HASCSCharakter.c HASCSGlobal.c HASCSMonster.c HASCSSystem.c Start.c \
	HASCSDisk.c HASCSGraphics.c HASCSOutput.c Image.c \
	HASCSIII.c HASCSSpieler.c Screen.c
HASCSIII_OBJS := $(HASCSIII_GEN:.c=.o) $(HASCSIII_SOURCE:.c=.o)

OBJS = $(HASCSEDITOR_OBJS) $(HASCSIII_OBJS)
SOURCE = $(HASCSEDITOR_SOURCE) $(HASCSIII_SOURCE)

TARGETS = HASCSEditor HASCSIII

all: $(TARGETS)

HASCSEditor: $(HASCSEDITOR_OBJS)
	$(CC) $(LFLAGS) $+ -o $@ $(LIBS)

HASCSIII: $(HASCSIII_OBJS)
	$(CC) $(LFLAGS) $+ -o $@ $(LIBS)

clean:
	rm -f $(OBJS)

realclean: clean
	rm -f $(TARGETS)

mrproper: realclean
	rm -f depend

# all header files and the Makefile must be specified first.
#depend: Makefile *.h $(SOURCE)
#	sh deptool.sh $@ $(CXX) "-MM -w" $? | tclsh depgen.tcl $@

#DEPEND
#include depend
