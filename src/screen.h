/*
  screen.h
  
  Super Tux - Screen Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 21, 2000
*/

#include <SDL.h>

#define NO_UPDATE 0
#define UPDATE 1
#define USE_ALPHA 0
#define IGNORE_ALPHA 1

void load_and_display_image(char * file);
void clearscreen(int r, int g, int b);
void updatescreen(void);
SDL_Surface * load_image(char * file, int use_alpha);
void drawimage(SDL_Surface * surf, int x, int y, int update);
void drawpart(SDL_Surface * surf, int x, int y, int w, int h, int update);
void drawtext(char * text, int x, int y, SDL_Surface * surf, int update);
void drawcenteredtext(char * text, int y, SDL_Surface * surf, int update);
void erasetext(char * text, int x, int y, SDL_Surface * surf, int update);
void erasecenteredtext(char * text, int y, SDL_Surface * surf, int update);
