//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/gl/gl33core_context.hpp"

#include "video/color.hpp"
#include "video/gl/gl_program.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/gl/gl_vertex_arrays.hpp"

GL33CoreContext::GL33CoreContext() :
  m_program(),
  m_vertex_arrays(),
  m_white_texture()
{
  m_program.reset(new GLProgram);
  m_vertex_arrays.reset(new GLVertexArrays(*this));
  m_white_texture.reset(new GLTexture(1, 1, Color::WHITE));
}

GL33CoreContext::~GL33CoreContext()
{
}

void
GL33CoreContext::bind()
{
  m_program->bind();
  m_vertex_arrays->bind();
}

void
GL33CoreContext::ortho(float width, float height)
{
  const float sx = 2.0f / static_cast<float>(width);
  const float sy = -2.0f / static_cast<float>(height);

  const float tx = -1.0f;
  const float ty = 1.0f;

  const float mvp_matrix[] = {
    sx, 0, tx,
    0, sy, ty,
    0, 0, 1
  };

  const GLint mvp_loc = m_program->get_uniform_location("modelviewprojection");
  glUniformMatrix3fv(mvp_loc, 1, false, mvp_matrix);
}

void
GL33CoreContext::blend_func(GLenum src, GLenum dst)
{
  glBlendFunc(src, dst);
}

void
GL33CoreContext::set_positions(const float* data, size_t size)
{
  m_vertex_arrays->set_positions(data, size);
}

void
GL33CoreContext::set_texcoords(const float* data, size_t size)
{
  m_vertex_arrays->set_texcoords(data, size);
}

void
GL33CoreContext::set_texcoord(float u, float v)
{
  m_vertex_arrays->set_texcoord(u, v);
}

void
GL33CoreContext::set_colors(const float* data, size_t size)
{
  m_vertex_arrays->set_colors(data, size);
}

void
GL33CoreContext::set_color(const Color& color)
{
  m_vertex_arrays->set_color(color);
}

void
GL33CoreContext::bind_texture(const GLTexture& texture)
{
  glBindTexture(GL_TEXTURE_2D, texture.get_handle());
}

void
GL33CoreContext::bind_no_texture()
{
  glBindTexture(GL_TEXTURE_2D, m_white_texture->get_handle());
}

void
GL33CoreContext::draw_arrays(GLenum type, GLint first, GLsizei count)
{
  glDrawArrays(type, first, count);
}

/* EOF */
