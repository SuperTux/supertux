# Makefile for supertux

# by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/

# Version 0.0.0

# April 11, 2000 - December 26, 2000


# User-definable stuff:

DATA_PREFIX=$(PWD)/data
JOY=YES


# Defaults for Linux:

TARGET=supertux
TARGET_DEF=LINUX


CFLAGS=-Wall -O2 $(SDL_CFLAGS) -DDATA_PREFIX=\"$(DATA_PREFIX)\" \
	-D$(NOSOUNDFLAG) -D$(TARGET_DEF) -DJOY_$(JOY)


# Other definitions:

SDL_MIXER=-lSDL_mixer
SDL_IMAGE=-lSDL_image
NOSOUNDFLAG=__SOUND
SDL_LIB=$(SDL_LDFLAGS) $(SDL_MIXER) $(SDL_IMAGE)
SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)
installbin = install -g root -o root -m 755 
installdat = install -g root -o root -m 644


OBJECTS=obj/supertux.o obj/setup.o obj/intro.o obj/title.o obj/gameloop.o \
	obj/screen.o obj/sound.o

# Make commands:

all:	$(TARGET)

#install: $(TARGET)
#	-$(installdat) data $(DESTDIR)/usr/share/games/$(TARGET)/data
#	-$(installbin) $(TARGET) $(DESTDIR)/usr/games/


nosound:
	make supertux SDL_MIXER= NOSOUNDFLAG=NOSOUND

win32:
	make TARGET_DEF=WIN32 TARGET=supertux.exe \
		DATA_PREFIX=data
	cp /usr/local/cross-tools/i386-mingw32/lib/SDL*.dll .
	chmod 644 SDL*.dll

clean:
	-rm supertux supertux.exe
	-rm obj/*.o
	-rm SDL*.dll


# Main executable:

$(TARGET):	$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(SDL_LIB)


# Objects:

obj/supertux.o:	src/supertux.c src/defines.h src/globals.h \
		src/setup.h src/intro.h src/title.h src/gameloop.h \
		src/screen.h src/sound.h
	$(CC) $(CFLAGS) src/supertux.c -c -o obj/supertux.o

obj/setup.o:	src/setup.c src/setup.h \
		src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/setup.c -c -o obj/setup.o

obj/intro.o:	src/intro.c src/intro.h \
		src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/intro.c -c -o obj/intro.o

obj/title.o:	src/title.c src/title.h \
		src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/title.c -c -o obj/title.o

obj/gameloop.o:	src/gameloop.c src/gameloop.h \
		src/defines.h src/globals.h src/screen.h src/sound.h \
		src/setup.h
	$(CC) $(CFLAGS) src/gameloop.c -c -o obj/gameloop.o

obj/screen.o:	src/screen.c src/defines.h src/globals.h src/screen.h
	$(CC) $(CFLAGS) src/screen.c -c -o obj/screen.o

obj/sound.o:	src/sound.c src/defines.h src/globals.h src/sound.h
	$(CC) $(CFLAGS) src/sound.c -c -o obj/sound.o
