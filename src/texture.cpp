//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <assert.h>
#include <iostream>
#include <algorithm>
#include "SDL.h"
#include "SDL_image.h"
#include "texture.h"
#include "globals.h"
#include "setup.h"

Surface::Surfaces Surface::surfaces;

SurfaceData::SurfaceData(SDL_Surface* temp, int use_alpha_)
    : type(SURFACE), surface(0), use_alpha(use_alpha_)
{
  // Copy the given surface and make sure that it is not stored in
  // video memory
  surface = SDL_CreateRGBSurface(temp->flags & (~SDL_HWSURFACE),
                                 temp->w, temp->h,
                                 temp->format->BitsPerPixel,
                                 temp->format->Rmask,
                                 temp->format->Gmask,
                                 temp->format->Bmask,
                                 temp->format->Amask);
  if(!surface)
    st_abort("No memory left.", "");
  SDL_SetAlpha(temp,0,0);
  SDL_BlitSurface(temp, NULL, surface, NULL);
}

SurfaceData::SurfaceData(const std::string& file_, int use_alpha_)
    : type(LOAD), surface(0), file(file_), use_alpha(use_alpha_)
{}

SurfaceData::SurfaceData(const std::string& file_, int x_, int y_, int w_, int h_, int use_alpha_)
    : type(LOAD_PART), surface(0), file(file_), use_alpha(use_alpha_),
    x(x_), y(y_), w(w_), h(h_)
{}

SurfaceData::~SurfaceData()
{
  SDL_FreeSurface(surface);
}

SurfaceImpl*
SurfaceData::create()
{
#ifndef NOOPENGL
  if (use_gl)
    return create_SurfaceOpenGL();
  else
    return create_SurfaceSDL();
#else
  return create_SurfaceSDL();
#endif
}

SurfaceSDL*
SurfaceData::create_SurfaceSDL()
{
  switch(type)
  {
  case LOAD:
    return new SurfaceSDL(file, use_alpha);
  case LOAD_PART:
    return new SurfaceSDL(file, x, y, w, h, use_alpha);
  case SURFACE:
    return new SurfaceSDL(surface, use_alpha);
  }
  assert(0);
}

SurfaceOpenGL*
SurfaceData::create_SurfaceOpenGL()
{
#ifndef NOOPENGL
  switch(type)
  {
  case LOAD:
    return new SurfaceOpenGL(file, use_alpha);
  case LOAD_PART:
    return new SurfaceOpenGL(file, x, y, w, h, use_alpha);
  case SURFACE:
    return new SurfaceOpenGL(surface, use_alpha);
  }
#endif
  assert(0);
}

#ifndef NOOPENGL
/* Quick utility function for texture creation */
static int power_of_two(int input)
{
  int value = 1;

  while ( value < input )
  {
    value <<= 1;
  }
  return value;
}
#endif

Surface::Surface(SDL_Surface* surf, int use_alpha)
    : data(surf, use_alpha), w(0), h(0)
{
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
  surfaces.push_back(this);
}

Surface::Surface(const std::string& file, int use_alpha)
    : data(file, use_alpha), w(0), h(0)
{
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
  surfaces.push_back(this);
}

Surface::Surface(const std::string& file, int x, int y, int w, int h, int use_alpha)
    : data(file, x, y, w, h, use_alpha), w(0), h(0)
{
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
  surfaces.push_back(this);
}

void
Surface::reload()
{
  delete impl;
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
}

Surface::~Surface()
{
#ifdef DEBUG
  bool found = false;
  for(std::list<Surface*>::iterator i = surfaces.begin(); i != surfaces.end();
      ++i)
  {
    if(*i == this)
    {
      found = true; break;
    }
  }
  if(!found)
    printf("Error: Surface freed twice!!!\n");
#endif
  surfaces.remove(this);
  delete impl;
}

void
Surface::reload_all()
{
  for(Surfaces::iterator i = surfaces.begin(); i != surfaces.end(); ++i)
  {
    (*i)->reload();
  }
}

