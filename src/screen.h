/*
  screen.h
  
  Super Tux - Screen Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 21, 2000
*/

#include <SDL.h>
#ifndef NOOPENGL
#include <SDL_opengl.h>

#ifndef GL_NV_texture_rectangle
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV   0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV     0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV  0x84F8
#endif
#endif

#define NO_UPDATE 0
#define UPDATE 1
#define USE_ALPHA 0
#define IGNORE_ALPHA 1

void load_and_display_image(char * file);
void clearscreen(float r, float g, float b);
void fillrect(float x, float y, float w, float h, float r, float g, float b);
void updatescreen(void);
void flipscreen(void);
SDL_Surface * load_image(char * file, int use_alpha);
/*
void drawimage(SDL_Surface * surf, float x, float y, int update);
void drawpart(SDL_Surface * surf, float x, float y, float w, float h, int update);
*/
void drawtext(char * text, int x, int y, SDL_Surface * surf, int update, int shadowsize);
void drawcenteredtext(char * text, int y, SDL_Surface * surf, int update, int shadowsize);
void erasetext(char * text, int x, int y, SDL_Surface * surf, int update, int shadowsize);
void erasecenteredtext(char * text, int y, SDL_Surface * surf, int update, int shadowsize);
void update_rect(SDL_Surface *scr, Sint32 x, Sint32 y, Sint32 w, Sint32 h);

