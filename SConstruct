#!/usr/bin/scons -Q
#
# A simple SConstruct file.
# See http://www.scons.org/ for more information about what SCons is and how it
# may help you... :-)
# I've never done anything with SCons before. Quite obviously this script is in
# a non-working state!! Maybe someone with more knowledge of the materia who
# thinks that SCons might be better suited than make can take over....
#                                              - Benjamin P. 'litespeed' Jung -
#


# TODO: such a static entry is obviously not what we want.
#       Using 'sdl-config --prefix' to obtain parameters would be muuuuuch
#       better.
SDL_INCLUDE_PATH='/usr/include/SDL'

libsupertux_src=[
  'lib/app/globals.cpp',
  'lib/app/setup.cpp',
  'lib/audio/musicref.cpp',
  'lib/audio/sound_manager.cpp',
  'lib/gui/button.cpp',
  'lib/gui/menu.cpp',
  'lib/gui/mousecursor.cpp',
  'lib/math/physic.cpp',
  'lib/math/vector.cpp',
  'lib/special/game_object.cpp',
  'lib/special/moving_object.cpp',
  'lib/special/sprite.cpp',
  'lib/special/sprite_manager.cpp',
  'lib/special/timer.cpp',
  'lib/special/frame_rate.cpp',
  'lib/utils/configfile.cpp',
  'lib/utils/lispreader.cpp',
  'lib/utils/lispwriter.cpp',
  'lib/video/drawing_context.cpp',
  'lib/video/font.cpp',
  'lib/video/screen.cpp',
  'lib/video/surface.cpp'
]

supertux_src=[
  'src/background.cpp',
  'src/badguy.cpp',
  'src/badguy_specs.cpp',
  'src/bitmask.cpp',
  'src/camera.cpp',
  'src/collision.cpp',
  'src/door.cpp',
  'src/gameloop.cpp',
  'src/gameobjs.cpp',
  'src/high_scores.cpp',
  'src/interactive_object.cpp',
  'src/intro.cpp',
  'src/level.cpp',
  'src/level_subset.cpp',
  'src/leveleditor.cpp',
  'src/misc.cpp',
  'src/particlesystem.cpp',
  'src/player.cpp',
  'src/resources.cpp',
  'src/scene.cpp',
  'src/sector.cpp',
  'src/serializable.cpp',
  'special.cpp',
  'statistics.cpp',
  'supertux.cpp',
  'tile.cpp'
  'tile_manager.cpp',
  'tilemap.cpp',
  'title.cpp',
  'worldmap.cpp'
]
			
Library(
  target="lib/supertux",
  source=libsupertux_src,
  CPPPATH=SDL_INCLUDE_PATH
)

Program(
  target="src/supertux",
  source=supertux_src,
  CPPPATH=SDL_INCLUDE_PATH,
  LIBS='lib/libsupertux'
)
