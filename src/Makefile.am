localedir = $(datadir)/locale

bin_PROGRAMS = supertux
supertux_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\"
supertux_LDADD = $(top_builddir)/lib/libsupertux.la @LIBINTL@


supertux_SOURCES = camera.cpp \
			camera.h collision.cpp collision.h intro.cpp intro.h \
			gameloop.cpp gameloop.h high_scores.cpp high_scores.h \
			level.cpp level.h level_subset.cpp level_subset.h \
                        leveleditor.cpp leveleditor.h particlesystem.cpp \
                        particlesystem.h player.cpp player.h scene.cpp \
			scene.h supertux.cpp title.cpp title.h worldmap.cpp \
			worldmap.h tile.h tile.cpp tile_manager.h tile_manager.cpp resources.h \
			resources.cpp gameobjs.h gameobjs.cpp background.h background.cpp tilemap.h \
			tilemap.cpp serializable.h sector.cpp sector.h misc.h misc.cpp defines.h \
			statistics.cpp badguy_specs.cpp \
                        timer.cpp timer.h \
                        object/coin.h object/coin.cpp \
                        object/block.h object/block.cpp \
                        object/platform.h object/platform.cpp \
                        object/fireworks.h object/fireworks.cpp \
                        object/bullet.h object/bullet.cpp \
                        object/specialriser.h object/specialriser.cpp \
                        object/star.h object/star.cpp \
                        object/oneup.h object/oneup.cpp \
                        object/flower.h object/flower.cpp \
                        object/growup.h object/growup.cpp \
                        object/invisible_block.h object/invisible_block.cpp \
                        badguy/badguy.h badguy/badguy.cpp \
                        badguy/bomb.h badguy/bomb.cpp \
                        badguy/bouncing_snowball.h badguy/bouncing_snowball.cpp\
                        badguy/flame.h badguy/flame.cpp \
                        badguy/jumpy.h badguy/jumpy.cpp \
                        badguy/mrbomb.h badguy/mrbomb.cpp \
                        badguy/mriceblock.h badguy/mriceblock.cpp \
                        badguy/snowball.h badguy/snowball.cpp \
                        badguy/spiky.h badguy/spiky.cpp \
                        trigger/trigger_base.h trigger/trigger_base.cpp \
                        trigger/door.h trigger/door.cpp \
                        trigger/sequence_trigger.h trigger/sequence_trigger.cpp

INCLUDES = -I$(top_srcdir)/lib
