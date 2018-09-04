//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/gl/gl_painter.hpp"

#include <iostream>
#include <algorithm>
#include <math.h>

#include "math/util.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_request.hpp"
#include "video/gl/gl_program.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/gl/gl_vertex_arrays.hpp"
#include "video/gl/gl_video_system.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

void
GLPainter::intern_draw(float left, float top, float right, float bottom,
                       float uv_left, float uv_top,
                       float uv_right, float uv_bottom,
                       float angle, float alpha,
                       const Color& color,
                       const Blend& blend,
                       const DrawingEffect& effect)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  if(effect & HORIZONTAL_FLIP)
    std::swap(uv_left, uv_right);

  if(effect & VERTICAL_FLIP)
    std::swap(uv_top, uv_bottom);

  glBlendFunc(blend.sfactor, blend.dfactor);
  //glColor4f(color.red, color.green, color.blue, color.alpha * alpha);

  vertex_arrays.set_color(Color(color.red, color.green, color.blue, color.alpha * alpha));

  // unrotated blit
  if (angle == 0.0f) {
    float vertices[] = {
      left, top,
      right, top,
      right, bottom,
      left, bottom,
    };
    //glVertexPointer(2, GL_FLOAT, 0, vertices);
    //glVertexAttribPointer(position_loc, 4, GL_FLOAT, GL_FALSE, 0, vertices);
    vertex_arrays.set_positions(vertices, sizeof(vertices));

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    //glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    vertex_arrays.set_texcoords(uvs, sizeof(uvs));

    //vertex_arrays.set_element_count();

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  } else {
    // rotated blit
    float center_x = (left + right) / 2;
    float center_y = (top + bottom) / 2;

    float sa = sinf(math::radians(angle));
    float ca = cosf(math::radians(angle));

    left  -= center_x;
    right -= center_x;

    top    -= center_y;
    bottom -= center_y;

    float vertices[] = {
      left*ca - top*sa + center_x, left*sa + top*ca + center_y,
      right*ca - top*sa + center_x, right*sa + top*ca + center_y,
      right*ca - bottom*sa + center_x, right*sa + bottom*ca + center_y,
      left*ca - bottom*sa + center_x, left*sa + bottom*ca + center_y
    };
    //glVertexPointer(2, GL_FLOAT, 0, vertices);
    vertex_arrays.set_positions(vertices, sizeof(vertices));

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    //glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    vertex_arrays.set_texcoords(uvs, sizeof(uvs));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
}

GLPainter::GLPainter(GLVideoSystem& video_system) :
  m_video_system(video_system)
{
}

void
GLPainter::draw_texture(const DrawingRequest& request)
{
  assert_gl("");
  const auto& data = static_cast<const TextureRequest&>(request);
  const auto& texture = static_cast<const GLTexture&>(*data.texture);

  glBindTexture(GL_TEXTURE_2D, texture.get_handle());

  intern_draw(data.dstrect.p1.x,
              data.dstrect.p1.y,
              data.dstrect.p2.x,
              data.dstrect.p2.y,

              data.srcrect.get_left() / static_cast<float>(texture.get_texture_width()),
              data.srcrect.get_top() / static_cast<float>(texture.get_texture_height()),
              data.srcrect.get_right() / static_cast<float>(texture.get_texture_width()),
              data.srcrect.get_bottom() / static_cast<float>(texture.get_texture_height()),

              request.angle,
              request.alpha,
              data.color,
              request.blend,
              request.drawing_effect);
  assert_gl("");
}

