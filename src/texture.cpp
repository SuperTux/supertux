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
#include "texture.h"
#include "globals.h"
#include "setup.h"

/* Quick utility function for texture creation */
static int power_of_two(int input)
{
  int value = 1;

  while ( value < input ) {
    value <<= 1;
  }
  return value;
}

Surface::Surface(SDL_Surface* surf, int use_alpha)
{
  if (use_gl)
    impl = new SurfaceOpenGL(surf, use_alpha);
  else
    impl = new SurfaceSDL(surf, use_alpha);

  w = impl->w;
 h = impl->h;
}

Surface::Surface(const std::string& file, int use_alpha)
{
  if (use_gl)
    impl = new SurfaceOpenGL(file, use_alpha);
  else
    impl = new SurfaceSDL(file, use_alpha);

  w = impl->w;
  h = impl->h;
}

Surface::Surface(const std::string& file, int x, int y, int w, int h, int use_alpha)
{
  if (use_gl)
    impl = new SurfaceOpenGL(file, x, y, w, h, use_alpha);
  else
    impl = new SurfaceSDL(file, x, y, w, h, use_alpha);

  w = impl->w;
  h = impl->h;
}

Surface::~Surface()
{
  delete impl;
}

void
Surface::draw(float x, float y, Uint8 alpha, bool update)
{
  if (impl) impl->draw(x, y, alpha, update);
}

void
Surface::draw_bg(Uint8 alpha, bool update)
{
  if (impl) impl->draw_bg(alpha, update);
}

void
Surface::draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update)
{
  if (impl) impl->draw_part(sx, sy, x, y, w, h, alpha, update);
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
  SDL_FreeSurface(sdl_surface);
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

void
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
  
  /* Avoid compiler warnings */
  if(update)
    {}
}

void
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

  /* Avoid compiler warnings */
  if(update)
    {}
}

void
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

  /* Avoid compiler warnings */
  if(update)
    {}
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

void
SurfaceSDL::draw(float x, float y, Uint8 alpha, bool update)
{
  SDL_Rect dest;

  dest.x = (int)x;
  dest.y = (int)y;
  dest.w = w;
  dest.h = h;
  
  if(alpha != 255) /* SDL isn't capable of this kind of alpha :( therefore we'll leave now. */
  return;
  
  SDL_SetAlpha(sdl_surface ,SDL_SRCALPHA,alpha);
  SDL_BlitSurface(sdl_surface, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}

void
SurfaceSDL::draw_bg(Uint8 alpha, bool update)
{
  SDL_Rect dest;
  
  dest.x = 0;
  dest.y = 0;
  dest.w = screen->w;
  dest.h = screen->h;

  if(alpha != 255)
  SDL_SetAlpha(sdl_surface ,SDL_SRCALPHA,alpha);
  SDL_SoftStretch(sdl_surface, NULL, screen, &dest);
  
  if (update == UPDATE)
    SDL_UpdateRect(screen, dest.x, dest.y, dest.w, dest.h);
}

void
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
  SDL_SetAlpha(sdl_surface ,SDL_SRCALPHA,alpha);
  
  SDL_BlitSurface(sdl_surface, &src, screen, &dest);

  if (update == UPDATE)
    update_rect(screen, dest.x, dest.y, dest.w, dest.h);
}

SurfaceSDL::~SurfaceSDL()
{
  SDL_FreeSurface(sdl_surface);
}

/* EOF */
