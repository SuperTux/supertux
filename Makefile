# Makefile for supertux

# by Bill Kendrick & Tobias Glaesser
# bill@newbreedsoftware.com
# tobi.web@gmx.de
# http://www.newbreedsoftware.com/

# Version 0.0.6 (in development)

# April 11, 2000 - December 29, 2000


# User-definable stuff:

ifeq ($(PREFIX),)
ifeq ($(USERNAME),root)
PREFIX=/usr/local
else
PREFIX=$(PWD)
endif
DATA_PREFIX=$(PWD)/data/
else
DATA_PREFIX=$(PREFIX)/share/games/supertux/data/
endif
JOY=YES


#undefine this for non debugging compilation

DEBUG_FLAG=-DDEBUG

# Defaults for Linux:

TARGET=supertux
TARGET_DEF=LINUX

CFLAGS=-Wall -O2 $(SDL_CFLAGS) -DDATA_PREFIX=\"$(DATA_PREFIX)\" \
	-D$(NOSOUNDFLAG) -D$(TARGET_DEF) -DJOY_$(JOY) $(DEBUG_FLAG)


# Other definitions:

SDL_MIXER=-lSDL_mixer
SDL_IMAGE=-lSDL_image
NOSOUNDFLAG=__SOUND
SDL_LIB=$(SDL_MIXER) $(SDL_IMAGE) $(SDL_LDFLAGS) 
SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)
installbin = install -g $(USERNAME) -o $(USERNAME) -m 755 
installdat = install -g $(USERNAME) -o $(USERNAME) -m 644


OBJECTS=obj/supertux.o obj/setup.o obj/intro.o obj/title.o obj/level.o obj/gameloop.o \
	obj/screen.o obj/sound.o obj/high_scores.o obj/menu.o obj/leveleditor.o

# Make commands:

all:	$(TARGET)

install: $(TARGET)
	mkdir -p $(PREFIX)/games/$(TARGET)
	mkdir -p $(PREFIX)/share/games/$(TARGET)
	mkdir -p $(PREFIX)/bin/
	cp -r data $(PREFIX)/share/games/$(TARGET)/
	chmod -R 0755 $(PREFIX)/share/games/$(TARGET)/data/
	-$(installbin) $(TARGET) $(PREFIX)/games/$(TARGET)/$(TARGET)
	ln -sf $(PREFIX)/games/$(TARGET)/$(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	rm -r $(PREFIX)/games/$(TARGET)
	rm -r $(PREFIX)/share/games/$(TARGET)
	rm $(PREFIX)/bin/$(TARGET)

nosound:
	make supertux SDL_MIXER= NOSOUNDFLAG=NOSOUND

win32:
	make TARGET_DEF=WIN32 TARGET=supertux.exe \
		DATA_PREFIX=data/
	cp /usr/local/cross-tools/i386-mingw32/lib/SDL*.dll .
	chmod 644 SDL*.dll

clean:
	-rm -f supertux supertux.exe
	-rm -f obj/*.o
	-rm -f SDL*.dll


# Main executable:

$(TARGET):	$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(SDL_LIB)

# Objects:

obj/supertux.o:	src/supertux.c src/supertux.h src/defines.h src/globals.h \
		obj/setup.o obj/intro.o obj/title.o obj/gameloop.o \
		obj/screen.o obj/sound.o obj/leveleditor.o
	$(CC) $(CFLAGS) src/supertux.c -c -o obj/supertux.o

obj/setup.o:	src/setup.c src/setup.h obj/sound.o \
		src/defines.h src/globals.h obj/screen.o
	$(CC) $(CFLAGS) src/setup.c -c -o obj/setup.o

obj/intro.o:	src/intro.c src/intro.h \
		src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/intro.c -c -o obj/intro.o

obj/title.o:	src/title.c src/title.h \
		src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/title.c -c -o obj/title.o

obj/level.o:	src/level.c src/defines.h src/globals.h \
			src/level.h src/gameloop.h src/screen.h src/badguy.h
	$(CC) $(CFLAGS) src/level.c -c -o obj/level.o
	
obj/gameloop.o:	src/gameloop.c src/gameloop.h \
		src/defines.h src/globals.h src/screen.h obj/sound.o \
		src/setup.h obj/level.o
	$(CC) $(CFLAGS) src/gameloop.c -c -o obj/gameloop.o

obj/screen.o:	src/screen.c src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/screen.c -c -o obj/screen.o

obj/sound.o:	src/sound.c src/defines.h src/globals.h src/sound.h
	$(CC) $(CFLAGS) src/sound.c -c -o obj/sound.o

obj/high_scores.o:	src/high_scores.c src/defines.h src/globals.h \
	 			obj/sound.o
	$(CC) $(CFLAGS) src/high_scores.c -c -o obj/high_scores.o

obj/menu.o:	src/menu.c src/defines.h src/globals.h \
			obj/sound.o src/menu.h src/leveleditor.h
	$(CC) $(CFLAGS) src/menu.c -c -o obj/menu.o

obj/leveleditor.o:	src/leveleditor.c src/defines.h src/globals.h \
			src/leveleditor.h src/gameloop.h src/screen.h src/badguy.h
	$(CC) $(CFLAGS) src/leveleditor.c -c -o obj/leveleditor.o