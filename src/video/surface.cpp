//  $Id: surface.cpp 2175 2004-11-24 19:02:49Z sik0fewl $
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
#include <config.h>

#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sstream>

#include <SDL.h>
#include <SDL_image.h>

#include "gameconfig.h"
#include "physfs/physfs_sdl.h"
#include "video/surface.h"
#include "video/screen.h"

Surface::Surfaces Surface::surfaces;

extern SDL_Surface* screen;

SurfaceData::SurfaceData(SDL_Surface* temp, bool use_alpha_)
    : type(SURFACE), surface(0), use_alpha(use_alpha_),
      x(0), y(0), w(0), h(0)
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
    throw std::runtime_error("No memory left for surface");
  
  SDL_SetAlpha(temp,0,0);
  SDL_BlitSurface(temp, NULL, surface, NULL);
}

SurfaceData::SurfaceData(const std::string& file_, bool use_alpha_)
    : type(LOAD), surface(0), file(file_), use_alpha(use_alpha_)
{}

SurfaceData::SurfaceData(const std::string& file_, int x_, int y_,
    int w_, int h_, bool use_alpha_)
    : type(LOAD_PART), surface(0), file(file_), use_alpha(use_alpha_),
    x(x_), y(y_), w(w_), h(h_)
{}

SurfaceData::SurfaceData(Color top_gradient_, Color bottom_gradient_,
    int w_, int h_)
    : type(GRADIENT), surface(0), use_alpha(false), w(w_), h(h_)
{
  top_gradient = top_gradient_;
  bottom_gradient = bottom_gradient_;
}


SurfaceData::~SurfaceData()
{
  SDL_FreeSurface(surface);
}

SurfaceImpl*
SurfaceData::create()
{
  if (config->use_gl)
    return create_SurfaceOpenGL();
  else
    return create_SurfaceSDL();
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
  case GRADIENT:
    return new SurfaceSDL(top_gradient, bottom_gradient, w, h);
  }
  assert(0);
}

SurfaceOpenGL*
SurfaceData::create_SurfaceOpenGL()
{
  switch(type)
  {
    case LOAD:
      return new SurfaceOpenGL(file, use_alpha);
    case LOAD_PART:
      return new SurfaceOpenGL(file, x, y, w, h, use_alpha);
    case SURFACE:
      return new SurfaceOpenGL(surface, use_alpha);
    case GRADIENT:
      return new SurfaceOpenGL(top_gradient, bottom_gradient, w, h);
    default:
      assert(false);
  }
}

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

Surface::Surface(SDL_Surface* surf, bool use_alpha)
    : impl(0), data(surf, use_alpha), w(0), h(0)
{
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
  surfaces.push_back(this);
}

Surface::Surface(const std::string& file, bool use_alpha)
    : impl(0), data(file, use_alpha), w(0), h(0)
{
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
  surfaces.push_back(this);
}

Surface::Surface(const std::string& file, int x, int y, int w_, int h_, bool use_alpha)
    : impl(0), data(file, x, y, w_, h_, use_alpha), w(0), h(0)
{
  impl = data.create();
  if (impl)
  {
    w = impl->w;
    h = impl->h;
  }
  surfaces.push_back(this);
}

Surface::Surface(Color top_background, Color bottom_background, int w_, int h_)
    : impl(0), data(top_background, bottom_background, w_, h_), w(0), h(0)
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
    for(std::vector<SurfaceData::Filter>::iterator i =
        data.applied_filters.begin(); i != data.applied_filters.end();
        i++)
      impl->apply_filter(i->type, i->color);
  }
}