void
GLPainter::draw_texture_batch(const DrawingRequest& request)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  assert_gl("");
  const auto& data = static_cast<const TextureBatchRequest&>(request);
  const auto& texture = static_cast<const GLTexture&>(*data.texture);

  assert(data.srcrects.size() == data.dstrects.size());

  glBindTexture(GL_TEXTURE_2D, texture.get_handle());

  std::vector<float> vertices;
  std::vector<float> uvs;
  for(size_t i = 0; i < data.srcrects.size(); ++i)
  {
    const float left = data.dstrects[i].p1.x;
    const float top = data.dstrects[i].p1.y;
    const float right  = data.dstrects[i].p2.x;
    const float bottom = data.dstrects[i].p2.y;

    float uv_left = data.srcrects[i].get_left() / static_cast<float>(texture.get_texture_width());
    float uv_top = data.srcrects[i].get_top() / static_cast<float>(texture.get_texture_height());
    float uv_right = data.srcrects[i].get_right() / static_cast<float>(texture.get_texture_width());
    float uv_bottom = data.srcrects[i].get_bottom() / static_cast<float>(texture.get_texture_height());

    if (request.drawing_effect & HORIZONTAL_FLIP)
      std::swap(uv_left, uv_right);

    if (request.drawing_effect & VERTICAL_FLIP)
      std::swap(uv_top, uv_bottom);

    auto vertices_lst = {
      left, top,
      right, top,
      right, bottom,

      left, bottom,
      left, top,
      right, bottom,
    };

    vertices.insert(vertices.end(), std::begin(vertices_lst), std::end(vertices_lst));

    auto uvs_lst = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,

      uv_left, uv_bottom,
      uv_left, uv_top,
      uv_right, uv_bottom,
    };

    uvs.insert(uvs.end(), std::begin(uvs_lst), std::end(uvs_lst));
  }

  //glVertexPointer(2, GL_FLOAT, 0, vertices.data());
  //glTexCoordPointer(2, GL_FLOAT, 0, uvs.data());
  vertex_arrays.set_positions(vertices.data(), sizeof(float) * vertices.size());
  vertex_arrays.set_texcoords(uvs.data(), sizeof(float) * uvs.size());

  glBlendFunc(request.blend.sfactor, request.blend.dfactor);
  vertex_arrays.set_color(Color(data.color.red, data.color.green, data.color.blue, data.color.alpha * request.alpha));
  // glColor4f(data.color.red, data.color.green, data.color.blue, data.color.alpha * request.alpha);

  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(data.srcrects.size() * 2 * 3));
  assert_gl("");
}

void
GLPainter::draw_gradient(const DrawingRequest& request)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  assert_gl("");
  const auto& data = static_cast<const GradientRequest&>(request);

  const Color& top = data.top;
  const Color& bottom = data.bottom;
  const GradientDirection& direction = data.direction;
  const Rectf& region = data.region;

  //glDisable(GL_TEXTURE_2D);
  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  //glEnableClientState(GL_COLOR_ARRAY);

  float vertices[] = {
    region.p1.x, region.p1.y,
    region.p2.x, region.p1.y,
    region.p2.x, region.p2.y,
    region.p1.x, region.p2.y
  };
  // glVertexPointer(2, GL_FLOAT, 0, vertices);
  vertex_arrays.set_positions(vertices, sizeof(vertices));

  if(direction == VERTICAL || direction == VERTICAL_SECTOR)
  {
    float colors[] = {
      top.red, top.green, top.blue, top.alpha,
      top.red, top.green, top.blue, top.alpha,
      bottom.red, bottom.green, bottom.blue, bottom.alpha,
      bottom.red, bottom.green, bottom.blue, bottom.alpha,
    };
    // glColorPointer(4, GL_FLOAT, 0, colors);
    vertex_arrays.set_colors(colors, sizeof(colors));
  }
  else
  {
    float colors[] = {
      top.red, top.green, top.blue, top.alpha,
      bottom.red, bottom.green, bottom.blue, bottom.alpha,
      bottom.red, bottom.green, bottom.blue, bottom.alpha,
      top.red, top.green, top.blue, top.alpha,
    };
    // glColorPointer(4, GL_FLOAT, 0, colors);
    vertex_arrays.set_colors(colors, sizeof(colors));
  }

  // white dummy texture to make the shader happy
  glBindTexture(GL_TEXTURE_2D, m_video_system.get_white_texture().get_handle());
  vertex_arrays.set_texcoord(0.0f, 0.0f);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  assert_gl("");
}

