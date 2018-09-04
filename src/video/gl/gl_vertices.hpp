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

#ifndef HEADER_SUPERTUX_VIDEO_GL_GL_VERTICES_HPP
#define HEADER_SUPERTUX_VIDEO_GL_GL_VERTICES_HPP

#include "video/glutil.hpp"

class GLVideoSystem;

class GLVertices
{
public:
  GLVertices(GLVideoSystem& video_system);
  ~GLVertices();

  void bind();

  void set_element_count(int n);

  void set_positions(const float* data, size_t size);

  /** size is in bytes */
  void set_texcoords(const float* data, size_t size);

private:
  GLVideoSystem& m_video_system;
  GLuint m_vao;
  GLsizei m_element_count;
  GLuint m_positions_buffer;
  GLuint m_texcoords_buffer;

private:
  GLVertices(const GLVertices&) = delete;
  GLVertices& operator=(const GLVertices&) = delete;
};

#endif

/* EOF */
