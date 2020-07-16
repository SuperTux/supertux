# Directory specifications
SRCDIR = src
EXTERNDIR = external
EXTERNAL_BUILDDIR = $(EXTERNDIR)/build
# =================================
# Components
ADDON_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/addon/*.cpp))
AUDIO_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/audio/*.cpp))
BADGUY_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/badguy/*.cpp))
COLLISION_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/collision/*.cpp))
CONTROL_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/control/*.cpp))
EDITOR_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/editor/*.cpp))
GUI_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/gui/*.cpp))
MATH_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/math/*.cpp))
OBJECT_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/object/*.cpp))
PHYSFS_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/physfs/*.cpp))
SCRIPTING_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/scripting/*.cpp))
SPRITE_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/sprite/*.cpp))
SQUIRREL_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/squirrel/*.cpp))
SUPERTUX_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/supertux/*.cpp) $(wildcard $(SRCDIR)/supertux/menu/*.cpp))
TRIGGER_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/trigger/*.cpp))
UTIL_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/util/*.cpp))
VIDEO_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/video/*.cpp) $(wildcard $(SRCDIR)/video/gl/*.cpp) $(wildcard $(SRCDIR)/video/null/*.cpp) $(wildcard $(SRCDIR)/video/sdl/*.cpp))
WORLDMAP_OBJ = $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/worldmap/*.cpp))
SUBSYTEMS = $(ADDON_OBJ) $(AUDIO_OBJ) $(BADGUY_OBJ) $(COLLISION_OBJ) $(CONTROL_OBJ) $(EDITOR_OBJ) $(GUI_OBJ) $(MATH_OBJ) $(OBJECT_OBJ) $(PHYSFS_OBJ) $(SCRIPTING_OBJ) $(SPRITE_OBJ) $(SQUIRREL_OBJ) $(SUPERTUX_OBJ) $(TRIGGER_OBJ) $(UTIL_OBJ) $(VIDEO_OBJ) $(WORLDMAP_OBJ)
# =================================
# External
EXT_FINDLOCALE_DIR = $(EXTERNDIR)/findlocale
EXT_SDL_SAVEPNG_DIR = $(EXTERNDIR)/SDL_SavePNG
EXT_SDL_TTF_DIR = $(EXTERNDIR)/SDL_ttf
EXT_SEXP_DIR = $(EXTERNDIR)/sexp-cpp
EXT_SQUIRREL_DIR = $(EXTERNDIR)/squirrel
EXT_TINYGETTEXT_DIR = $(EXTERNDIR)/tinygettext
# =================================
# Commands and flags
# Compiler flags
INCLUDE_PATH = -I/usr/include/SDL2 -I/usr/include/AL -I. -Isrc -I$(EXTERNDIR)/sexp-cpp/include -I$(EXTERNDIR)/tinygettext/build/ -I$(EXTERNDIR)/tinygettext/include/ -I$(EXTERNDIR)/squirrel/include/ -Iexternal/findlocale -Iexternal/SDL_SavePNG -Iexternal/SDL_ttf/external/freetype-2.4.12/include
LINKDIRS = -L. -Lbuild -L$(EXTERNAL_BUILDDIR)
WARNINGS = -W -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-function -Wno-sign-compare
OPTIM = -O3
STD = -std=gnu++14
CXXFLAGS = $(INCLUDE_PATH) $(WARNINGS) $(OPTIM) $(STD)
CXX = g++
CMAKE = cmake
AR = ar
# The 'shut up' flags
V = @
BULDEXT_QUIET = >/dev/null 2>&1 # These external deps generate a lot of noise...
# =================================
# To use implicit rules
CC = $(CXX)
CFLAGS = $(CXXFLAGS)

.PHONY : build_extlibs all clean

all : supertux2

supertux2 : $(SRCDIR)/main.o libsupertux2.a
	$V printf "Linking final binary \033[1m$@\033[0m...\n"
	$V $(CXX) $(CXXFLAGS) -o $@ $(LINKDIRS) \
		$< \
		-lsupertux2 -lsquirrel_static -lsqstdlib_static -ltinygettext -lsexp -lSDL2_ttf -lsavepng -lfindlocale \
		-lGL -lGLEW -lphysfs -lSDL2 -lSDL2_image -lopenal -lvorbisfile -lcurl -lfreetype -lboost_filesystem -lboost_locale -lpng

libsupertux2.a : $(SUBSYTEMS)
	$V printf "Building supertux static library \033[1m$@\033[0m from object files...\n"
	$V $(AR) rcs $@ $^

%.o : %.cpp
	$V printf "Building \033[1m$@\033[0m from $^...\n"
	$V $(CXX) $(CXXFLAGS) -c -o $@ $^

build_extlibs : FORCE
	$V printf "Building \033[1mexternal libraries\033[0m: "
	$V printf "\033[3msquirrel...\033[0m "
	$V mkdir -p $(EXT_SQUIRREL_DIR)/build $(BULDEXT_QUIET)
	$V $(CMAKE) -B$(EXT_SQUIRREL_DIR)/build -S$(EXT_SQUIRREL_DIR) $(BULDEXT_QUIET)
	$V $(MAKE) -C$(EXT_SQUIRREL_DIR)/build $(BULDEXT_QUIET)
	$V printf "\033[3mtinygettext...\033[0m "
	$V mkdir -p $(EXT_TINYGETTEXT_DIR)/build $(BULDEXT_QUIET)
	$V $(CMAKE) -B$(EXT_TINYGETTEXT_DIR)/build -S$(EXT_TINYGETTEXT_DIR) -DBUILD_SHARED_LIBS=OFF $(BULDEXT_QUIET)
	$V $(MAKE) -C$(EXT_TINYGETTEXT_DIR)/build $(BULDEXT_QUIET)
	$V printf "\033[3msexp-cpp...\033[0m "
	$V mkdir -p $(EXT_SEXP_DIR)/build/ $(BULDEXT_QUIET)
	$V $(CMAKE) -B$(EXT_SEXP_DIR)/build -S$(EXT_SEXP_DIR) $(BULDEXT_QUIET)
	$V $(MAKE) -C$(EXT_SEXP_DIR)/build $(BULDEXT_QUIET)
	$V printf "\033[3mSDL_SavePNG...\033[0m "
	$V $(MAKE) -C$(EXT_SDL_SAVEPNG_DIR) $(BULDEXT_QUIET)
	$V $(AR) rcs $(EXT_SDL_SAVEPNG_DIR)/libsavepng.a $(EXT_SDL_SAVEPNG_DIR)/savepng.o $(BULDEXT_QUIET)
	$V printf "\033[3mfindlocale...\033[0m "
	$V $(MAKE) -C$(EXT_FINDLOCALE_DIR) findlocale.o $(BULDEXT_QUIET)
	$V $(AR) rcs $(EXT_FINDLOCALE_DIR)/libfindlocale.a $(EXT_FINDLOCALE_DIR)/findlocale.o $(BULDEXT_QUIET)
	$V printf "\033[3mSDL_ttf...\033[0m "
	$V $(MAKE) -s CC=$(CXX) CFLAGS='$(CXXFLAGS)' $(EXT_SDL_TTF_DIR)/SDL_ttf.o
	$V $(AR) rcs $(EXT_SDL_TTF_DIR)/libSDL_ttf.a $(EXT_SDL_TTF_DIR)/SDL_ttf.o $(BULDEXT_QUIET)
	$V echo "Done!"
	$V echo "Copying external libraries to a common location..."
	$V cp $(EXT_SQUIRREL_DIR)/build/squirrel/libsquirrel_static.a $(EXT_SQUIRREL_DIR)/build/sqstdlib/libsqstdlib_static.a $(EXT_TINYGETTEXT_DIR)/build/libtinygettext.a $(EXT_SEXP_DIR)/build/libsexp.a $(EXT_SDL_SAVEPNG_DIR)/libsavepng.a $(EXT_FINDLOCALE_DIR)/libfindlocale.a $(EXTERNAL_BUILDDIR)

ensure_directories : FORCE
	$V echo "Creating neccessary directories..."
	$V mkdir -p $(EXTERNAL_BUILDDIR)

clean : FORCE
	$V printf "Cleaning... "
	$V find \( -name '*.o' -or -name '*.a' -or -name '*.so' \) -delete
	$V echo "Clean!"

FORCE :