void
Surface::debug_check()
{
  for(Surfaces::iterator i = surfaces.begin(); i != surfaces.end(); ++i)
  {
    printf("Surface not freed: T:%d F:%s.\n", (*i)->data.type,
           (*i)->data.file.c_str());
  }
}

void
Surface::draw(float x, float y, Uint8 alpha, bool update)
{
  if (impl)
  {
#ifdef GP2X
    if (impl->draw(x/2, y/2, alpha, update) == -2)
#else
    if (impl->draw(x, y, alpha, update) == -2)
#endif
      reload();
  }
}

void
Surface::draw_bg(Uint8 alpha, bool update)
{
  if (impl)
  {
    if (impl->draw_bg(alpha, update) == -2)
      reload();
  }
}

void
Surface::draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update)
{
  if (impl)
  {
#ifdef GP2X
	if (impl->draw_part(sx, sy, x, y/2, w, h, alpha, update) == -2)
#else
	if (impl->draw_part(sx, sy, x, y, w, h, alpha, update) == -2)
#endif
      reload();
  }
}

void
Surface::draw_stretched(float x, float y, int w, int h, Uint8 alpha, bool update)
{
  if (impl)
  {
#ifdef GP2X
    if (impl->draw_stretched(x/2, y/2, w, h, alpha, update) == -2)
#else
    if (impl->draw_stretched(x, y, w, h, alpha, update) == -2)
#endif
      reload();
  }
}

void
Surface::resize(int w_, int h_)
{
  if (impl)
  {
    w = w_;
    h = h_;
    if (impl->resize(w_,h_) == -2)
      reload();
  }
}

Surface* Surface::CaptureScreen()
{
  Surface *cap_screen;

  if (!(screen->flags & SDL_OPENGL))
  {
    cap_screen = new Surface(SDL_GetVideoSurface(),false);
  }

#ifndef NOOPENGL
  if (use_gl)
  {
    SDL_Surface *temp;
    unsigned char *pixels;
    int i;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
                                0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
                               );
    if (temp == NULL)
      st_abort("Error while trying to capture the screen in OpenGL mode","");

    pixels = (unsigned char*) malloc(3 * screen->w * screen->h);
    if (pixels == NULL)
    {
      SDL_FreeSurface(temp);
      st_abort("Error while trying to capture the screen in OpenGL mode","");
    }

    glReadPixels(0, 0, screen->w, screen->h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    for (i=0; i<screen->h; i++)
      memcpy(((char *) temp->pixels) + temp->pitch * i, pixels + 3*screen->w * (screen->h-i-1), screen->w*3);
    free(pixels);

    cap_screen = new Surface(temp,false);
    SDL_FreeSurface(temp);

  }
#endif
  
return cap_screen;
}

SDL_Surface*
sdl_surface_part_from_file(const std::string& file, int x, int y, int w, int h,  int use_alpha)
{
  SDL_Rect src;
  SDL_Surface * sdl_surface;
  SDL_Surface * temp;
  SDL_Surface * conv;

  temp = IMG_Load(file.c_str());

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
    sdl_surface = SDL_DisplayFormat(conv);
  else
    sdl_surface = SDL_DisplayFormatAlpha(conv);

  if (sdl_surface == NULL)
    st_abort("Can't covert to display format", file);

  if (use_alpha == IGNORE_ALPHA && !use_gl)
    SDL_SetAlpha(sdl_surface, 0, 0);

  SDL_FreeSurface(temp);
  SDL_FreeSurface(conv);

  return sdl_surface;
}

SDL_Surface*
sdl_surface_from_file(const std::string& file, int use_alpha)
{
  SDL_Surface* sdl_surface;
  SDL_Surface* temp;

  temp = IMG_Load(file.c_str());

  if (temp == NULL)
    st_abort("Can't load", file);

  if(use_alpha == IGNORE_ALPHA && !use_gl)
    sdl_surface = SDL_DisplayFormat(temp);
  else
    sdl_surface = SDL_DisplayFormatAlpha(temp);

  if (sdl_surface == NULL)
    st_abort("Can't covert to display format", file);

  if (use_alpha == IGNORE_ALPHA && !use_gl)
    SDL_SetAlpha(sdl_surface, 0, 0);

  SDL_FreeSurface(temp);

  return sdl_surface;
}

