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

/* Texture type */
typedef struct texture_type
  {
   SDL_Surface* sdl_surface;
   unsigned gl_texture;
   int w;
   int h;
  }  
texture_type;

void texture_setup(int opengl);
void texture_load(texture_type* ptexture, char * file, int use_alpha);
void texture_from_sdl_surface(texture_type* ptexture, SDL_Surface * sdl_surf, int use_alpha);
void texture_free(texture_type* ptexture);
void texture_draw(texture_type* ptexture, float x, float y, int update);
void texture_draw_bg(texture_type* ptexture, int update);
void texture_draw_part(texture_type* ptexture, float x, float y, float w, float h, int update);

#endif /*SUPERTUX_TEXTURE_H*/

