localedir = $(datadir)/locale

bin_PROGRAMS = supertux
supertux_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\"
supertux_LDADD = $(top_builddir)/lib/libsupertux.la @LIBINTL@


supertux_SOURCES = badguy.cpp badguy.h bitmask.cpp bitmask.h camera.cpp \
			camera.h collision.cpp collision.h intro.cpp intro.h \
			gameloop.cpp gameloop.h high_scores.cpp high_scores.h interactive_object.cpp \
			interactive_object.h level.cpp level.h level_subset.cpp level_subset.h leveleditor.cpp \
			leveleditor.h particlesystem.cpp particlesystem.h player.cpp player.h scene.cpp \
			scene.h special.cpp special.h supertux.cpp title.cpp title.h worldmap.cpp \
			worldmap.h tile.h tile.cpp tile_manager.h tile_manager.cpp resources.h \
			resources.cpp gameobjs.h gameobjs.cpp background.h background.cpp tilemap.h \
			tilemap.cpp serializable.h sector.cpp sector.h misc.h misc.cpp defines.h \
			statistics.cpp badguy_specs.cpp \
                        timer.cpp timer.h \
                        object/coin.h object/coin.cpp \
                        object/block.h object/block.cpp \
                        object/platform.h object/platform.cpp \
                        object/door.h object/door.cpp \
                        object/fireworks.h object/fireworks.cpp \
                        object/bullet.h object/bullet.cpp

# EOF #
INCLUDES = -I$(top_srcdir)/lib
noinst_HEADERS = misc.h
