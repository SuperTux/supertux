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

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_opengl.h>
#include "globals.h"
#include "screen.h"
#include "setup.h"
#include "texture.h"

void texture_load(texture_type* ptexture, char * file, int use_alpha)
{
  SDL_Surface * temp;

  temp = IMG_Load(file);

  if (temp == NULL)
    st_abort("Can't load", file);

  ptexture->sdl_surface = SDL_DisplayFormatAlpha(temp);

  if (ptexture->sdl_surface == NULL)
    st_abort("Can't covert to display format", file);

  if (use_alpha == IGNORE_ALPHA)
    SDL_SetAlpha(ptexture->sdl_surface, 0, 0);

  SDL_FreeSurface(temp);

  ptexture->w = ptexture->sdl_surface->w;
  ptexture->h = ptexture->sdl_surface->h;

  if(use_gl)
    {
      create_gl_texture(ptexture->sdl_surface,&ptexture->gl_texture);
    }
}

void texture_from_sdl_surface(texture_type* ptexture, SDL_Surface* sdl_surf, int use_alpha)
{
 /* SDL_Surface * temp;

  temp = IMG_Load(file);

  if (temp == NULL)
    st_abort("Can't load", file);*/

  ptexture->sdl_surface = SDL_DisplayFormatAlpha(sdl_surf);

  if (ptexture->sdl_surface == NULL)
    st_abort("Can't covert to display format", "SURFACE");

  if (use_alpha == IGNORE_ALPHA)
    SDL_SetAlpha(ptexture->sdl_surface, 0, 0);

  ptexture->w = ptexture->sdl_surface->w;
  ptexture->h = ptexture->sdl_surface->h;

  if(use_gl)
    {
      create_gl_texture(ptexture->sdl_surface,&ptexture->gl_texture);
    }
}

void texture_draw(texture_type* ptexture, float x, float y, int update)
{
  if(use_gl)
    {
      glColor4ub(255, 255, 255,255);
      glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glEnable (GL_BLEND);
      glBindTexture(GL_TEXTURE_RECTANGLE_NV, ptexture->gl_texture);

      glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex2f(x, y);
      glTexCoord2f((float)ptexture->w, 0);
      glVertex2f((float)ptexture->w+x, y);
      glTexCoord2f((float)ptexture->w, (float)ptexture->h);
      glVertex2f((float)ptexture->w+x, (float)ptexture->h+y);
      glTexCoord2f(0, (float)ptexture->h);
      glVertex2f(x, (float)ptexture->h+y);
      glEnd();
    }
  else
    {
      SDL_Rect dest;

      dest.x = x;
      dest.y = y;
      dest.w = ptexture->w;
      dest.h = ptexture->h;

      SDL_BlitSurface(ptexture->sdl_surface, NULL, screen, &dest);

      if (update == UPDATE)
        SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
    }
}

void texture_draw_bg(texture_type* ptexture, int update)
{
if(use_gl)
{
    //glColor3ub(255, 255, 255);

    glEnable(GL_TEXTURE_RECTANGLE_NV);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, ptexture->gl_texture);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);    glVertex2f(0, 0);
        glTexCoord2f((float)ptexture->w, 0);    glVertex2f(screen->w, 0);
        glTexCoord2f((float)ptexture->w, (float)ptexture->h);    glVertex2f(screen->w, screen->h);
        glTexCoord2f(0, (float)ptexture->h); glVertex2f(0, screen->h);
    glEnd();
 
}
else
{
  SDL_Rect dest;
  
  dest.x = 0;
  dest.y = 0;
  dest.w = screen->w;
  dest.h = screen->h;
  
  SDL_BlitSurface(ptexture->sdl_surface, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}
}

void texture_draw_part(texture_type* ptexture, float x, float y, float w, float h, int update)
{
  if(use_gl)
    {
      glColor3ub(255, 255, 255);

      glEnable(GL_TEXTURE_RECTANGLE_NV);
      glBindTexture(GL_TEXTURE_RECTANGLE_NV, ptexture->gl_texture);

      glBegin(GL_QUADS);
      glTexCoord2f(x, y);
      glVertex2f(x, y);
      glTexCoord2f(x+w, y);
      glVertex2f(w+x, y);
      glTexCoord2f(x+w, y+h);
      glVertex2f(w+x, h+y);
      glTexCoord2f(x, y+h);
      glVertex2f(x, h+y);
      glEnd();
    }
  else
    {
      SDL_Rect src, dest;

      src.x = x;
      src.y = y;
      src.w = w;
      src.h = h;

      dest.x = x;
      dest.y = y;
      dest.w = w;
      dest.h = h;


      SDL_BlitSurface(ptexture->sdl_surface, &src, screen, &dest);

      if (update == UPDATE)
        update_rect(screen, dest.x, dest.y, dest.w, dest.h);
    }
}

void texture_free(texture_type* ptexture)
{
  SDL_FreeSurface(ptexture->sdl_surface);
  if(use_gl)
    glDeleteTextures(1, &ptexture->gl_texture);
}

