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

#include <algorithm>

#include "supertux/resources.hpp"
#include "video/drawing_request.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"

GLuint GLPainter::s_last_texture = static_cast<GLuint>(-1);

namespace {

inline void intern_draw(float left, float top, float right, float bottom,
                        float uv_left, float uv_top,
                        float uv_right, float uv_bottom,
                        float angle, float alpha,
                        const Color& color,
                        const Blend& blend,
                        DrawingEffect effect)
{
  if(effect & HORIZONTAL_FLIP)
    std::swap(uv_left, uv_right);

  if(effect & VERTICAL_FLIP)
    std::swap(uv_top, uv_bottom);

  glBlendFunc(blend.sfactor, blend.dfactor);
  glColor4f(color.red, color.green, color.blue, color.alpha * alpha);

  // unrotated blit
  if (angle == 0.0f) {
    float vertices[] = {
      left, top,
      right, top,
      right, bottom,
      left, bottom,
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  } else {
    // rotated blit
    float center_x = (left + right) / 2;
    float center_y = (top + bottom) / 2;

    float sa = sinf(angle/180.0f*M_PI);
    float ca = cosf(angle/180.0f*M_PI);

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
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  // FIXME: find a better way to restore the blend mode
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

} // namespace

void
GLPainter::draw_surface(const DrawingRequest& request)
{
  const Surface* surface = static_cast<const SurfaceRequest*>(request.request_data)->surface;
  if(surface == NULL)
  {
    return;
  }
  GLTexture* gltexture = static_cast<GLTexture*>(surface->get_texture().get());
  if(gltexture == NULL)
  {
    return;
  }
  GLSurfaceData *surface_data = static_cast<GLSurfaceData*>(surface->get_surface_data());
  if(surface_data == NULL)
  {
    return;
  }

  GLuint th = gltexture->get_handle();
  if (th != s_last_texture) {
    s_last_texture = th;
    glBindTexture(GL_TEXTURE_2D, th);
  }
  intern_draw(request.pos.x, request.pos.y,
              request.pos.x + surface->get_width(),
              request.pos.y + surface->get_height(),
              surface_data->get_uv_left(),
              surface_data->get_uv_top(),
              surface_data->get_uv_right(),
              surface_data->get_uv_bottom(),
              request.angle,
              request.alpha,
              request.color,
              request.blend,
              request.drawing_effect);
}

void
GLPainter::draw_surface_part(const DrawingRequest& request)
{
  const SurfacePartRequest* surfacepartrequest
    = static_cast<SurfacePartRequest*>(request.request_data);
  const Surface* surface = surfacepartrequest->surface;
  std::shared_ptr<GLTexture> gltexture = std::dynamic_pointer_cast<GLTexture>(surface->get_texture());
  GLSurfaceData *surface_data = reinterpret_cast<GLSurfaceData *>(surface->get_surface_data());

  float uv_width = surface_data->get_uv_right() - surface_data->get_uv_left();
  float uv_height = surface_data->get_uv_bottom() - surface_data->get_uv_top();

  float uv_left = surface_data->get_uv_left() + (uv_width * surfacepartrequest->srcrect.p1.x) / surface->get_width();
  float uv_top = surface_data->get_uv_top() + (uv_height * surfacepartrequest->srcrect.p1.y) / surface->get_height();
  float uv_right = surface_data->get_uv_left() + (uv_width * surfacepartrequest->srcrect.p2.x) / surface->get_width();
  float uv_bottom = surface_data->get_uv_top() + (uv_height * surfacepartrequest->srcrect.p2.y) / surface->get_height();

  GLuint th = gltexture->get_handle();
  if (th != s_last_texture) {
    s_last_texture = th;
    glBindTexture(GL_TEXTURE_2D, th);
  }
  intern_draw(request.pos.x, request.pos.y,
              request.pos.x + surfacepartrequest->dstsize.width,
              request.pos.y + surfacepartrequest->dstsize.height,
              uv_left,
              uv_top,
              uv_right,
              uv_bottom,
              0.0,
              request.alpha,
              request.color,
              Blend(),
              request.drawing_effect);
}

void
GLPainter::draw_gradient(const DrawingRequest& request)
{
  const GradientRequest* gradientrequest
    = static_cast<GradientRequest*>(request.request_data);
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;
  const GradientDirection& direction = gradientrequest->direction;
  const Rectf& region = gradientrequest->region;

  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  float vertices[] = {
    region.p1.x, region.p1.y,
    region.p2.x, region.p1.y,
    region.p2.x, region.p2.y,
    region.p1.x, region.p2.y
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

if(direction == VERTICAL || direction == VERTICAL_SECTOR)
{
  float colors[] = {
    top.red, top.green, top.blue, top.alpha,
    top.red, top.green, top.blue, top.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
  };
  glColorPointer(4, GL_FLOAT, 0, colors);
}
else
{
  float colors[] = {
    top.red, top.green, top.blue, top.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
    top.red, top.green, top.blue, top.alpha,
  };
  glColorPointer(4, GL_FLOAT, 0, colors);
}

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLPainter::draw_filled_rect(const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = static_cast<FillRectRequest*>(request.request_data);

  glDisable(GL_TEXTURE_2D);
  glColor4f(fillrectrequest->color.red, fillrectrequest->color.green,
            fillrectrequest->color.blue, fillrectrequest->color.alpha);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if (fillrectrequest->radius != 0.0f)
  {
    // draw round rect
    // Keep radius in the limits, so that we get a circle instead of
    // just graphic junk
    float radius = std::min(fillrectrequest->radius,
                            std::min(fillrectrequest->size.x/2,
                                     fillrectrequest->size.y/2));

    // inner rectangle
    Rectf irect(request.pos.x    + radius,
                request.pos.y    + radius,
                request.pos.x + fillrectrequest->size.x - radius,
                request.pos.y + fillrectrequest->size.y - radius);

    int n = 8;
    int p = 0;
    std::vector<float> vertices((n+1) * 4 * 2);

    for(int i = 0; i <= n; ++i)
    {
      float x = sinf(i * (M_PI/2) / n) * radius;
      float y = cosf(i * (M_PI/2) / n) * radius;

      vertices[p++] = irect.get_left() - x;
      vertices[p++] = irect.get_top()  - y;

      vertices[p++] = irect.get_right() + x;
      vertices[p++] = irect.get_top()   - y;
    }

    for(int i = 0; i <= n; ++i)
    {
      float x = cosf(i * (M_PI/2) / n) * radius;
      float y = sinf(i * (M_PI/2) / n) * radius;

      vertices[p++] = irect.get_left()   - x;
      vertices[p++] = irect.get_bottom() + y;

      vertices[p++] = irect.get_right()  + x;
      vertices[p++] = irect.get_bottom() + y;
    }

    glVertexPointer(2, GL_FLOAT, 0, &*vertices.begin());
    glDrawArrays(GL_TRIANGLE_STRIP, 0,  vertices.size()/2);
  }
  else
  {
    float x = request.pos.x;
    float y = request.pos.y;
    float w = fillrectrequest->size.x;
    float h = fillrectrequest->size.y;

    float vertices[] = {
      x,   y,
      x+w, y,
      x+w, y+h,
      x,   y+h
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLPainter::draw_inverse_ellipse(const DrawingRequest& request)
{
  const InverseEllipseRequest* ellipse = static_cast<InverseEllipseRequest*> (request.request_data);

  glDisable(GL_TEXTURE_2D);
  glColor4f(ellipse->color.red,  ellipse->color.green,
            ellipse->color.blue, ellipse->color.alpha);

  float x = request.pos.x;
  float y = request.pos.y;
  float w = ellipse->size.x/2.0f;
  float h = ellipse->size.y/2.0f;

  static const int slices = 16;
  static const int points = (slices+1) * 12;

  float vertices[points * 2];
  int   p = 0;

  // Bottom
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x;            vertices[p++] = y+h;

  // Top
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = x;            vertices[p++] = y-h;

  // Left
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x+w;          vertices[p++] = y;

  // Right
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x-w;          vertices[p++] = y;

  for(int i = 0; i < slices; ++i)
  {
    float ex1 = sinf(M_PI/2 / slices * i) * w;
    float ey1 = cosf(M_PI/2 / slices * i) * h;

    float ex2 = sinf(M_PI/2 / slices * (i+1)) * w;
    float ey2 = cosf(M_PI/2 / slices * (i+1)) * h;

    // Bottom/Right
    vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
    vertices[p++] = x + ex1;      vertices[p++] = y + ey1;
    vertices[p++] = x + ex2;      vertices[p++] = y + ey2;

    // Top/Left
    vertices[p++] = 0;            vertices[p++] = 0;
    vertices[p++] = x - ex1;      vertices[p++] = y - ey1;
    vertices[p++] = x - ex2;      vertices[p++] = y - ey2;

    // Top/Right
    vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
    vertices[p++] = x + ex1;      vertices[p++] = y - ey1;
    vertices[p++] = x + ex2;      vertices[p++] = y - ey2;

    // Bottom/Left
    vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
    vertices[p++] = x - ex1;      vertices[p++] = y + ey1;
    vertices[p++] = x - ex2;      vertices[p++] = y + ey2;
  }

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_TRIANGLES, 0, points);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLPainter::draw_text(const DrawingRequest& request)
{
  const TextRequest* textrequest = static_cast<TextRequest*>(request.request_data);

  auto font = textrequest->font->get_ttf_font();
  int line_height = textrequest->font->get_height();
  int shadow_size = textrequest->font->get_shadow_size();

  int last_pos = 0;
  int last_y = request.pos.y;
  for(size_t i = 0; i < textrequest->text.length(); i++)
  {
    if(textrequest->text[i] != '\n' /* new line */ && i != textrequest->text.length() - 1 /* end of string */)
    {
      continue;
    }
    std::string str;
    if(textrequest->text[i] == '\n')
      str = textrequest->text.substr(last_pos, i - last_pos);
    else
      str = textrequest->text.substr(last_pos, i + 1);

    last_pos = i + 1;

    auto texture = TextureManager::current()->get(font, str, request.color);
    auto gltexture = std::dynamic_pointer_cast<GLTexture>(texture);
    auto surface = Surface::create(texture);
    auto surface_data = static_cast<GLSurfaceData*>(surface->get_surface_data());
    if(surface_data == NULL)
    {
      return;
    }

    int last_x = request.pos.x;
    if(textrequest->alignment == ALIGN_CENTER)
      last_x -= surface->get_width() / 2;
    else if(textrequest->alignment == ALIGN_RIGHT)
      last_x -= surface->get_width();

    GLuint th = gltexture->get_handle();
    if (th != s_last_texture) {
      s_last_texture = th;
      glBindTexture(GL_TEXTURE_2D, th);
    }

    intern_draw(last_x + shadow_size, last_y + shadow_size,
                last_x + shadow_size + surface->get_width(),
                last_y + shadow_size + surface->get_height(),
                surface_data->get_uv_left(),
                surface_data->get_uv_top(),
                surface_data->get_uv_right(),
                surface_data->get_uv_bottom(),
                request.angle,
                request.alpha,
                Color(0, 0, 0, 0.45),
                request.blend,
                request.drawing_effect);

    intern_draw(last_x, last_y,
                last_x + surface->get_width(),
                last_y + surface->get_height(),
                surface_data->get_uv_left(),
                surface_data->get_uv_top(),
                surface_data->get_uv_right(),
                surface_data->get_uv_bottom(),
                request.angle,
                request.alpha,
                request.color,
                request.blend,
                request.drawing_effect);
    last_y += line_height;
  }
}

void
GLPainter::draw_line(const DrawingRequest& request)
{
  const LineRequest* linerequest
    = static_cast<LineRequest*>(request.request_data);

  glDisable(GL_TEXTURE_2D);
  glColor4f(linerequest->color.red, linerequest->color.green,
            linerequest->color.blue, linerequest->color.alpha);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  float x1 = request.pos.x;
  float y1 = request.pos.y;
  float x2 = linerequest->dest_pos.x;
  float y2 = linerequest->dest_pos.y;

  float vertices[] = {
    x1, y1,
    x2, y2
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_LINES, 0, 2);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLPainter::draw_triangle(const DrawingRequest& request)
{
  const TriangleRequest* trianglerequest
    = static_cast<TriangleRequest*>(request.request_data);

  glDisable(GL_TEXTURE_2D);
  glColor4f(trianglerequest->color.red, trianglerequest->color.green,
            trianglerequest->color.blue, trianglerequest->color.alpha);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  float x1 = request.pos.x;
  float y1 = request.pos.y;
  float x2 = trianglerequest->pos2.x;
  float y2 = trianglerequest->pos2.y;
  float x3 = trianglerequest->pos3.x;
  float y3 = trianglerequest->pos3.y;

  float vertices[] = {
    x1, y1,
    x2, y2,
    x3, y3
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

/* EOF */