void Surface::apply_filter(int filter, Color color)
{
  impl->apply_filter(filter, color);

  SurfaceData::Filter apply_filter;
  apply_filter.type = filter;
  apply_filter.color = color;
  data.applied_filters.push_back(apply_filter);
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
apply_filter_to_surface(SDL_Surface* surface, int filter, Color color)
{
  if(filter == HORIZONTAL_FLIP_FILTER) {
    SDL_Surface* sur_copy = sdl_surface_from_sdl_surface(surface, true);
    SDL_BlitSurface(surface, NULL, sur_copy, NULL);
    SDL_SetAlpha(sur_copy,0,0);

    SDL_Rect src, dst;
    src.y = dst.y = 0;
    src.w = dst.w = 1;
    src.h = dst.h = sur_copy->h;
    for(int x = 0; x < sur_copy->w; x++)
    {
      src.x = x; dst.x = sur_copy->w-1 - x;
      SDL_BlitSurface(sur_copy, &src, surface, &dst);
    }

    SDL_FreeSurface(sur_copy);
  } else if(filter == MASK_FILTER) {
    SDL_Surface* sur_copy = sdl_surface_from_sdl_surface(surface, true);

    Uint8 r,g,b,a;

    SDL_LockSurface(sur_copy);
    for(int x = 0; x < sur_copy->w; x++)
      for(int y = 0; y < sur_copy->h; y++) {
        SDL_GetRGBA(getpixel(sur_copy,x,y), sur_copy->format, &r,&g,&b,&a);
        if(a != 0) {
          putpixel(sur_copy, x,y, color.map_rgba(sur_copy));
        }
      }
    SDL_UnlockSurface(sur_copy);

    SDL_BlitSurface(sur_copy, NULL, surface, NULL);
    SDL_FreeSurface(sur_copy);
  }
}

SDL_Surface*
sdl_surface_part_from_file(const std::string& file, int x, int y, int w, int h,  bool use_alpha)
{
  SDL_Rect src;
  SDL_Surface * sdl_surface;
  SDL_Surface * temp;
  SDL_Surface * conv;

  temp = IMG_Load_RW(get_physfs_SDLRWops(file), true);
  if (temp == 0) {
    std::stringstream msg;
    msg << "Couldn't load '" << file << "': " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  
  /* Set source rectangle for conv: */

  src.x = x;
  src.y = y;
  src.w = w;
  src.h = h;

  conv = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, temp->format->BitsPerPixel,
                              temp->format->Rmask,
                              temp->format->Gmask,
                              temp->format->Bmask,
                              temp->format->Amask);

  SDL_SetAlpha(temp,0,0);

  SDL_BlitSurface(temp, &src, conv, NULL);
  if(use_alpha == false && !config->use_gl)
    sdl_surface = SDL_DisplayFormat(conv);
  else
    sdl_surface = SDL_DisplayFormatAlpha(conv);

  if (sdl_surface == NULL) {
    std::stringstream msg;
    msg << "Can't convert file '" << file << "' to display format.";
    throw std::runtime_error(msg.str());
  }

  if (use_alpha == false && !config->use_gl)
    SDL_SetAlpha(sdl_surface, 0, 0);

  SDL_FreeSurface(temp);
  SDL_FreeSurface(conv);

  return sdl_surface;
}

SDL_Surface*
sdl_surface_from_file(const std::string& file, bool use_alpha)
{
  SDL_Surface* sdl_surface;
  SDL_Surface* temp;

  temp = IMG_Load_RW(get_physfs_SDLRWops(file), true);
  if (temp == 0) {
    std::stringstream msg;
    msg << "Couldn't load file '" << file << "': " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  if(use_alpha == false && !config->use_gl)
    sdl_surface = SDL_DisplayFormat(temp);
  else
    sdl_surface = SDL_DisplayFormatAlpha(temp);

  if (sdl_surface == NULL) {
    std::stringstream msg;
    msg << "Couldn't convert file '" << file << "' to display format";
    throw std::runtime_error(msg.str());
  }

  if (use_alpha == false && !config->use_gl)
    SDL_SetAlpha(sdl_surface, 0, 0);

  SDL_FreeSurface(temp);

  return sdl_surface;
}

SDL_Surface*
sdl_surface_from_sdl_surface(SDL_Surface* sdl_surf, bool use_alpha)
{
  SDL_Surface* sdl_surface;
#if 0
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
#endif

  if(use_alpha == false && !config->use_gl)
    sdl_surface = SDL_DisplayFormat(sdl_surf);
  else
    sdl_surface = SDL_DisplayFormatAlpha(sdl_surf);

#if 0
  /* Restore the alpha blending attributes */
  if ( (saved_flags & SDL_SRCALPHA)
       == SDL_SRCALPHA )
  {
    SDL_SetAlpha(sdl_surface, saved_flags, saved_alpha);
  }
#endif

  if (sdl_surface == NULL) {
    std::stringstream msg;
    msg << "Can't convert surface to display format.";
    throw std::runtime_error(msg.str());
  }

  if (use_alpha == false && !config->use_gl)
    SDL_SetAlpha(sdl_surface, 0, 0);

  return sdl_surface;
}

SDL_Surface*
sdl_surface_from_gradient(Color top, Color bottom, int w, int h)
{
  SDL_Surface* sdl_surface
    = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
                    screen->format->BitsPerPixel, screen->format->Rmask,
                    screen->format->Gmask, screen->format->Bmask, 0);

  if(sdl_surface == 0)
    throw std::runtime_error("Can't create gradient surface");

  if(top == bottom) {
    SDL_FillRect(sdl_surface, NULL, SDL_MapRGB(sdl_surface->format,
        top.red, top.green, top.blue));
  } else {
    float redstep = (float(bottom.red)-float(top.red)) / float(h);
    float greenstep = (float(bottom.green)-float(top.green)) / float(h);
    float bluestep = (float(bottom.blue) - float(top.blue)) / float(h);

    SDL_Rect rect;
    rect.x = 0;
    rect.w = w;
    rect.h = 1;
    for(float y = 0; y < h; y++) {
      rect.y = (int)y;
      SDL_FillRect(sdl_surface, &rect, SDL_MapRGB(sdl_surface->format,
            int(float(top.red) + redstep * y),
            int(float(top.green) + greenstep * y),
            int(float(top.blue) + bluestep * y)));
    }
  }

  return sdl_surface;
}

//---------------------------------------------------------------------------

SurfaceImpl::SurfaceImpl()
  : sdl_surface(0)
{}

SurfaceImpl::~SurfaceImpl()
{
  if(sdl_surface != 0)
    SDL_FreeSurface(sdl_surface);
}

SDL_Surface* SurfaceImpl::get_sdl_surface() const
{
  return sdl_surface;
}

SurfaceOpenGL::SurfaceOpenGL(SDL_Surface* surf, bool use_alpha)
{
  sdl_surface = sdl_surface_from_sdl_surface(surf, use_alpha);
  create_gl(sdl_surface,&gl_texture);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceOpenGL::SurfaceOpenGL(const std::string& file, bool use_alpha)
{
  sdl_surface = sdl_surface_from_file(file, use_alpha);
  create_gl(sdl_surface,&gl_texture);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceOpenGL::SurfaceOpenGL(const std::string& file_, int x_, int y_,
    int w_, int h_, bool use_alpha_)
{
  sdl_surface = sdl_surface_part_from_file(file_,x_,y_,w_,h_,use_alpha_);
  
  create_gl(sdl_surface, &gl_texture);
  w = sdl_surface->w;
  h = sdl_surface->h;  
}

SurfaceOpenGL::SurfaceOpenGL(Color top_gradient, Color bottom_gradient,
    int _w, int _h)
{
  sdl_surface = sdl_surface_from_gradient(top_gradient, bottom_gradient,_w,_h);
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

  // We check all the pixels of the surface to figure out which
  // internal format OpenGL should use for storing it, ie. if no alpha
  // is present store in RGB instead of RGBA, this saves a few bytes
  // of memory, but much more importantly it makes the game look
  // *much* better in 16bit color mode
  int internal_format = GL_RGBA;
  bool has_alpha = false;

  unsigned char* buf = static_cast<unsigned char*>(conv->pixels);
  for (int y = 0; y < surf->h; ++y)
    for (int x = 0; x < surf->w; ++x)
      {
        if (buf[(conv->pitch*y + x*4) + 3] != 255)
          {
            has_alpha = true;
            break;
          }
      }

  if (!has_alpha)
    {
      internal_format = GL_RGB;
    }

  glGenTextures(1, &*tex);
  glBindTexture(GL_TEXTURE_2D , *tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, conv->pitch / conv->format->BytesPerPixel);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, conv->pixels);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  SDL_FreeSurface(conv);
}

int
SurfaceOpenGL::draw(float x, float y, Uint8 alpha, Uint32 effect)
{
  float pw = power_of_two(w);
  float ph = power_of_two(h);

  if(effect & SEMI_TRANSPARENT)
    alpha = 128;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(alpha, alpha, alpha, alpha);

  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glBegin(GL_QUADS);

  if(effect & VERTICAL_FLIP & HORIZONTAL_FLIP)
    {
    glTexCoord2f(0, 0);
    glVertex2f((float)w+x, (float)h+y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f(x, (float)h+y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f(x, y);

    glTexCoord2f(0, (float)h / ph);
    glVertex2f((float)w+x, y);
    }
  else if(effect & VERTICAL_FLIP)
    {
    glTexCoord2f(0, 0);
    glVertex2f(x, (float)h+y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f((float)w+x, (float)h+y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f((float)w+x, y);
    
    glTexCoord2f(0, (float)h / ph);
    glVertex2f(x, y);
    }
  else if(effect & HORIZONTAL_FLIP)
    {
    glTexCoord2f(0, 0);
    glVertex2f((float)w+x, y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f(x, y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f(x, (float)h+y);

    glTexCoord2f(0, (float)h / ph);
    glVertex2f((float)w+x, (float)h+y);
    }
  else
    {
    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f((float)w+x, y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f((float)w+x, (float)h+y);

    glTexCoord2f(0, (float)h / ph);
    glVertex2f(x, (float)h+y);
    }
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

  return 0;
}

int
SurfaceOpenGL::draw_part(float sx, float sy, float x, float y, float w, float h, Uint8 alpha, Uint32 effect)
{
  float pw = power_of_two(int(this->w));
  float ph = power_of_two(int(this->h));

  if(effect & SEMI_TRANSPARENT)
    alpha = 128;

  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(alpha, alpha, alpha, alpha);

  glEnable(GL_TEXTURE_2D);


  glBegin(GL_QUADS);

  if(effect & VERTICAL_FLIP & HORIZONTAL_FLIP)
    {
    glTexCoord2f(sx / pw, (float)(sy+h) / ph);
    glVertex2f((float)w+x, (float)h+y);

    glTexCoord2f((sx+w) / pw, (sy+h) / ph);
    glVertex2f(x, (float)h+y);

    glTexCoord2f((float)(sx + w) / pw, sy / ph);
    glVertex2f(x, y);

    glTexCoord2f(sx / pw, sy / ph);
    glVertex2f((float)w+x, y);
    }
  else if(effect & VERTICAL_FLIP)
    {
    glTexCoord2f(sx / pw, sy / ph);
    glVertex2f(x, y);

    glTexCoord2f((float)(sx + w) / pw, sy / ph);
    glVertex2f(w+x, y);

    glTexCoord2f((sx+w) / pw, (sy+h) / ph);
    glVertex2f(w +x, h+y);

    glTexCoord2f(sx / pw, (float)(sy+h) / ph);
    glVertex2f(x, h+y);
    }
  else if(effect & HORIZONTAL_FLIP)
    {
    glTexCoord2f(sx / pw, sy / ph);
    glVertex2f((float)w+x, y);

    glTexCoord2f((float)(sx + w) / pw, sy / ph);
    glVertex2f(x, y);

    glTexCoord2f((sx+w) / pw, (sy+h) / ph);
    glVertex2f(x, (float)h+y);

    glTexCoord2f(sx / pw, (float)(sy+h) / ph);
    glVertex2f((float)w+x, (float)h+y);
    }
  else
    {
    glTexCoord2f(sx / pw, (float)(sy+h) / ph);
    glVertex2f(x, h+y);

    glTexCoord2f((sx+w) / pw, (sy+h) / ph);
    glVertex2f(w +x, h+y);

    glTexCoord2f((float)(sx + w) / pw, sy / ph);
    glVertex2f(w+x, y);

    glTexCoord2f(sx / pw, sy / ph);
    glVertex2f(x, y);
    }

  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

  return 0;
}

int
SurfaceOpenGL::draw_stretched(float x, float y, int sw, int sh, Uint8 alpha, Uint32 effect)
{
  float pw = power_of_two(sw);
  float ph = power_of_two(sh);

  if(effect & SEMI_TRANSPARENT)
    alpha = 128;

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4ub(alpha, alpha, alpha, alpha);

  glBindTexture(GL_TEXTURE_2D, gl_texture);

  glBegin(GL_QUADS);

  if(effect & VERTICAL_FLIP & HORIZONTAL_FLIP)
    {
    glTexCoord2f(0, 0);
    glVertex2f((float)sw+x, (float)sh+y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f(x, (float)sh+y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f(x, y);

    glTexCoord2f(0, (float)h / ph);
    glVertex2f((float)sw+x, y);
    }
  else if(effect & VERTICAL_FLIP)
    {
    glTexCoord2f(0, 0);
    glVertex2f(x, (float)sh+y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f((float)sw+x, (float)sh+y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f((float)sw+x, y);
    
    glTexCoord2f(0, (float)h / ph);
    glVertex2f(x, y);
    }
  else if(effect & HORIZONTAL_FLIP)
    {
    glTexCoord2f(0, 0);
    glVertex2f((float)sw+x, y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f(x, y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f(x, (float)sh+y);

    glTexCoord2f(0, (float)h / ph);
    glVertex2f((float)sw+x, (float)sh+y);
    }
  else
    {
    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f((float)w / pw, 0);
    glVertex2f((float)sw+x, y);

    glTexCoord2f((float)w / pw, (float)h / ph);
    glVertex2f((float)sw+x, (float)sh+y);

    glTexCoord2f(0, (float)h / ph);
    glVertex2f(x, (float)sh+y);
    }
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);

  return 0;
}

void
SurfaceOpenGL::apply_filter(int filter, Color color)
{
  ::apply_filter_to_surface(sdl_surface, filter, color);
  create_gl(sdl_surface,&gl_texture);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceSDL::SurfaceSDL(SDL_Surface* surf, bool use_alpha)
{
  sdl_surface = sdl_surface_from_sdl_surface(surf, use_alpha);
  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceSDL::SurfaceSDL(const std::string& file, bool use_alpha)
{
  sdl_surface = sdl_surface_from_file(file, use_alpha);
  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceSDL::SurfaceSDL(const std::string& file, int x, int y, int _w, int _h,
    bool use_alpha)
{
  sdl_surface = sdl_surface_part_from_file(file, x, y, _w, _h, use_alpha);
  w = sdl_surface->w;
  h = sdl_surface->h;  
}

SurfaceSDL::SurfaceSDL(Color top_gradient, Color bottom_gradient,
    int _w, int _h)
{
  sdl_surface = sdl_surface_from_gradient(top_gradient, bottom_gradient,_w,_h);
  w = sdl_surface->w;
  h = sdl_surface->h;  
}

int
SurfaceSDL::draw(float x, float y, Uint8 alpha, Uint32 effect)
{
  SDL_Rect dest;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = w;
  dest.h = h;

  if(effect & SEMI_TRANSPARENT)
    alpha = 128;

  if(effect & VERTICAL_FLIP & HORIZONTAL_FLIP)
    {
    // FIXME: this hack is damn slow. Just keep it cause it isn't that used.
    for(float sx = 0; sx < w; sx++)
      for(float sy = 0; sy < h; sy++)
        if(draw_part(sx, sy, x+(w-sx), y+(h-sy), 1, 1, alpha, NONE_EFFECT) == -2)
          return -2;
    return 0;
    }
  else if(effect & VERTICAL_FLIP)    // FIXME: feel free to replace this hack
    {
    for(float sy = 0; sy < h; sy++)
      if(draw_part(0, sy, x, y+(h-sy), w, 1, alpha, NONE_EFFECT) == -2)
        return -2;
    return 0;
    }
  else if(effect & HORIZONTAL_FLIP)    // FIXME: feel free to replace this hack
    {
    for(float sx = 0; sx < w; sx++)
      if(draw_part(sx, 0, x+(w-sx), y, 1, h, alpha, NONE_EFFECT) == -2)
        return -2;
    return 0;
    }

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

    SDL_FreeSurface (sdl_surface_copy);
    return ret;
    }

  int ret = SDL_BlitSurface(sdl_surface, NULL, screen, &dest);

  return ret;
}

int
SurfaceSDL::draw_part(float sx, float sy, float x, float y, float w, float h, Uint8 alpha, Uint32 effect)
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

  if(effect & SEMI_TRANSPARENT)
    alpha = 128;

  if(effect & VERTICAL_FLIP & HORIZONTAL_FLIP)
    {
    // FIXME: this hack is damn slow. Just keep it cause it isn't that used.
    for(float sx_ = 0; sx_ < w; sx++)
      for(float sy_ = 0; sy_ < h; sy++)
        if(draw_part(sx_, sy_, sx+(w-sx_), sy+(h-sy_), 1, 1, alpha, NONE_EFFECT) == -2)
          return -2;
    return 0;
    }
  else if(effect & VERTICAL_FLIP)    // FIXME: feel free to replace this hack
    {
    for(float sy_ = sy; sy_ < h; sy_++)
      if(draw_part(sx, sy_, x, y+(h-sy_), w, 1, alpha, NONE_EFFECT) == -2)
        return -2;
    return 0;
    }
  else if(effect & HORIZONTAL_FLIP)    // FIXME: feel free to replace this hack
    {
    for(float sx_ = 0; sx_ < w; sx_++)
      if(draw_part(sx_, 0, sx+(w-sx_), sy, 1, h, alpha, NONE_EFFECT) == -2)
        return -2;
    return 0;
    }

  if(alpha != 255)
    {
    /* Create a Surface, make it using colorkey, blit surface into temp, apply alpha
      to temp sur, blit the temp into the screen */
    /* Note: this has to be done, since SDL doesn't allow to set alpha to surfaces that
      already have an alpha mask, yet... */

    SDL_Surface* sdl_surface_copy = SDL_CreateRGBSurface (sdl_surface->flags,
                                    (int)w, (int)h, sdl_surface->format->BitsPerPixel,
                                    sdl_surface->format->Rmask, sdl_surface->format->Gmask,
                                    sdl_surface->format->Bmask,
                                    0);
    int colorkey = SDL_MapRGB(sdl_surface_copy->format, 255, 0, 255);
    SDL_FillRect(sdl_surface_copy, NULL, colorkey);
    SDL_SetColorKey(sdl_surface_copy, SDL_SRCCOLORKEY, colorkey);


    SDL_BlitSurface(sdl_surface, &src, sdl_surface_copy, NULL);
    SDL_SetAlpha(sdl_surface_copy ,SDL_SRCALPHA,alpha);

    int ret = SDL_BlitSurface(sdl_surface_copy, NULL, screen, &dest);

    SDL_FreeSurface (sdl_surface_copy);
    return ret;
    }

  int ret = SDL_BlitSurface(sdl_surface, &src, screen, &dest);

  return ret;
}

int
SurfaceSDL::draw_stretched(float x, float y, int sw, int sh, Uint8 alpha, Uint32 effect)
{
  SDL_Rect dest;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = (int)sw;
  dest.h = (int)sh;

  if(effect & SEMI_TRANSPARENT)
    alpha = 128;

  SDL_Surface* sdl_surface_copy = SDL_CreateRGBSurface (sdl_surface->flags,
                                  sw, sh, sdl_surface->format->BitsPerPixel,
                                  sdl_surface->format->Rmask, sdl_surface->format->Gmask,
                                  sdl_surface->format->Bmask,
                                  0);

  SDL_BlitSurface(sdl_surface, NULL, sdl_surface_copy, NULL);
  SDL_SoftStretch(sdl_surface_copy, NULL, sdl_surface_copy, &dest);

  if(alpha != 255)
    SDL_SetAlpha(sdl_surface_copy,SDL_SRCALPHA,alpha);

  int ret = SDL_BlitSurface(sdl_surface_copy,NULL,screen,&dest);
  SDL_FreeSurface(sdl_surface_copy);

  return ret;
}

void
SurfaceSDL::apply_filter(int filter, Color color)
{
  ::apply_filter_to_surface(sdl_surface, filter, color);

  w = sdl_surface->w;
  h = sdl_surface->h;
}

SurfaceSDL::~SurfaceSDL()
{}
