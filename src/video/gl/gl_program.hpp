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

#ifndef HEADER_SUPERTUX_VIDEO_GL_GL_PROGRAM_HPP
#define HEADER_SUPERTUX_VIDEO_GL_GL_PROGRAM_HPP

#include <assert.h>
#include <memory>

#include "video/gl.hpp"
#include "video/gl/gl_shader.hpp"

class GLProgram final
{
public:
  GLProgram();
  ~GLProgram();

  void bind();
  void validate();

  GLuint get_handle() const { return m_program; }

  enum GLAttribEnum
  {
    attrib_position,
    attrib_texcoord,
    attrib_texcoord_repeat,
    attrib_diffuse,
    attrib_max
  };

  enum GLUniformEnum
  {
    uniform_backbuffer,
    uniform_fragcoord2uv,
    uniform_diffuse_texture,
    uniform_displacement_texture,
    uniform_framebuffer_texture,
    uniform_game_time,
    uniform_modelviewprojection,
    uniform_animate,
    uniform_displacement_animate,
    uniform_max
  };

  GLint get_attrib_location(GLAttribEnum name) const
  {
    return m_attribs[name];
  }

  GLint get_uniform_location(GLUniformEnum name) const
  {
    return m_uniforms[name];
  }

private:
  GLint get_attrib_location(const char* name) const;
  GLint get_uniform_location(const char* name) const;

  bool get_link_status() const;
  bool get_validate_status() const;
  std::string get_info_log() const;

private:
  GLuint m_program;

  std::unique_ptr<GLShader> m_frag_shader;
  std::unique_ptr<GLShader> m_vert_shader;
  // std::map is too good for this kind of lookup
  GLint m_attribs[attrib_max];
  GLint m_uniforms[uniform_max];

private:
  GLProgram(const GLProgram&) = delete;
  GLProgram& operator=(const GLProgram&) = delete;
};

#endif

/* EOF */
