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

#include "video/video_system.hpp"

#include <assert.h>
#include <boost/optional.hpp>
#include <config.h>
#include <iomanip>
#include <physfs.h>
#include <sstream>
#include <savepng.h>

#include "util/log.hpp"
#include "video/sdl/sdl_video_system.hpp"

#ifdef HAVE_OPENGL
#  include "video/gl/gl_video_system.hpp"
#endif

std::unique_ptr<VideoSystem>
VideoSystem::create(VideoSystem::Enum video_system)
{
  switch(video_system)
  {
    case AUTO_VIDEO:
#ifdef HAVE_OPENGL
      try
      {
        return std::unique_ptr<VideoSystem>(new GLVideoSystem);
      }
      catch(std::exception& err)
      {
        log_warning << "Error creating GLVideoSystem, using SDL fallback: "  << err.what() << std::endl;
        return std::unique_ptr<VideoSystem>(new SDLVideoSystem);
      }
#else
      log_info << "new SDL renderer\n";
      return std::unique_ptr<VideoSystem>(new SDLVideoSystem);
#endif

    case OPENGL:
#ifdef HAVE_OPENGL
      return std::unique_ptr<VideoSystem>(new GLVideoSystem);
#else
      log_warning << "OpenGL requested, but missing using SDL fallback" << std::endl;
      return std::unique_ptr<VideoSystem>(new SDLVideoSystem);
#endif

    case PURE_SDL:
      log_info << "new SDL renderer\n";
      return std::unique_ptr<VideoSystem>(new SDLVideoSystem);

    default:
      log_fatal << "invalid video system in config" << std::endl;
      assert(false);
      return {};
  }
}

VideoSystem::Enum
VideoSystem::get_video_system(const std::string &video)
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
#ifdef HAVE_OPENGL
    throw std::runtime_error("invalid VideoSystem::Enum, valid values are 'auto', 'sdl' and 'opengl'");
#else
    throw std::runtime_error("invalid VideoSystem::Enum, valid values are 'auto' and 'sdl'");
#endif
  }
}

std::string
VideoSystem::get_video_string(VideoSystem::Enum video)
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
      log_fatal << "invalid video system in config" << std::endl;
      assert(false);
      return "auto";
  }
}

void
VideoSystem::do_take_screenshot()
{
  SDL_Surface* surface = make_screenshot();
  if (!surface) {
    log_warning << "Creating the screenshot has failed" << std::endl;
    return;
  }

  auto find_filename = [&]() -> boost::optional<std::string>
    {
      const std::string writeDir = PHYSFS_getWriteDir();
      const std::string dirSep = PHYSFS_getDirSeparator();
      const std::string baseName = "screenshot";
      const std::string fileExt = ".png";

      for (int num = 0; num < 1000000; ++num)
      {
        std::ostringstream oss;
        oss << baseName << std::setw(6) << std::setfill('0') << num << fileExt;
        if (!PHYSFS_exists(oss.str().c_str()))
        {
          std::ostringstream fullpath;
          fullpath << writeDir << dirSep << oss.str();
          return fullpath.str();
        }
      }

      return boost::none;
    };

  auto filename = find_filename();
  if (!filename)
  {
    log_info << "Failed to find filename to save screenshot" << std::endl;
  }
  else
  {
    SDL_Surface* tmp = SDL_PNGFormatAlpha(surface);
    if (SDL_SavePNG(tmp, filename->c_str()))
    {
      log_warning << "Saving screenshot failed: " << SDL_GetError() << std::endl;
    }
    else
    {
      log_info << "Wrote screenshot to \"" << *filename << "\"" << std::endl;
    }

    if (tmp != surface)
    {
      SDL_FreeSurface(tmp);
    }
  }

  SDL_FreeSurface(surface);
}

/* EOF */