void
GLPainter::draw_filled_rect(const DrawingRequest& request)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  assert_gl("");
  const auto& data = static_cast<const FillRectRequest&>(request);

  //glDisable(GL_TEXTURE_2D);
  //glColor4f(data.color.red, data.color.green,
  //          data.color.blue, data.color.alpha);
  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  vertex_arrays.set_color(data.color);

  // white dummy texture to make the shader happy
  glBindTexture(GL_TEXTURE_2D, m_video_system.get_white_texture().get_handle());
  vertex_arrays.set_texcoord(0.0f, 0.0f);

  if (data.radius != 0.0f)
  {
    // draw round rect
    // Keep radius in the limits, so that we get a circle instead of
    // just graphic junk
    float radius = std::min(data.radius,
                            std::min(data.size.x/2,
                                     data.size.y/2));

    // inner rectangle
    Rectf irect(data.pos.x    + radius,
                data.pos.y    + radius,
                data.pos.x + data.size.x - radius,
                data.pos.y + data.size.y - radius);

    int n = 8;
    size_t p = 0;
    std::vector<float> vertices((n+1) * 4 * 2);

    for(int i = 0; i <= n; ++i)
    {
      float x = sinf(static_cast<float>(i) * math::PI_2 / static_cast<float>(n)) * radius;
      float y = cosf(static_cast<float>(i) * math::PI_2 / static_cast<float>(n)) * radius;

      vertices[p++] = irect.get_left() - x;
      vertices[p++] = irect.get_top()  - y;

      vertices[p++] = irect.get_right() + x;
      vertices[p++] = irect.get_top()   - y;
    }

    for(int i = 0; i <= n; ++i)
    {
      float x = cosf(static_cast<float>(i) * math::PI_2 / static_cast<float>(n)) * radius;
      float y = sinf(static_cast<float>(i) * math::PI_2 / static_cast<float>(n)) * radius;

      vertices[p++] = irect.get_left()   - x;
      vertices[p++] = irect.get_bottom() + y;

      vertices[p++] = irect.get_right()  + x;
      vertices[p++] = irect.get_bottom() + y;
    }

    //glVertexPointer(2, GL_FLOAT, 0, &*vertices.begin());
    vertex_arrays.set_positions(vertices.data(), sizeof(float) * vertices.size());

    glDrawArrays(GL_TRIANGLE_STRIP, 0,  static_cast<GLsizei>(vertices.size() / 2));
  }
  else
  {
    float x = data.pos.x;
    float y = data.pos.y;
    float w = data.size.x;
    float h = data.size.y;

    float vertices[] = {
      x,   y,
      x+w, y,
      x+w, y+h,
      x,   y+h
    };

    //glVertexPointer(2, GL_FLOAT, 0, vertices);
    vertex_arrays.set_positions(vertices, sizeof(vertices));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  assert_gl("");
}

void
GLPainter::draw_inverse_ellipse(const DrawingRequest& request)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  assert_gl("");
  const auto& data = static_cast<const InverseEllipseRequest&>(request);

  //glDisable(GL_TEXTURE_2D);
  //glColor4f(data.color.red,  data.color.green,
  //          data.color.blue, data.color.alpha);
  vertex_arrays.set_color(data.color);

  // white dummy texture to make the shader happy
  glBindTexture(GL_TEXTURE_2D, m_video_system.get_white_texture().get_handle());
  vertex_arrays.set_texcoord(0.0f, 0.0f);

  float x = data.pos.x;
  float y = data.pos.y;
  float w = data.size.x/2.0f;
  float h = data.size.y/2.0f;

  static const int slices = 16;
  static const int points = (slices+1) * 12;

  float vertices[points * 2];
  int   p = 0;

  const Viewport& viewport = m_video_system.get_viewport();
  float screen_width = static_cast<float>(viewport.get_screen_width());
  float screen_height = static_cast<float>(viewport.get_screen_height());

  // Bottom
  vertices[p++] = screen_width; vertices[p++] = screen_height;
  vertices[p++] = 0;            vertices[p++] = screen_height;
  vertices[p++] = x;            vertices[p++] = y+h;

  // Top
  vertices[p++] = screen_width; vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = x;            vertices[p++] = y-h;

  // Left
  vertices[p++] = screen_width; vertices[p++] = 0;
  vertices[p++] = screen_width; vertices[p++] = screen_height;
  vertices[p++] = x+w;          vertices[p++] = y;

  // Right
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = screen_height;
  vertices[p++] = x-w;          vertices[p++] = y;

  for(int i = 0; i < slices; ++i)
  {
    float ex1 = sinf(math::PI_2 / static_cast<float>(slices) * static_cast<float>(i)) * w;
    float ey1 = cosf(math::PI_2 / static_cast<float>(slices) * static_cast<float>(i)) * h;

    float ex2 = sinf(math::PI_2 / static_cast<float>(slices) * static_cast<float>(i+1)) * w;
    float ey2 = cosf(math::PI_2 / static_cast<float>(slices) * static_cast<float>(i+1)) * h;

    // Bottom/Right
    vertices[p++] = screen_width; vertices[p++] = screen_height;
    vertices[p++] = x + ex1;      vertices[p++] = y + ey1;
    vertices[p++] = x + ex2;      vertices[p++] = y + ey2;

    // Top/Left
    vertices[p++] = 0;            vertices[p++] = 0;
    vertices[p++] = x - ex1;      vertices[p++] = y - ey1;
    vertices[p++] = x - ex2;      vertices[p++] = y - ey2;

    // Top/Right
    vertices[p++] = screen_width; vertices[p++] = 0;
    vertices[p++] = x + ex1;      vertices[p++] = y - ey1;
    vertices[p++] = x + ex2;      vertices[p++] = y - ey2;

    // Bottom/Left
    vertices[p++] = 0;            vertices[p++] = screen_height;
    vertices[p++] = x - ex1;      vertices[p++] = y + ey1;
    vertices[p++] = x - ex2;      vertices[p++] = y + ey2;
  }

  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  //glVertexPointer(2, GL_FLOAT, 0, vertices);
  vertex_arrays.set_positions(vertices, sizeof(vertices));

  glDrawArrays(GL_TRIANGLES, 0, points);

  assert_gl("");
}

