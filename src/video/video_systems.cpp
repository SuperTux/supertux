//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <config.h>

#include "supertux/gameconfig.hpp"
#include "video/gl/gl_lightmap.hpp"
#include "video/gl/gl_renderer.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/sdl/sdl_lightmap.hpp"
#include "video/sdl/sdl_renderer.hpp"
#include "video/sdl/sdl_surface_data.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/texture.hpp"
#include "video/video_systems.hpp"

Renderer *new_renderer()
{
  switch(config->video)
  {
    case AUTO_VIDEO:
#ifdef HAVE_OPENGL
      log_info << "new GL renderer\n";
      return new GL::Renderer();
#else
      log_warning << "new SDL renderer\n";
      return new SDL::Renderer();
#endif
#ifdef HAVE_OPENGL
    case OPENGL:
      log_info << "new GL renderer\n";
      return new GL::Renderer();
#endif
    case PURE_SDL:
      log_warning << "new SDL renderer\n";
      return new SDL::Renderer();
    default:
      assert(0 && "invalid video system in config");
#ifdef HAVE_OPENGL
      log_info << "new GL renderer\n";
      return new GL::Renderer();
#else
      log_warning << "new SDL renderer\n";
      return new SDL::Renderer();
#endif
  }
}

Lightmap *new_lightmap()
{
  switch(config->video)
  {
    case AUTO_VIDEO:
#ifdef HAVE_OPENGL
      return new GL::Lightmap();
#else
      return new SDL::Lightmap();
#endif
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::Lightmap();
#endif
    case PURE_SDL:
      return new SDL::Lightmap();
    default:
      assert(0 && "invalid video system in config");
#ifdef HAVE_OPENGL
      return new GL::Lightmap();
#else
      return new SDL::Lightmap();
#endif
  }
}

Texture *new_texture(SDL_Surface *image)
{
  switch(config->video)
  {
    case AUTO_VIDEO:
#ifdef HAVE_OPENGL
      return new GL::Texture(image);
#else
      return new SDL::Texture(image);
#endif
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::Texture(image);
#endif
    case PURE_SDL:
      return new SDL::Texture(image);
    default:
      assert(0 && "invalid video system in config");
#ifdef HAVE_OPENGL
      return new GL::Texture(image);
#else
      return new SDL::Texture(image);
#endif
  }
}

void *new_surface_data(const Surface &surface)
{
  switch(config->video)
  {
    case AUTO_VIDEO:
#ifdef HAVE_OPENGL
      return new GL::SurfaceData(surface);
#else
      return new SDL::SurfaceData(surface);
#endif
#ifdef HAVE_OPENGL
    case OPENGL:
      return new GL::SurfaceData(surface);
#endif
    case PURE_SDL:
      return new SDL::SurfaceData(surface);
    default:
      assert(0 && "invalid video system in config");
#ifdef HAVE_OPENGL
      return new GL::SurfaceData(surface);
#else
      return new SDL::SurfaceData(surface);
#endif
  }
}

void free_surface_data(void *surface_data)
{
  delete reinterpret_cast<char *>(surface_data);
}

VideoSystem get_video_system(const std::string &video)
{
  if(video == "auto")
  {
    return AUTO_VIDEO;
  }
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
    return AUTO_VIDEO;
  }
}

std::string get_video_string(VideoSystem video)
{
  switch(video)
  {
    case AUTO_VIDEO:
      return "auto";
    case OPENGL:
      return "opengl";
    case PURE_SDL:
      return "sdl";
    default:
      assert(0 && "invalid video system in config");
      return "auto";
  }
}

/* EOF */
