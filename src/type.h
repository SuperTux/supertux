//
// C Interface: type
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_TYPE_H
#define SUPERTUX_TYPE_H

#include <SDL/SDL.h>

/* 'Interoperability' type */

typedef struct itop_type
  {
    int* alive;
    float* x;
    float* y;
    float* width;
    float* height;
    int* updated;
  }
itop_type;

double get_frame_ratio(itop_type* pit);

/* Timer type */
typedef struct timer_type
  {
   unsigned int period;
   unsigned int time;
  }
timer_type;

void timer_init(timer_type* ptimer);
void timer_start(timer_type* ptimer, unsigned int period);
void timer_stop(timer_type* ptimer);
int timer_check(timer_type* ptimer);
int timer_started(timer_type* ptimer);
int timer_get_left(timer_type* ptimer);
int timer_get_gone(timer_type* ptimer);

/* Texture type */
typedef struct texture_type
  {
   SDL_Surface* sdl_surface;
   unsigned gl_texture;
   int w;
   int h;
  }  
texture_type;

void texture_load(texture_type* ptexture, char * file, int use_alpha);
void texture_from_sdl_surface(texture_type* ptexture, SDL_Surface * sdl_surf, int use_alpha);
void texture_free(texture_type* ptexture);
void texture_draw(texture_type* ptexture, float x, float y, int update);
void texture_draw_bg(texture_type* ptexture, int update);
void texture_draw_part(texture_type* ptexture, float x, float y, float w, float h, int update);

#endif /*SUPERTUX_TYPE_H*/

