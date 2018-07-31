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

#include <string>

#include "math/size.hpp"
#include "util/currenton.hpp"
#include "video/texture_ptr.hpp"

class Lightmap;
class Rect;
class Renderer;
class Surface;
class SurfaceData;
class Viewport;
class Viewport;
struct SDL_Surface;

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

  virtual Renderer& get_renderer() const = 0;
  virtual Lightmap& get_lightmap() const = 0;

  virtual TexturePtr new_texture(SDL_Surface *image) = 0;

  virtual const Viewport& get_viewport() const = 0;
  virtual void apply_config() = 0;
  virtual void flip() = 0;
  virtual void on_resize(int w, int h) = 0;

  virtual void set_gamma(float gamma) = 0;
  virtual void set_title(const std::string& title) = 0;
  virtual void set_icon(SDL_Surface* icon) = 0;
  virtual void do_take_screenshot() = 0;

private:
  VideoSystem(const VideoSystem&) = delete;
  VideoSystem& operator=(const VideoSystem&) = delete;
};

#endif

/* EOF */
