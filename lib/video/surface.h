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
#ifndef SUPERTUX_TEXTURE_H
#define SUPERTUX_TEXTURE_H

#include <string>
#include <list>

#ifndef NOOPENGL
#include <SDL_opengl.h>
#endif

#include "SDL.h"

#include "math/vector.h"
#include "video/screen.h"

namespace SuperTux
  {

  void apply_filter_to_surface(SDL_Surface *surface, int filter, int value);
  SDL_Surface* sdl_surface_from_sdl_surface(SDL_Surface* sdl_surf, bool use_alpha);
  SDL_Surface* sdl_surface_from_nothing();

  class SurfaceImpl;
  class SurfaceSDL;
  class SurfaceOpenGL;
  class DrawingContext;
  
  /// bitset for drawing effects
  enum {
    /** Don't apply anything */
    NONE_EFFECT       = 0x0000,
    /** Draw the Surface upside down */
    VERTICAL_FLIP     = 0x0001,
    /** Draw the Surface from left to down */
    HORIZONTAL_FLIP   = 0x0002,
    /** Draw the Surface with alpha equal to 128 */
    SEMI_TRANSPARENT  = 0x0004
  };

  /// types of filters
  enum {
    HORIZONTAL_FLIP_FILTER,
    MASK_FILTER,
    NONE_FILTER
  };

  /** This class holds all the data necessary to construct a surface */
  class SurfaceData
    {
    public:
      enum ConstructorType { LOAD, LOAD_PART, SURFACE, GRADIENT };
      ConstructorType type;
      SDL_Surface* surface;
      std::string file;

      struct Filter { int type; Color color; };
      std::vector<Filter> applied_filters;

      bool use_alpha;
      int x;
      int y;
      int w;
      int h;
      Color top_gradient;
      Color bottom_gradient;

      SurfaceData(SDL_Surface* surf, bool use_alpha_);
      SurfaceData(const std::string& file_, bool use_alpha_);
      SurfaceData(const std::string& file_, int x_, int y_, int w_, int h_, bool use_alpha_);
      SurfaceData(Color top_gradient_, Color bottom_gradient_, int w_, int h_);
      ~SurfaceData();

      SurfaceSDL* create_SurfaceSDL();
      SurfaceOpenGL* create_SurfaceOpenGL();
      SurfaceImpl* create();
    };


  /// Surface
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
      static void debug_check();

      Surface(SDL_Surface* surf, bool use_alpha);
      Surface(const std::string& file, bool use_alpha);
      Surface(const std::string& file, int x, int y, int w, int h, bool use_alpha);
      Surface(Color top_gradient, Color bottom_gradient, int w_, int h_);
      ~Surface();

      /** Reload the surface, which is necesarry in case of a mode swich */
      void reload();

      void apply_filter(int filter, Color color = Color(0,0,0));
    };

  /** Surface implementation, all implementation have to inherit from
      this class */
  class SurfaceImpl
    {
    protected:
      SDL_Surface* sdl_surface;

    public:
      int w;
      int h;

    public:
      SurfaceImpl();
      virtual ~SurfaceImpl();

      /** Return 0 on success, -2 if surface needs to be reloaded */
      virtual int draw(float x, float y, Uint8 alpha, Uint32 effect = NONE_EFFECT) = 0;
      virtual int draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, Uint32 effect = NONE_EFFECT) = 0;
      virtual int draw_stretched(float x, float y, int w, int h, Uint8 alpha, Uint32 effect = NONE_EFFECT) = 0;


      SDL_Surface* get_sdl_surface() const; // @evil@ try to avoid this function

      virtual void apply_filter(int filter, Color color = Color(0,0,0)) = 0;
    };

  class SurfaceSDL : public SurfaceImpl
    {
    public:
      SurfaceSDL(SDL_Surface* surf, bool use_alpha);
      SurfaceSDL(const std::string& file, bool use_alpha);
      SurfaceSDL(const std::string& file, int x, int y, int w, int h, bool use_alpha);
      SurfaceSDL(Color top_gradient, Color bottom_gradient, int w, int h);
      virtual ~SurfaceSDL();

      int draw(float x, float y, Uint8 alpha, Uint32 effect = NONE_EFFECT);
      int draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, Uint32 effect = NONE_EFFECT);
      int draw_stretched(float x, float y, int w, int h, Uint8 alpha, Uint32 effect = NONE_EFFECT);

      void apply_filter(int filter, Color color);
    };

#ifndef NOOPENGL
  class SurfaceOpenGL : public SurfaceImpl
    {
    public:
      GLuint gl_texture;

    public:
      SurfaceOpenGL(SDL_Surface* surf, bool use_alpha);
      SurfaceOpenGL(const std::string& file, bool use_alpha);
      SurfaceOpenGL(const std::string& file, int x, int y, int w, int h, bool use_alpha);
      SurfaceOpenGL(Color top_gradient, Color bottom_gradient, int w, int h);

      virtual ~SurfaceOpenGL();

      int draw(float x, float y, Uint8 alpha, Uint32 effect = NONE_EFFECT);
      int draw_part(float sx, float sy, float x, float y, float w, float h,  Uint8 alpha, Uint32 effect = NONE_EFFECT);
      int draw_stretched(float x, float y, int w, int h, Uint8 alpha, Uint32 effect = NONE_EFFECT);

      void apply_filter(int filter, Color color);

    private:
      void create_gl(SDL_Surface * surf, GLuint * tex);
    };
#endif

} //namespace SuperTux

#endif /*SUPERTUX_TEXTURE_H*/

/* Local Variables: */
/* mode: c++ */
/* End: */
