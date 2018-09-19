//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/gl/gl_texture_renderer.hpp"

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

GLTextureRenderer::GLTextureRenderer(GLVideoSystem& video_system, const Size& size, int downscale) :
  m_video_system(video_system),
  m_painter(m_video_system, *this),
  m_size(size),
  m_downscale(downscale),
  m_texture(),
  m_framebuffer()
{
}

GLTextureRenderer::~GLTextureRenderer()
{
}

void
GLTextureRenderer::prepare()
{
  if (!m_texture)
  {
    m_texture.reset(new GLTexture(m_size.width / m_downscale,
                                  m_size.height / m_downscale));

    if (m_video_system.get_context().supports_framebuffer())
    {
      m_framebuffer = std::make_unique<GLFramebuffer>(*m_texture);
    }
  }
}

void
GLTextureRenderer::start_draw()
{
  assert_gl();
  prepare();

  GLContext& context = m_video_system.get_context();
  context.bind();

  if (m_framebuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer->get_handle());
  }

  glViewport(0, 0, m_texture->get_image_width(), m_texture->get_image_height());

  context.ortho(static_cast<float>(m_size.width), static_cast<float>(m_size.height));

  // clear the screen to get rid of lightmap remains
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  assert_gl();
}

void
GLTextureRenderer::end_draw()
{
  assert_gl();

  if (m_framebuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  else
  {
    assert_gl();
    glBindTexture(GL_TEXTURE_2D, m_texture->get_handle());
    glCopyTexSubImage2D(GL_TEXTURE_2D,
                        0, // level
                        0, 0, // offset
                        0, 0, // x, y
                        m_texture->get_image_width(),
                        m_texture->get_image_height());
  }

  assert_gl();
}

Size
GLTextureRenderer::get_logical_size() const
{
  return m_size;
}

Rect
GLTextureRenderer::get_rect() const
{
  return Rect(0, 0,
              Size(m_texture->get_image_width(),
                   m_texture->get_image_height()));
}

void
GLTextureRenderer::render()
{
  GLContext& context = m_video_system.get_context();

  assert_gl();
  // multiple the lightmap with the framebuffer
  context.blend_func(GL_DST_COLOR, GL_ZERO);

  context.bind_texture(*m_texture);
  context.set_color(Color::WHITE);

  float vertices[] = {
    0, 0,
    static_cast<float>(m_size.width), 0,
    static_cast<float>(m_size.width), static_cast<float>(m_size.height),
    0, static_cast<float>(m_size.height)
  };
  context.set_positions(vertices, sizeof(vertices));

  float uv_right = static_cast<float>(m_texture->get_image_width()) / static_cast<float>(m_texture->get_texture_width());
  float uv_bottom = static_cast<float>(m_texture->get_image_height()) / static_cast<float>(m_texture->get_texture_height());
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

/* EOF */
