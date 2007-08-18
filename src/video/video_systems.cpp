//  $Id: video_systems.cpp 5063 2007-05-27 11:32:00Z matzeb $
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include "video_systems.hpp"
#include "gameconfig.hpp"
#include "renderer.hpp"
#include "gl_renderer.hpp"
#include "sdl_renderer.hpp"
#include "lightmap.hpp"
#include "gl_lightmap.hpp"
#include "sdl_lightmap.hpp"
#include "texture.hpp"
#include "gl_texture.hpp"
#include "sdl_texture.hpp"
#include "gl_surface_data.hpp"
#include "sdl_surface_data.hpp"

Renderer *new_renderer()
{
  switch(config->video)
  {
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::Renderer();
#endif
    case PURE_SDL:
      return new SDL::Renderer();
    default:
      assert(0 && "invalid video system in config");
      return new SDL::Renderer();
  }
}

Lightmap *new_lightmap()
{
  switch(config->video)
  {
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::Lightmap();
#endif
    case PURE_SDL:
      return new SDL::Lightmap();
    default:
      assert(0 && "invalid video system in config");
      return new SDL::Lightmap();
  }
}

Texture *new_texture(SDL_Surface *image)
{
  switch(config->video)
  {
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::Texture(image);
#endif
    case PURE_SDL:
      return new SDL::Texture(image);
    default:
      assert(0 && "invalid video system in config");
      return new SDL::Texture(image);
  }
}

void *new_surface_data(const Surface &surface)
{
  switch(config->video)
  {
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::SurfaceData(surface);
#endif
    case PURE_SDL:
      return new SDL::SurfaceData(surface);
    default:
      assert(0 && "invalid video system in config");
      return new SDL::SurfaceData(surface);
  }
}

void free_surface_data(void *surface_data)
{
  delete reinterpret_cast<char *>(surface_data);
}

VideoSystem get_video_system(const std::string &video)
{
  if(0) {}
#ifdef HAVE_OPENGL
  else if(video == "opengl")
  {
    return OPENGL;
  }
#endif
  else if(video == "sdl")
  {
    return PURE_SDL;
  }
  else
  {
#ifdef HAVE_OPENGL
    return OPENGL;
#else
    return PURE_SDL;
#endif
  }
}

std::string get_video_string(VideoSystem video)
{
  switch(video)
  {
    case OPENGL:
      return "opengl";
    case PURE_SDL:
      return "sdl";
    default:
      assert(0 && "invalid video system in config");
      return "sdl";
  }
}
