/*
  screen.h
  
  Super Tux - Screen Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/

#ifndef SUPERTUX_SCREEN_H
#define SUPERTUX_SCREEN_H

#include <SDL.h>
#ifndef NOOPENGL
#include <SDL_opengl.h>
#endif
#include "texture.h"

#define NO_UPDATE false
#define UPDATE true
#define USE_ALPHA 0
#define IGNORE_ALPHA 1

struct Color
{
  Color() 
    : red(0), green(0), blue(0)
  {}
  
  Color(int red_, int green_, int blue_)
    : red(red_), green(green_), blue(blue_)
  {}

  int red, green, blue;
};

void drawline(int x1, int y1, int x2, int y2, int r, int g, int b, int a);
void clearscreen(int r, int g, int b);
void drawgradient(Color top_clr, Color bot_clr);
void fillrect(float x, float y, float w, float h, int r, int g, int b, int a);
void updatescreen(void);
void flipscreen(void);
void update_rect(SDL_Surface *scr, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

#endif /*SUPERTUX_SCREEN_H*/