SDL_Surface*
sdl_surface_from_sdl_surface(SDL_Surface* sdl_surf, int use_alpha)
{
  SDL_Surface* sdl_surface;
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
    sdl_surface = SDL_DisplayFormat(sdl_surf);
  else
    sdl_surface = SDL_DisplayFormatAlpha(sdl_surf);

  /* Restore the alpha blending attributes */
  if ( (saved_flags & SDL_SRCALPHA)
       == SDL_SRCALPHA )
  {
    SDL_SetAlpha(sdl_surface, saved_flags, saved_alpha);
  }

  if (sdl_surface == NULL)
    st_abort("Can't covert to display format", "SURFACE");

  if (use_alpha == IGNORE_ALPHA && !use_gl)
    SDL_SetAlpha(sdl_surface, 0, 0);

  return sdl_surface;
}

//---------------------------------------------------------------------------

SurfaceImpl::SurfaceImpl()
{}

SurfaceImpl::~SurfaceImpl()
{
  SDL_FreeSurface(sdl_surface);
}

SDL_Surface* SurfaceImpl::get_sdl_surface() const
{
  return sdl_surface;
}

int SurfaceImpl::resize(int w_, int h_)
{
  w = w_;
  h = h_;
  SDL_Rect dest;
  dest.x = 0;
  dest.y = 0;
  dest.w = w;
  dest.h = h;
  int ret = SDL_SoftStretch(sdl_surface, NULL,
                            sdl_surface, &dest);
  return ret;
}

