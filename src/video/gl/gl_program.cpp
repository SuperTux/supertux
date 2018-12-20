//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/gl/gl_program.hpp"

#include <sstream>

#include "util/log.hpp"
#include "video/glutil.hpp"

GLProgram::GLProgram() :
  m_program(glCreateProgram()),
  m_frag_shader(),
  m_vert_shader()
{
  assert_gl();

#if defined(USE_OPENGLES2)
  m_frag_shader = GLShader::from_file(GL_FRAGMENT_SHADER, "shader/shader100.frag");
  m_vert_shader = GLShader::from_file(GL_VERTEX_SHADER, "shader/shader100.vert");
#else
  m_frag_shader = GLShader::from_file(GL_FRAGMENT_SHADER, "shader/shader330.frag");
  m_vert_shader = GLShader::from_file(GL_VERTEX_SHADER, "shader/shader330.vert");
#endif
  assert_gl();

  glAttachShader(m_program, m_frag_shader->get_handle());
  glAttachShader(m_program, m_vert_shader->get_handle());

  glLinkProgram(m_program);

  if (!get_link_status())
  {
    std::ostringstream out;
    out << "link failure:\n" << get_info_log() << std::endl;
    throw std::runtime_error(out.str());
  }

  assert_gl();
}

GLProgram::~GLProgram()
{
  m_vert_shader.reset();
  m_frag_shader.reset();
  glDeleteProgram(m_program);
}

void
GLProgram::bind()
{
  glUseProgram(m_program);
}

void
GLProgram::validate()
{
  glValidateProgram(m_program);
  if (!get_validate_status())
  {
    std::ostringstream out;
    out << "validate failure:\n" << get_info_log() << std::endl;
    throw std::runtime_error(out.str());
  }
}

GLint
GLProgram::get_attrib_location(const char* name) const
{
  GLint loc = glGetAttribLocation(m_program, name);
  if (loc == -1)
  {
    log_debug << "GLProgram::get_attrib_location(\"" << name << "\") failed" << std::endl;
  }
  return loc;
}

GLint
GLProgram::get_uniform_location(const char* name) const
{
  GLint loc = glGetUniformLocation(m_program, name);
  if (loc == -1)
  {
    log_debug << "GLProgram::get_uniform_location(\"" << name << "\") failed" << std::endl;
  }
  return loc;
}

bool
GLProgram::get_link_status() const
{
  GLint link_status;
  glGetProgramiv(m_program, GL_LINK_STATUS, &link_status);
  return link_status != 0;
}

bool
GLProgram::get_validate_status() const
{
  GLint validate_status;
  glGetProgramiv(m_program, GL_VALIDATE_STATUS, &validate_status);
  return validate_status != 0;
}

std::string
GLProgram::get_info_log() const
{
  GLint length;
  glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);

  if (length == 0)
  {
    return {};
  }
  else
  {
    GLsizei out_length;
    std::vector<char> str(length);
    glGetProgramInfoLog(m_program, static_cast<GLsizei>(str.size()), &out_length, str.data());
    return std::string(str.begin(), str.end());
  }
}

/* EOF */
