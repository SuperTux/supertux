require 'mkmf'

$CFLAGS += `sdl-config --cflags`.chomp
$LDFLAGS += `sdl-config --libs`.chomp

have_library "physfs", "PHYSFS_init"
have_library "SDL", "SDL_AllocRW"

create_makefile "physfs_so"
