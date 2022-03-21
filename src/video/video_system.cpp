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

#include "util/file_system.hpp"
#include "util/log.hpp"
#include "video/null/null_video_system.hpp"
#include "video/sdl/sdl_video_system.hpp"
#include "video/sdl_surface.hpp"
#include "video/sdl_surface_ptr.hpp"

#ifdef HAVE_OPENGL
#  include "video/gl/gl_video_system.hpp"
#endif

std::unique_ptr<VideoSystem>
VideoSystem::create(VideoSystem::Enum video_system)
{
  switch (video_system)
  {
    case VIDEO_AUTO:
#ifdef HAVE_OPENGL
  #ifdef __EMSCRIPTEN__
      try
      {
        log_warning << "WebGL detected, using GLVideoSystem-20" << std::endl;
        return std::make_unique<GLVideoSystem>(false, false);
      }
      catch(std::exception& err2)
      {
        log_warning << "Error creating GLVideoSystem-20, using SDL fallback: "  << err2.what() << std::endl;
        return std::make_unique<SDLVideoSystem>();
      }
  #else
      try
      {
        return std::make_unique<GLVideoSystem>(true, true);
      }
      catch(std::exception& err)
      {
        try
        {
          log_warning << "Error creating GLVideoSystem-330core, using GLVideoSystem-20 fallback: "  << err.what() << std::endl;
          return std::make_unique<GLVideoSystem>(false, false);
        }
        catch(std::exception& err2)
        {
          log_warning << "Error creating GLVideoSystem-20, using SDL fallback: "  << err2.what() << std::endl;
          return std::make_unique<SDLVideoSystem>();
        }
      }
  #endif
#else
      log_info << "new SDL renderer\n";
      return std::make_unique<SDLVideoSystem>();
#endif

#ifdef HAVE_OPENGL
    case VIDEO_OPENGL33CORE:
      return std::make_unique<GLVideoSystem>(true, false);

    case VIDEO_OPENGL20:
      return std::make_unique<GLVideoSystem>(false, false);
#else
    case VIDEO_OPENGL33CORE:
    case VIDEO_OPENGL20:
      log_warning << "OpenGL requested, but missing using SDL fallback" << std::endl;
      return std::make_unique<SDLVideoSystem>();
#endif

    case VIDEO_SDL:
      log_info << "new SDL renderer\n";
      return std::make_unique<SDLVideoSystem>();

    case VIDEO_NULL:
      return std::make_unique<NullVideoSystem>();

    default:
      log_fatal << "invalid video system in config" << std::endl;
      assert(false);
      return {};
  }
}

VideoSystem::Enum
VideoSystem::get_video_system(const std::string &video)
{
  if (video == "auto")
  {
    return VIDEO_AUTO;
  }
#ifdef HAVE_OPENGL
  else if (video == "opengl" || video == "opengl33" || video == "opengl33core")
  {
    return VIDEO_OPENGL33CORE;
  }
  else if (video == "opengl20")
  {
    return VIDEO_OPENGL20;
  }
#endif
  else if (video == "sdl")
  {
    return VIDEO_SDL;
  }
  else if (video == "null")
  {
    return VIDEO_NULL;
  }
  else
  {
#ifdef HAVE_OPENGL
    throw std::runtime_error("invalid VideoSystem::Enum, valid values are 'auto', 'sdl', 'opengl', 'opengl20' and 'null'");
#else
    throw std::runtime_error("invalid VideoSystem::Enum, valid values are 'auto' and 'sdl'");
#endif
  }
}

std::string
VideoSystem::get_video_string(VideoSystem::Enum video)
{
  switch (video)
  {
    case VIDEO_AUTO:
      return "auto";
    case VIDEO_OPENGL33CORE:
      return "opengl";
    case VIDEO_OPENGL20:
      return "opengl20";
    case VIDEO_SDL:
      return "sdl";
    case VIDEO_NULL:
      return "null";
    default:
      log_fatal << "invalid video system in config" << std::endl;
      assert(false);
      return "auto";
  }
}

void
VideoSystem::do_take_screenshot()
{
  SDLSurfacePtr surface = make_screenshot();
  if (!surface) {
    log_warning << "Creating the screenshot has failed" << std::endl;
    return;
  }

  const std::string screenshots_dir = "/screenshots";
  if (!PHYSFS_exists(screenshots_dir.c_str())) {
    if (!PHYSFS_mkdir(screenshots_dir.c_str())) {
      log_warning << "Creating '" << screenshots_dir << "' failed" << std::endl;
      return;
    }
  }

  auto find_filename = [&]() -> boost::optional<std::string>
    {
      for (int num = 0; num < 1000000; ++num)
      {
        std::ostringstream oss;
        oss << "screenshot" << std::setw(6) << std::setfill('0') << num << ".png";
        const std::string screenshot_filename = FileSystem::join(screenshots_dir, oss.str());
        if (!PHYSFS_exists(screenshot_filename.c_str())) {
          return screenshot_filename;
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
    if (SDLSurface::save_png(*surface, *filename)) {
      log_info << "Wrote screenshot to \"" << *filename << "\"" << std::endl;
    }
  }
}

/* EOF */
