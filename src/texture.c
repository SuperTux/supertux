//
// C Implementation: texture
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "SDL.h"
#include "SDL_image.h"
#include "globals.h"
#include "screen.h"
#include "setup.h"
#include "texture.h"

void (*texture_load) (texture_type* ptexture, char * file, int use_alpha);
void (*texture_load_part) (texture_type* ptexture, char * file, int x, int y, int w, int h, int use_alpha);
void (*texture_free) (texture_type* ptexture);  
void (*texture_draw) (texture_type* ptexture, float x, float y, int update);  
void (*texture_draw_bg) (texture_type* ptexture, int update);  
void (*texture_draw_part) (texture_type* ptexture, float sx, float sy, float x, float y, float w, float h, int update);


void texture_setup(void)
{
#ifdef NOOPENGL
  texture_load = texture_load_sdl;
  texture_load_part = texture_load_part_sdl;
  texture_free = texture_free_sdl;
  texture_draw = texture_draw_sdl;
  texture_draw_bg = texture_draw_bg_sdl;
  texture_draw_part = texture_draw_part_sdl;
#else

  if(use_gl)
    {
      texture_load = texture_load_gl;
      texture_load_part = texture_load_part_gl;
      texture_free = texture_free_gl;
      texture_draw = texture_draw_gl;
      texture_draw_bg = texture_draw_bg_gl;
      texture_draw_part = texture_draw_part_gl;
    }
  else
    {
      texture_load = texture_load_sdl;
      texture_load_part = texture_load_part_sdl;
      texture_free = texture_free_sdl;
      texture_draw = texture_draw_sdl;
      texture_draw_bg = texture_draw_bg_sdl;
      texture_draw_part = texture_draw_part_sdl;
    }
#endif
}

#ifndef NOOPENGL
void texture_load_gl(texture_type* ptexture, char * file, int use_alpha)
{
  texture_load_sdl(ptexture,file,use_alpha);
  texture_create_gl(ptexture->sdl_surface,&ptexture->gl_texture);
}

void texture_load_part_gl(texture_type* ptexture, char * file, int x, int y, int w, int h, int use_alpha)
{
  texture_load_part_sdl(ptexture,file,x,y,w,h,use_alpha);
  texture_create_gl(ptexture->sdl_surface,&ptexture->gl_texture);
}

/* Quick utility function for texture creation */
static int power_of_two(int input)
{
	int value = 1;

	while ( value < input ) {
		value <<= 1;
	}
	return value;
}

void texture_create_gl(SDL_Surface * surf, GLuint * tex)
{
  Uint32 saved_flags;
  Uint8  saved_alpha;
  int w, h;
  SDL_Surface *conv;
  
  w = power_of_two(surf->w);
  h = power_of_two(surf->h),
  conv = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, surf->format->BitsPerPixel,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                              0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else

                              0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif

  /* Save the alpha blending attributes */
  saved_flags = surf->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
  saved_alpha = surf->format->alpha;
  if ( (saved_flags & SDL_SRCALPHA)
       == SDL_SRCALPHA )
    {
      SDL_SetAlpha(surf, 0, 0);
    }

  SDL_BlitSurface(surf, 0, conv, 0);

  /* Restore the alpha blending attributes */
  if ( (saved_flags & SDL_SRCALPHA)
       == SDL_SRCALPHA )
    {
      SDL_SetAlpha(surf, saved_flags, saved_alpha);
    }

  glGenTextures(1, &*tex);
  glBindTexture(GL_TEXTURE_2D , *tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, conv->pitch / conv->format->BytesPerPixel);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, conv->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      
  SDL_FreeSurface(conv);
}

void texture_free_gl(texture_type* ptexture)
{
  SDL_FreeSurface(ptexture->sdl_surface);
  glDeleteTextures(1, &ptexture->gl_texture);
}

void texture_draw_gl(texture_type* ptexture, float x, float y, int update)
{
float pw = power_of_two(ptexture->w);
float ph = power_of_two(ptexture->h);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(255, 255, 255,255);

  glBindTexture(GL_TEXTURE_2D, ptexture->gl_texture);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(x, y);
  glTexCoord2f((float)ptexture->w / pw, 0);
  glVertex2f((float)ptexture->w+x, y);
  glTexCoord2f((float)ptexture->w / pw, (float)ptexture->h / ph);  glVertex2f((float)ptexture->w+x, (float)ptexture->h+y);
  glTexCoord2f(0, (float)ptexture->h / ph);
  glVertex2f(x, (float)ptexture->h+y);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void texture_draw_bg_gl(texture_type* ptexture, int update)
{
float pw = power_of_two(ptexture->w);
float ph = power_of_two(ptexture->h);

  glColor3ub(255, 255, 255);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, ptexture->gl_texture);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(0, 0);
  glTexCoord2f((float)ptexture->w / pw, 0);
  glVertex2f(screen->w, 0);
  glTexCoord2f((float)ptexture->w / pw, (float)ptexture->h / ph);
  glVertex2f(screen->w, screen->h);
  glTexCoord2f(0, (float)ptexture->h / ph);
  glVertex2f(0, screen->h);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
}

