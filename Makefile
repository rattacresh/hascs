CC = gcc
CFLAGS = -g3 -O2 -DBITSET=uint16_t -funsigned-char
LIBS = 
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
SDLDEMO_SOURCE = SDLDemo.c HASCSSystem.c HASCSGraphics.c
SDLDEMO_OBJS := $(SDLDEMO_GEN:.c=.o) $(SDLDEMO_SOURCE:.c=.o)

OBJS = $(HASCSEDITOR_OBJS) $(HASCSIII_OBJS) $(SDLDEMO_OBJS)
SOURCE = $(HASCSEDITOR_SOURCE) $(HASCSIII_SOURCE) $(SDLDEMO_SOURCE)

#TARGETS = SDLDemo HASCSEditor HASCSIII
TARGETS = SDLDemo

all: $(TARGETS)

HASCSEditor: $(HASCSEDITOR_OBJS)
	$(CC) $(LFLAGS) $+ -o $@ $(LIBS)

HASCSIII: $(HASCSIII_OBJS)
	$(CC) $(LFLAGS) $+ -o $@ $(LIBS)

SDLDemo: $(SDLDEMO_OBJS)
	$(CC) $(LFLAGS) $+ -o $@ $(LIBS) -lSDL

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
