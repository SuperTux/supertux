#
# A simple SConstruct file.
# See http://www.scons.org/ for more information about what SCons is and how it
# may help you... :-)
# I've never done anything with SCons before. Quite obviously this script is in
# a non-working state!! Maybe someone with more knowledge of the materia can
# take over....
#                                              - Benjamin P. 'litespeed' Jung -
#

# TODO: such static entries are obviously not what we want.
# Using e.g. 'sdl-config' to obtain parameters would be muuuuuch
# better.


DATA_PREFIX = '\\\"/usr/local/share/supertux\\\"'
LOCALEDIR = '\\\"/usr/local/share/locale\\\"'

CCFLAGS = [
  '-O2',
  '-DDATA_PREFIX=' + DATA_PREFIX,
  '-DLOCALEDIR=' + LOCALEDIR
]

CPPPATH = ['/usr/include/SDL', '/usr/include/X11', 'src', 'lib', 'intl', '.']

LIBPATH = [
  'lib',
  '/lib',
  '/usr/lib',
  '/usr/lib/X11',
  '/usr/local/lib'
]

LIBS = [
  'supertux',
  'SDL',
  'SDL_gfx',
  'SDL_image',
  'SDL_mixer',
  'SDL_sound',
  'pthread',
  'm',
  'dl',
  'asound',
  'GL',
  'GLU'
]
  

libsupertux_src = [
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

supertux_src = [
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
  'src/special.cpp',
  'src/statistics.cpp',
  'src/supertux.cpp',
  'src/tile.cpp',
  'src/tile_manager.cpp',
  'src/tilemap.cpp',
  'src/title.cpp',
  'src/worldmap.cpp'
]
			

StaticLibrary(
  target = 'lib/supertux',
  source = libsupertux_src,
  CPPPATH = CPPPATH,
  CCFLAGS = CCFLAGS
)

Program(
  target = 'src/supertux',
  source = supertux_src,
  CPPPATH = CPPPATH,
  CCFLAGS = CCFLAGS,
  LIBPATH = LIBPATH,
  LIBS = LIBS
)
