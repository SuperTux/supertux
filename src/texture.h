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

#include <list>
#include "screen.h"

class SurfaceImpl;
class SurfaceSDL;
class SurfaceOpenGL;

/** This class holds all the data necessary to construct a surface */
class SurfaceData 
{
public:
  enum ConstructorType { LOAD, LOAD_PART, SURFACE };
  ConstructorType type;
  SDL_Surface* surface;
  std::string file;
  int use_alpha;
  int x;
  int y;
  int w;
  int h;

  SurfaceData(SDL_Surface* surf, int use_alpha_);
  SurfaceData(const std::string& file_, int use_alpha_);
  SurfaceData(const std::string& file_, int x_, int y_, int w_, int h_, int use_alpha_);
  ~SurfaceData();

  SurfaceSDL* create_SurfaceSDL();
  SurfaceOpenGL* create_SurfaceOpenGL();
  SurfaceImpl* create();
};

/** Container class that holds a surface, necessary so that we can
    switch Surface implementations (OpenGL, SDL) on the fly */
class Surface
{
public:
  SurfaceData data;
  SurfaceImpl* impl;
  int w; 
  int h;
  
  typedef std::list<Surface*> Surfaces;
  static Surfaces surfaces;
public:
  static void reload_all();

  Surface(SDL_Surface* surf, int use_alpha);  
  Surface(const std::string& file, int use_alpha);  
  Surface(const std::string& file, int x, int y, int w, int h, int use_alpha);
  ~Surface();
  
  /** Reload the surface, which is necesarry in case of a mode swich */
  void reload();

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
  /** Return 0 on success, -2 if surface needs to be reloaded */
  virtual int draw(float x, float y, Uint8 alpha, bool update) = 0;
  virtual int draw_bg(Uint8 alpha, bool update) = 0;
  virtual int draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update) = 0;
};

class SurfaceSDL : public SurfaceImpl
{
public:
  
public:
  SurfaceSDL(SDL_Surface* surf, int use_alpha);
  SurfaceSDL(const std::string& file, int use_alpha);  
  SurfaceSDL(const std::string& file, int x, int y, int w, int h, int use_alpha);
  virtual ~SurfaceSDL();

  int draw(float x, float y, Uint8 alpha, bool update);
  int draw_bg(Uint8 alpha, bool update);
  int draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update);
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

  int draw(float x, float y, Uint8 alpha, bool update);
  int draw_bg(Uint8 alpha, bool update);
  int draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, bool update);

private:
  void create_gl(SDL_Surface * surf, GLuint * tex);
};

#endif /*SUPERTUX_TEXTURE_H*/

/* Local Variables: */
/* mode: c++ */
/* End: */
