//
// C Interface: texture
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_TEXTURE_H
#define SUPERTUX_TEXTURE_H

#include "SDL.h"
#ifndef NOOPENGL
#include "SDL_opengl.h"
#endif

/* Texture type */
typedef struct texture_type
  {
   SDL_Surface* sdl_surface;
   unsigned gl_texture;
   int w;
   int h;
  } texture_type;

void texture_setup(void);
extern void (*texture_load) (texture_type* ptexture, char * file, int use_alpha);  
extern void (*texture_load_part) (texture_type* ptexture, char * file, int x, int y, int w, int h, int use_alpha);
extern void (*texture_free) (texture_type* ptexture);  
extern void (*texture_draw) (texture_type* ptexture, float x, float y, int update);  
extern void (*texture_draw_bg) (texture_type* ptexture, int update);  
extern void (*texture_draw_part) (texture_type* ptexture, float sx, float sy, float x, float y, float w, float h, int update);
void texture_load_sdl(texture_type* ptexture, char * file, int use_alpha);
void texture_load_part_sdl(texture_type* ptexture, char * file, int x, int y, int w, int h, int use_alpha);
void texture_free_sdl(texture_type* ptexture);
void texture_draw_sdl(texture_type* ptexture, float x, float y, int update);
void texture_draw_bg_sdl(texture_type* ptexture, int update);
void texture_draw_part_sdl(texture_type* ptexture,float sx, float sy, float x, float y, float w, float h, int update);
void texture_from_sdl_surface(texture_type* ptexture, SDL_Surface * sdl_surf, int use_alpha);
#ifndef NOOPENGL
void texture_load_gl(texture_type* ptexture, char * file, int use_alpha);
void texture_load_part_gl(texture_type* ptexture, char * file, int x, int y, int w, int h, int use_alpha);
void texture_free_gl(texture_type* ptexture);
void texture_draw_gl(texture_type* ptexture, float x, float y, int update);
void texture_draw_bg_gl(texture_type* ptexture, int update);
void texture_draw_part_gl(texture_type* ptexture, float sx, float sy, float x, float y, float w, float h, int update);
void texture_create_gl(SDL_Surface * surf, GLuint * tex);
#endif

#endif /*SUPERTUX_TEXTURE_H*/

