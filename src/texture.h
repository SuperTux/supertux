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

#include <SDL.h>
#include <string>
#ifndef NOOPENGL
#include <SDL_opengl.h>
#endif

#include "screen.h"

/* Texture type */
struct texture_type
{
  SDL_Surface* sdl_surface;
  unsigned gl_texture;
  int w;
  int h;
};

void texture_setup(void);
extern void (*texture_load) (texture_type* ptexture, const std::string& file, int use_alpha);  
extern void (*texture_load_part) (texture_type* ptexture, const std::string& file, int x, int y, int w, int h, int use_alpha);
extern void (*texture_free) (texture_type* ptexture);  
extern void (*texture_draw) (texture_type* ptexture, float x, float y, Uint8 alpha = 255, bool update = false);
extern void (*texture_draw_bg) (texture_type* ptexture,  Uint8 alpha = 255, bool update = false);
extern void (*texture_draw_part) (texture_type* ptexture, float sx, float sy, float x, float y, float w, float h,  Uint8 alpha = 255, bool update = false);
void texture_load_sdl(texture_type* ptexture, const std::string&, int use_alpha);
void texture_load_part_sdl(texture_type* ptexture, const std::string& file, int x, int y, int w, int h, int use_alpha);
void texture_free_sdl(texture_type* ptexture);
void texture_draw_sdl(texture_type* ptexture, float x, float y, Uint8 alpha, bool update);
void texture_draw_bg_sdl(texture_type* ptexture,  Uint8 alpha, bool update);
void texture_draw_part_sdl(texture_type* ptexture,float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update);
void texture_from_sdl_surface(texture_type* ptexture, SDL_Surface * sdl_surf, int use_alpha);
#ifndef NOOPENGL
void texture_load_gl(texture_type* ptexture, const std::string& file, int use_alpha);
void texture_load_part_gl(texture_type* ptexture, const std::string& file, int x, int y, int w, int h, int use_alpha);
void texture_free_gl(texture_type* ptexture);
void texture_draw_gl(texture_type* ptexture, float x, float y,  Uint8 alpha, bool update);
void texture_draw_bg_gl(texture_type* ptexture,  Uint8 alpha, bool update);
void texture_draw_part_gl(texture_type* ptexture, float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update);
void texture_create_gl(SDL_Surface * surf, GLuint * tex);
#endif

#endif /*SUPERTUX_TEXTURE_H*/

