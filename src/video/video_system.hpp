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

#ifndef HEADER_SUPERTUX_VIDEO_VIDEO_SYSTEM_HPP
#define HEADER_SUPERTUX_VIDEO_VIDEO_SYSTEM_HPP

#include <SDL.h>
#include <string>

#include "util/currenton.hpp"
#include "video/texture_ptr.hpp"

class Renderer;
class Lightmap;
class Surface;
class SurfaceData;

class VideoSystem : public Currenton<VideoSystem>
{
public:
  enum Enum {
    AUTO_VIDEO,
    OPENGL,
    PURE_SDL,
    NUM_SYSTEMS
  };

  static std::unique_ptr<VideoSystem> create(VideoSystem::Enum video_system);

  static Enum get_video_system(const std::string &video);
  static std::string get_video_string(Enum video);

public:
  VideoSystem() {}
  virtual ~VideoSystem() {}

  virtual Renderer& get_renderer() = 0;
  virtual Lightmap& get_lightmap() = 0;
  virtual TexturePtr new_texture(SDL_Surface *image) = 0;
  virtual SurfaceData* new_surface_data(const Surface &surface) = 0;
  virtual void free_surface_data(SurfaceData* surface_data) = 0;

  virtual void apply_config() = 0;
  virtual void resize(int w, int h) = 0;

private:
  VideoSystem(const VideoSystem&) = delete;
  VideoSystem& operator=(const VideoSystem&) = delete;
};

#endif

/* EOF */
