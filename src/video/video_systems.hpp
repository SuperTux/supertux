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

#ifndef HEADER_SUPERTUX_VIDEO_VIDEO_SYSTEMS_HPP
#define HEADER_SUPERTUX_VIDEO_VIDEO_SYSTEMS_HPP

#include <config.h>

#include <SDL.h>
#include <string>

class Renderer;
class Lightmap;
class Texture;
class Surface;

class VideoSystem
{
public:
  enum Enum {
    AUTO_VIDEO,
    OPENGL,
    PURE_SDL,
    NUM_SYSTEMS
  };

public:
  static Renderer* new_renderer();
  static Lightmap* new_lightmap();
  static Texture*  new_texture(SDL_Surface *image);
  static void*     new_surface_data(const Surface &surface);
  static void      free_surface_data(void *surface_data);

  static Enum get_video_system(const std::string &video);
  static std::string get_video_string(Enum video);

private:
  VideoSystem();
  VideoSystem(const VideoSystem&);
  VideoSystem& operator=(const VideoSystem&);
};

#endif

/* EOF */