void texture_draw_part_gl(texture_type* ptexture,float sx, float sy, float x, float y, float w, float h, int update)
{
float pw = power_of_two(ptexture->w);
float ph = power_of_two(ptexture->h);

  glBindTexture(GL_TEXTURE_2D, ptexture->gl_texture);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(255, 255, 255,255);

  glEnable(GL_TEXTURE_2D);


  glBegin(GL_QUADS);
  glTexCoord2f(sx / pw, sy / ph);
  glVertex2f(x, y);
  glTexCoord2f((float)(sx + w) / pw, sy / ph);
  glVertex2f(w+x, y);
  glTexCoord2f((sx+w) / pw, (sy+h) / ph);
  glVertex2f(w +x, h+y);
  glTexCoord2f(sx / pw, (float)(sy+h) / ph);
  glVertex2f(x, h+y);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

}
#endif

void texture_load_sdl(texture_type* ptexture, char * file, int use_alpha)
{
  SDL_Surface * temp;
  
  temp = IMG_Load(file);

  if (temp == NULL)
    st_abort("Can't load", file);

  if(use_alpha == IGNORE_ALPHA && !use_gl)
  ptexture->sdl_surface = SDL_DisplayFormat(temp);
  else
  ptexture->sdl_surface = SDL_DisplayFormatAlpha(temp);
  
  if (ptexture->sdl_surface == NULL)
    st_abort("Can't covert to display format", file);

  if (use_alpha == IGNORE_ALPHA && !use_gl)
    SDL_SetAlpha(ptexture->sdl_surface, 0, 0);

  SDL_FreeSurface(temp);

  ptexture->w = ptexture->sdl_surface->w;
  ptexture->h = ptexture->sdl_surface->h;
  
}

void texture_load_part_sdl(texture_type* ptexture, char * file, int x, int y, int w, int h,  int use_alpha)
{

  SDL_Rect src;
  SDL_Surface * temp;
  SDL_Surface * conv;

  temp = IMG_Load(file);

  if (temp == NULL)
    st_abort("Can't load", file);

  /* Set source rectangle for conv: */

  src.x = x;
  src.y = y;
  src.w = w;
  src.h = h;

  conv = SDL_CreateRGBSurface(temp->flags, w, h, temp->format->BitsPerPixel,
                              temp->format->Rmask,
                              temp->format->Gmask,
                              temp->format->Bmask,
                              temp->format->Amask);

  /* #if SDL_BYTEORDER == SDL_BIG_ENDIAN
                               0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
  #else

                               0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
  #endif*/

  SDL_SetAlpha(temp,0,0);

  SDL_BlitSurface(temp, &src, conv, NULL);
  if(use_alpha == IGNORE_ALPHA && !use_gl)
  ptexture->sdl_surface = SDL_DisplayFormat(conv);
  else
  ptexture->sdl_surface = SDL_DisplayFormatAlpha(conv);

  if (ptexture->sdl_surface == NULL)
    st_abort("Can't covert to display format", file);

  if (use_alpha == IGNORE_ALPHA && !use_gl)
    SDL_SetAlpha(ptexture->sdl_surface, 0, 0);

  SDL_FreeSurface(temp);
  SDL_FreeSurface(conv);

  ptexture->w = ptexture->sdl_surface->w;
  ptexture->h = ptexture->sdl_surface->h;
}

void texture_from_sdl_surface(texture_type* ptexture, SDL_Surface* sdl_surf, int use_alpha)
{
  Uint32 saved_flags;
  Uint8  saved_alpha;
  
  /* Save the alpha blending attributes */
  saved_flags = sdl_surf->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
  saved_alpha = sdl_surf->format->alpha;
  if ( (saved_flags & SDL_SRCALPHA)
       == SDL_SRCALPHA )
    {
      SDL_SetAlpha(sdl_surf, 0, 0);
    }
   
  if(use_alpha == IGNORE_ALPHA && !use_gl)
  ptexture->sdl_surface = SDL_DisplayFormat(sdl_surf);
  else
  ptexture->sdl_surface = SDL_DisplayFormatAlpha(sdl_surf);

  /* Restore the alpha blending attributes */
  if ( (saved_flags & SDL_SRCALPHA)
       == SDL_SRCALPHA )
    {
      SDL_SetAlpha(ptexture->sdl_surface, saved_flags, saved_alpha);
    }
  
  if (ptexture->sdl_surface == NULL)
    st_abort("Can't covert to display format", "SURFACE");

  if (use_alpha == IGNORE_ALPHA && !use_gl)
    SDL_SetAlpha(ptexture->sdl_surface, 0, 0);

  ptexture->w = ptexture->sdl_surface->w;
  ptexture->h = ptexture->sdl_surface->h;

#ifndef NOOPENGL

  if(use_gl)
    {
      texture_create_gl(ptexture->sdl_surface,&ptexture->gl_texture);
    }
#endif
}

void texture_draw_sdl(texture_type* ptexture, float x, float y, int update)
{

  SDL_Rect dest;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = ptexture->w;
  dest.h = ptexture->h;
  SDL_BlitSurface(ptexture->sdl_surface, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}


void texture_draw_bg_sdl(texture_type* ptexture, int update)
{
  SDL_Rect dest;
  
  dest.x = 0;
  dest.y = 0;
  dest.w = screen->w;
  dest.h = screen->h;
  
  SDL_SoftStretch(ptexture->sdl_surface, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}

void texture_draw_part_sdl(texture_type* ptexture, float sx, float sy, float x, float y, float w, float h, int update)
{
  SDL_Rect src, dest;

  src.x = (int)sx;
  src.y = (int)sy;
  src.w = (int)w;
  src.h = (int)h;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = (int)w;
  dest.h = (int)h;


  SDL_BlitSurface(ptexture->sdl_surface, &src, screen, &dest);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
}

void texture_free_sdl(texture_type* ptexture)
{
  SDL_FreeSurface(ptexture->sdl_surface);
}

