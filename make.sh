# This let's me use SDL_Mixer installed from source in /usr/local/...
# so that I can keep SDL_Mixer 1.2.1 Debian package installed under Woody,
# but sneak a copy of SDL_Mixer 1.2.5 source install
# -bjk; 2003.12.28

make SDL_LDFLAGS="-L/usr/local/lib `sdl-config --libs`" \
     SDL_CFLAGS="-I/usr/local/include/SDL `sdl-config --cflags`"
