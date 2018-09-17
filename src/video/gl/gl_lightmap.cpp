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

#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/drawing_request.hpp"
#include "video/gl.hpp"
#include "video/gl/gl_context.hpp"
#include "video/gl/gl_framebuffer.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/gl/gl_program.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/gl/gl_vertex_arrays.hpp"
#include "video/gl/gl_video_system.hpp"
#include "video/glutil.hpp"

GLLightmap::GLLightmap(GLVideoSystem& video_system, const Size& size) :
  m_video_system(video_system),
  m_size(size),
  m_painter(m_video_system),
  m_lightmap(),
  m_framebuffer(),
  m_lightmap_width(),
  m_lightmap_height()
{
}

GLLightmap::~GLLightmap()
{
}

void
GLLightmap::start_draw()
{
  assert_gl();
  GLContext& context = m_video_system.get_context();
  context.bind();

  if (!m_lightmap)
  {
    m_lightmap_width = m_size.width / s_LIGHTMAP_DIV;
    m_lightmap_height = m_size.height / s_LIGHTMAP_DIV;

    m_lightmap.reset(new GLTexture(next_power_of_two(m_lightmap_width),
                                   next_power_of_two(m_lightmap_height)));

    if (m_video_system.get_context().supports_framebuffer())
    {
      m_framebuffer = std::make_unique<GLFramebuffer>(*m_lightmap);
    }
  }

  if (m_framebuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer->get_handle());
  }

  glViewport(0, 0, m_lightmap_width, m_lightmap_height);

  context.ortho(static_cast<float>(m_size.width), static_cast<float>(m_size.height));

  assert_gl();
}

void
GLLightmap::end_draw()
{
  assert_gl();

  if (m_framebuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  else
  {
    assert_gl();
    glBindTexture(GL_TEXTURE_2D, m_lightmap->get_handle());
    glCopyTexSubImage2D(GL_TEXTURE_2D,
                        0, // level
                        0, 0, // offset
                        0, 0, // x, y
                        m_lightmap_width,
                        m_lightmap_height);
  }

  assert_gl();
}

void
GLLightmap::render()
{
  GLContext& context = m_video_system.get_context();

  assert_gl();
  // multiple the lightmap with the framebuffer
  context.blend_func(GL_DST_COLOR, GL_ZERO);

  context.bind_texture(*m_lightmap);
  context.set_color(Color::WHITE);

  float vertices[] = {
    0, 0,
    static_cast<float>(m_size.width), 0,
    static_cast<float>(m_size.width), static_cast<float>(m_size.height),
    0, static_cast<float>(m_size.height)
  };
  context.set_positions(vertices, sizeof(vertices));

  float uv_right = static_cast<float>(m_lightmap_width) / static_cast<float>(m_lightmap->get_texture_width());
  float uv_bottom = static_cast<float>(m_lightmap_height) / static_cast<float>(m_lightmap->get_texture_height());
  float uvs[] = {
    0, uv_bottom,
    uv_right, uv_bottom,
    uv_right, 0,
    0, 0
  };
  context.set_texcoords(uvs, sizeof(uvs));

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  context.blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  assert_gl();
}

void
GLLightmap::clear(const Color& color)
{
  assert_gl();
  glClearColor(color.red, color.green, color.blue, color.alpha);
  glClear(GL_COLOR_BUFFER_BIT);
  assert_gl();
}

void
GLLightmap::set_clip_rect(const Rect& clip_rect)
{
  assert_gl();
  glScissor(m_lightmap_width * clip_rect.left / m_size.width,
            m_lightmap_height * clip_rect.top / m_size.height,
            m_lightmap_width * clip_rect.get_width() / m_size.width,
            m_lightmap_height * clip_rect.get_height() / m_size.height);
  glEnable(GL_SCISSOR_TEST);
  assert_gl();
}

void
GLLightmap::clear_clip_rect()
{
  assert_gl();
  glDisable(GL_SCISSOR_TEST);
  assert_gl();
}

void
GLLightmap::get_light(const DrawingRequest& request) const
{
  assert_gl();
  const auto& data = static_cast<const GetLightRequest&>(request);

  float pixels[3] = { 0.0f, 0.0f, 0.0f };

  float x = data.pos.x * static_cast<float>(m_lightmap_width) / static_cast<float>(m_size.width);
  float y = data.pos.y * static_cast<float>(m_lightmap_height) / static_cast<float>(m_size.height);

  glReadPixels(static_cast<GLint>(x),
               m_lightmap_height - static_cast<GLint>(y),
               1, 1, GL_RGB, GL_FLOAT, pixels);

  *(data.color_ptr) = Color(pixels[0], pixels[1], pixels[2]);
  assert_gl();
}

/* EOF */
