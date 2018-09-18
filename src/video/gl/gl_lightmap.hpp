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

#ifndef HEADER_SUPERTUX_VIDEO_GL_LIGHTMAP_HPP
#define HEADER_SUPERTUX_VIDEO_GL_LIGHTMAP_HPP

#include <memory>

#include "video/gl.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/renderer.hpp"

class GLFramebuffer;
class GLTexture;
class GLVideoSystem;
class Rect;
class Texture;
struct DrawingRequest;

class GLLightmap final : public Renderer
{
public:
  GLLightmap(GLVideoSystem& video_system, const Size& size);
  ~GLLightmap();

  virtual void start_draw() override;
  virtual void end_draw() override;

  virtual GLPainter& get_painter() override { return m_painter; }

  virtual void clear(const Color& color) override;

  virtual void set_clip_rect(const Rect& rect) override;
  virtual void clear_clip_rect() override;

  virtual void get_light(const DrawingRequest& request) const override;
  virtual void render() override;

private:
  static const int s_LIGHTMAP_DIV = 5;

private:
  GLVideoSystem& m_video_system;
  Size m_size;
  GLPainter m_painter;

  std::shared_ptr<GLTexture> m_lightmap;
  std::unique_ptr<GLFramebuffer> m_framebuffer;
  int m_lightmap_width;
  int m_lightmap_height;

private:
  GLLightmap(const GLLightmap&);
  GLLightmap& operator=(const GLLightmap&);
};

#endif

/* EOF */