void
GLPainter::draw_line(const DrawingRequest& request)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  assert_gl("");
  const auto& data = static_cast<const LineRequest&>(request);

  //glDisable(GL_TEXTURE_2D);
  //glColor4f(data.color.red, data.color.green, data.color.blue, data.color.alpha);
  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  vertex_arrays.set_color(data.color);

  // white dummy texture to make the shader happy
  glBindTexture(GL_TEXTURE_2D, m_video_system.get_white_texture().get_handle());
  vertex_arrays.set_texcoord(0.0f, 0.0f);

  float x1 = data.pos.x;
  float y1 = data.pos.y;
  float x2 = data.dest_pos.x;
  float y2 = data.dest_pos.y;

  // OpenGL3.3 doesn't have GL_LINES anymore, so instead we transform
  // the line into a quad and draw it as triangle strip.
  // triangle strip
  float x_step = (y2 - y1);
  float y_step = -(x2 - x1);

  float step_norm = sqrtf(x_step * x_step + y_step * y_step);
  x_step /= step_norm;
  y_step /= step_norm;

  x_step *= 0.5f;
  y_step *= 0.5f;

  // FIXME: this results in lines of not quite consistant width
  float vertices[] = {
    (x1 - x_step), (y1 - y_step),
    (x2 - x_step), (y2 - y_step),
    (x1 + x_step), (y1 + y_step),
    (x2 + x_step), (y2 + y_step),
  };

  //glVertexPointer(2, GL_FLOAT, 0, vertices);
  vertex_arrays.set_positions(vertices, sizeof(vertices));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  assert_gl("");
}

void
GLPainter::draw_triangle(const DrawingRequest& request)
{
  GLVertexArrays& vertex_arrays = m_video_system.get_vertex_arrays();

  assert_gl("");
  const auto& data = static_cast<const TriangleRequest&>(request);

  //glDisable(GL_TEXTURE_2D);
  //glColor4f(data.color.red, data.color.green, data.color.blue, data.color.alpha);
  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  vertex_arrays.set_color(data.color);

  // white dummy texture to make the shader happy
  glBindTexture(GL_TEXTURE_2D, m_video_system.get_white_texture().get_handle());
  vertex_arrays.set_texcoord(0.0f, 0.0f);

  float x1 = data.pos1.x;
  float y1 = data.pos1.y;
  float x2 = data.pos2.x;
  float y2 = data.pos2.y;
  float x3 = data.pos3.x;
  float y3 = data.pos3.y;

  float vertices[] = {
    x1, y1,
    x2, y2,
    x3, y3
  };
  //glVertexPointer(2, GL_FLOAT, 0, vertices);
  vertex_arrays.set_positions(vertices, sizeof(vertices));

  glDrawArrays(GL_TRIANGLES, 0, 3);

  assert_gl("");
}

/* EOF */
