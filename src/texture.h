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

class SurfaceImpl;

/** Container class that holds a surface, necessary so that we can
    switch Surface implementations (OpenGL, SDL) on the fly */
class Surface
{
public:
  SurfaceImpl* impl;
  int w; 
  int h;
public:
  Surface(SDL_Surface* surf, int use_alpha);  
  Surface(const std::string& file, int use_alpha);  
  Surface(const std::string& file, int x, int y, int w, int h, int use_alpha);
  ~Surface();

  void draw(float x, float y, Uint8 alpha = 255, bool update = false);
  void draw_bg(Uint8 alpha = 255, bool update = false);
  void draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha = 255, bool update = false);
};

/** Surface implementation, all implementation have to inherit from
    this class */
class SurfaceImpl
{
public:
  SDL_Surface* sdl_surface;
  int w;
  int h;

public:
  virtual void draw(float x, float y, Uint8 alpha, bool update) = 0;
  virtual void draw_bg(Uint8 alpha, bool update) = 0;
  virtual void draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update) = 0;
};

class SurfaceSDL : public SurfaceImpl
{
public:
  
public:
  SurfaceSDL(SDL_Surface* surf, int use_alpha);
  SurfaceSDL(const std::string& file, int use_alpha);  
  SurfaceSDL(const std::string& file, int x, int y, int w, int h, int use_alpha);
  virtual ~SurfaceSDL();

  void draw(float x, float y, Uint8 alpha, bool update);
  void draw_bg(Uint8 alpha, bool update);
  void draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update);
};

class SurfaceOpenGL : public SurfaceImpl
{
public:
  unsigned gl_texture;

public:
  SurfaceOpenGL(SDL_Surface* surf, int use_alpha);
  SurfaceOpenGL(const std::string& file, int use_alpha);  
  SurfaceOpenGL(const std::string& file, int x, int y, int w, int h, int use_alpha);
  virtual ~SurfaceOpenGL();

  void draw(float x, float y, Uint8 alpha, bool update);
  void draw_bg(Uint8 alpha, bool update);
  void draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update);

private:
  void create_gl(SDL_Surface * surf, GLuint * tex);
};

#endif /*SUPERTUX_TEXTURE_H*/

/* Local Variables: */
/* mode: c++ */
/* End: */
