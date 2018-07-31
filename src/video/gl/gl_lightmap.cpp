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

#include "video/gl/gl_lightmap.hpp"

#include <iostream>

#include "supertux/globals.hpp"
#include "video/drawing_request.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/gl/gl_video_system.hpp"
#include "video/glutil.hpp"

inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}

GLLightmap::GLLightmap(GLVideoSystem& video_system) :
  m_video_system(video_system),
  m_painter(m_video_system),
  m_lightmap(),
  m_lightmap_width(),
  m_lightmap_height(),
  m_lightmap_uv_right(),
  m_lightmap_uv_bottom()
{
}

GLLightmap::~GLLightmap()
{
}

void
GLLightmap::start_draw()
{
  if (!m_lightmap)
  {
    auto window_size = m_video_system.get_window_size();

    m_lightmap_width = window_size.width / s_LIGHTMAP_DIV;
    m_lightmap_height = window_size.height / s_LIGHTMAP_DIV;

    unsigned int width = next_po2(m_lightmap_width);
    unsigned int height = next_po2(m_lightmap_height);

    m_lightmap.reset(new GLTexture(width, height));

    m_lightmap_uv_right = static_cast<float>(m_lightmap_width) / static_cast<float>(width);
    m_lightmap_uv_bottom = static_cast<float>(m_lightmap_height) / static_cast<float>(height);
    TextureManager::current()->register_texture(m_lightmap.get());
  }

  const Viewport& viewport = m_video_system.get_viewport();
  const Rect& rect = viewport.get_rect();

  glViewport(rect.left,
             rect.bottom - m_lightmap_height,
             m_lightmap_width,
             m_lightmap_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0,
          viewport.get_screen_width(),
          viewport.get_screen_height(),
          0,
          -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void
GLLightmap::end_draw()
{
  const Viewport& viewport = m_video_system.get_viewport();
  const Rect& rect = viewport.get_rect();

  glBindTexture(GL_TEXTURE_2D, m_lightmap->get_handle());
  glCopyTexSubImage2D(GL_TEXTURE_2D,
                      0, 0,
                      0, rect.left,
                      rect.bottom - m_lightmap_height,
                      m_lightmap_width, m_lightmap_height);
}

void
GLLightmap::render()
{
  // multiple the lightmap with the framebuffer
  glBlendFunc(GL_DST_COLOR, GL_ZERO);

  glBindTexture(GL_TEXTURE_2D, m_lightmap->get_handle());

  const Viewport& viewport = m_video_system.get_viewport();

  float vertices[] = {
    0, 0,
    static_cast<float>(viewport.get_screen_width()), 0,
    static_cast<float>(viewport.get_screen_width()), static_cast<float>(viewport.get_screen_height()),
    0, static_cast<float>(viewport.get_screen_height())
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  float uvs[] = {
    0,                   m_lightmap_uv_bottom,
    m_lightmap_uv_right, m_lightmap_uv_bottom,
    m_lightmap_uv_right, 0,
    0, 0
  };
  glTexCoordPointer(2, GL_FLOAT, 0, uvs);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
GLLightmap::clear(const Color& color)
{
  glClearColor(color.red, color.green, color.blue, color.alpha);
  glClear(GL_COLOR_BUFFER_BIT);
}

void
GLLightmap::set_clip_rect(const Rect& clip_rect)
{
  auto window_size = m_video_system.get_window_size();

  const Viewport& viewport = m_video_system.get_viewport();
  glScissor(m_lightmap_width * clip_rect.left / viewport.get_screen_width(),
            window_size.height - (m_lightmap_height * clip_rect.bottom / viewport.get_screen_height()),
            m_lightmap_width * clip_rect.get_width() / viewport.get_screen_width(),
            m_lightmap_height * clip_rect.get_height() / viewport.get_screen_height());

  glEnable(GL_SCISSOR_TEST);
}

void
GLLightmap::clear_clip_rect()
{
  glDisable(GL_SCISSOR_TEST);
}

void
GLLightmap::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest
    = static_cast<GetLightRequest*>(request.request_data);

  float pixels[3] = { 0.0f, 0.0f, 0.0f };

  const Viewport& viewport = m_video_system.get_viewport();
  const Rect& rect = viewport.get_rect();

  float posX = request.pos.x * static_cast<float>(m_lightmap_width) / static_cast<float>(viewport.get_screen_width()) + static_cast<float>(rect.left);
  float posY = static_cast<float>((rect.get_height() * 1.0) + (rect.top * 1.0) - request.pos.y * static_cast<float>(m_lightmap_height) / static_cast<float>(viewport.get_screen_height()));

  glReadPixels(static_cast<GLint>(posX), static_cast<GLint>(posY), 1, 1, GL_RGB, GL_FLOAT, pixels);
  *(getlightrequest->color_ptr) = Color(pixels[0], pixels[1], pixels[2]);
}

/* EOF */