#ifndef NOOPENGL
SurfaceOpenGL::SurfaceOpenGL(SDL_Surface* surf, int use_alpha)
{
  sdl_surface = sdl_surface_from_sdl_surface(surf, use_alpha);
  create_gl(sdl_surface,&gl_texture);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceOpenGL::SurfaceOpenGL(const std::string& file, int use_alpha)
{
  sdl_surface = sdl_surface_from_file(file, use_alpha);
  create_gl(sdl_surface,&gl_texture);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceOpenGL::SurfaceOpenGL(const std::string& file, int x, int y, int w, int h, int use_alpha)
{
  sdl_surface = sdl_surface_part_from_file(file,x,y,w,h,use_alpha);
  create_gl(sdl_surface, &gl_texture);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceOpenGL::~SurfaceOpenGL()
{
  glDeleteTextures(1, &gl_texture);
}

void
SurfaceOpenGL::create_gl(SDL_Surface * surf, GLuint * tex)
{
  Uint32 saved_flags;
  Uint8  saved_alpha;
  int w, h;
  SDL_Surface *conv;

  w = power_of_two(surf->w);
  h = power_of_two(surf->h),

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      conv = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, surf->format->BitsPerPixel,
                                  0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
      conv = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, surf->format->BitsPerPixel,
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

int
SurfaceOpenGL::draw(float x, float y, Uint8 alpha, bool update)
{
  float pw = power_of_two(w);
  float ph = power_of_two(h);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(alpha, alpha, alpha, alpha);

  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(x, y);
  glTexCoord2f((float)w / pw, 0);
  glVertex2f((float)w+x, y);
  glTexCoord2f((float)w / pw, (float)h / ph);  glVertex2f((float)w+x, (float)h+y);
  glTexCoord2f(0, (float)h / ph);
  glVertex2f(x, (float)h+y);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

  (void) update; // avoid compiler warning

  return 0;
}

int
SurfaceOpenGL::draw_bg(Uint8 alpha, bool update)
{
  float pw = power_of_two(w);
  float ph = power_of_two(h);

  glColor3ub(alpha, alpha, alpha);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(0, 0);
  glTexCoord2f((float)w / pw, 0);
  glVertex2f(screen->w, 0);
  glTexCoord2f((float)w / pw, (float)h / ph);
  glVertex2f(screen->w, screen->h);
  glTexCoord2f(0, (float)h / ph);
  glVertex2f(0, screen->h);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  (void) update; // avoid compiler warning

  return 0;
}

int
SurfaceOpenGL::draw_part(float sx, float sy, float x, float y, float w, float h, Uint8 alpha, bool update)
{
  float pw = power_of_two(int(this->w));
  float ph = power_of_two(int(this->h));

  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(alpha, alpha, alpha, alpha);

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

  (void) update; // avoid warnings
  return 0;
}

int
SurfaceOpenGL::draw_stretched(float x, float y, int sw, int sh, Uint8 alpha, bool update)
{
  float pw = power_of_two(int(this->w));
  float ph = power_of_two(int(this->h));

  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(alpha, alpha, alpha, alpha);

  glEnable(GL_TEXTURE_2D);


  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(x, y);
  glTexCoord2f((float)w / pw, 0);
  glVertex2f(sw+x, y);
  glTexCoord2f((float)w / pw, (float)h / ph);  glVertex2f((float)sw+x, (float)sh+y);
  glVertex2f(sw +x, sh+y);
  glTexCoord2f(0, (float)h / ph);
  glVertex2f(x, sh+y);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

  (void) update; // avoid warnings
  return 0;
}

#endif

SurfaceSDL::SurfaceSDL(SDL_Surface* surf, int use_alpha)
{
  sdl_surface = sdl_surface_from_sdl_surface(surf, use_alpha);
  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceSDL::SurfaceSDL(const std::string& file, int use_alpha)
{
  sdl_surface = sdl_surface_from_file(file, use_alpha);
  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceSDL::SurfaceSDL(const std::string& file, int x, int y, int w, int h,  int use_alpha)
{
  sdl_surface = sdl_surface_part_from_file(file, x, y, w, h, use_alpha);
  w = sdl_surface->w;
  h = sdl_surface->h;
}

int
SurfaceSDL::draw(float x, float y, Uint8 alpha, bool update)
{
  SDL_Rect dest;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = w;
  dest.h = h;

  if(alpha != 255)
    {
    /* Create a Surface, make it using colorkey, blit surface into temp, apply alpha
      to temp sur, blit the temp into the screen */
    /* Note: this has to be done, since SDL doesn't allow to set alpha to surfaces that
      already have an alpha mask yet... */

    SDL_Surface* sdl_surface_copy = SDL_CreateRGBSurface (sdl_surface->flags,
                                    sdl_surface->w, sdl_surface->h, sdl_surface->format->BitsPerPixel,
                                    sdl_surface->format->Rmask, sdl_surface->format->Gmask,
                                    sdl_surface->format->Bmask,
                                    0);
    int colorkey = SDL_MapRGB(sdl_surface_copy->format, 255, 0, 255);
    SDL_FillRect(sdl_surface_copy, NULL, colorkey);
    SDL_SetColorKey(sdl_surface_copy, SDL_SRCCOLORKEY, colorkey);


    SDL_BlitSurface(sdl_surface, NULL, sdl_surface_copy, NULL);
    SDL_SetAlpha(sdl_surface_copy ,SDL_SRCALPHA,alpha);

    int ret = SDL_BlitSurface(sdl_surface_copy, NULL, screen, &dest);

    if (update == UPDATE)
      SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);

    SDL_FreeSurface (sdl_surface_copy);
    return ret;
    }

  int ret = SDL_BlitSurface(sdl_surface, NULL, screen, &dest);

  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);

  return ret;
}

int
SurfaceSDL::draw_bg(Uint8 alpha, bool update)
{
  SDL_Rect dest;

  dest.x = 0;
  dest.y = 0;
  dest.w = screen->w;
  dest.h = screen->h;

  if(alpha != 255)
    {
    /* Create a Surface, make it using colorkey, blit surface into temp, apply alpha
      to temp sur, blit the temp into the screen */
    /* Note: this has to be done, since SDL doesn't allow to set alpha to surfaces that
      already have an alpha mask yet... */

    SDL_Surface* sdl_surface_copy = SDL_CreateRGBSurface (sdl_surface->flags,
                                    sdl_surface->w, sdl_surface->h, sdl_surface->format->BitsPerPixel,
                                    sdl_surface->format->Rmask, sdl_surface->format->Gmask,
                                    sdl_surface->format->Bmask,
                                    0);
    int colorkey = SDL_MapRGB(sdl_surface_copy->format, 255, 0, 255);
    SDL_FillRect(sdl_surface_copy, NULL, colorkey);
    SDL_SetColorKey(sdl_surface_copy, SDL_SRCCOLORKEY, colorkey);


    SDL_BlitSurface(sdl_surface, NULL, sdl_surface_copy, NULL);
    SDL_SetAlpha(sdl_surface_copy ,SDL_SRCALPHA,alpha);

    int ret = SDL_BlitSurface(sdl_surface_copy, NULL, screen, &dest);

    if (update == UPDATE)
      SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);

    SDL_FreeSurface (sdl_surface_copy);
    return ret;
    }

  int ret = SDL_SoftStretch(sdl_surface, NULL, screen, &dest);

  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);

  return ret;
}

int
SurfaceSDL::draw_part(float sx, float sy, float x, float y, float w, float h, Uint8 alpha, bool update)
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

  if(alpha != 255)
    {
    /* Create a Surface, make it using colorkey, blit surface into temp, apply alpha
      to temp sur, blit the temp into the screen */
    /* Note: this has to be done, since SDL doesn't allow to set alpha to surfaces that
      already have an alpha mask yet... */

    SDL_Surface* sdl_surface_copy = SDL_CreateRGBSurface (sdl_surface->flags,
                                    sdl_surface->w, sdl_surface->h, sdl_surface->format->BitsPerPixel,
                                    sdl_surface->format->Rmask, sdl_surface->format->Gmask,
                                    sdl_surface->format->Bmask,
                                    0);
    int colorkey = SDL_MapRGB(sdl_surface_copy->format, 255, 0, 255);
    SDL_FillRect(sdl_surface_copy, NULL, colorkey);
    SDL_SetColorKey(sdl_surface_copy, SDL_SRCCOLORKEY, colorkey);


    SDL_BlitSurface(sdl_surface, NULL, sdl_surface_copy, NULL);
    SDL_SetAlpha(sdl_surface_copy ,SDL_SRCALPHA,alpha);

    int ret = SDL_BlitSurface(sdl_surface_copy, NULL, screen, &dest);

    if (update == UPDATE)
      SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);

    SDL_FreeSurface (sdl_surface_copy);
    return ret;
    }

  int ret = SDL_BlitSurface(sdl_surface, &src, screen, &dest);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);

  return ret;
}

int
SurfaceSDL::draw_stretched(float x, float y, int sw, int sh, Uint8 alpha, bool update)
{
  SDL_Rect dest;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = (int)sw;
  dest.h = (int)sh;

  if(alpha != 255)
    SDL_SetAlpha(sdl_surface ,SDL_SRCALPHA,alpha);


  SDL_Surface* sdl_surface_copy = SDL_CreateRGBSurface (sdl_surface->flags,
                                  sw, sh, sdl_surface->format->BitsPerPixel,
                                  sdl_surface->format->Rmask, sdl_surface->format->Gmask,
                                  sdl_surface->format->Bmask,
                                  0);

  SDL_BlitSurface(sdl_surface, NULL, sdl_surface_copy, NULL);
  SDL_SoftStretch(sdl_surface_copy, NULL, sdl_surface_copy, &dest);

  int ret = SDL_BlitSurface(sdl_surface_copy,NULL,screen,&dest);
  SDL_FreeSurface(sdl_surface_copy);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);

  return ret;
}

SurfaceSDL::~SurfaceSDL()
{}

/* EOF */
