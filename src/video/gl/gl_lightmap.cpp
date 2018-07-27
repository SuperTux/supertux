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
#include "video/drawing_request.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/gl/gl_video_system.hpp"

inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}

GLLightmap::GLLightmap(GLVideoSystem& video_system) :
  m_video_system(video_system),
  m_lightmap(),
  m_lightmap_width(),
  m_lightmap_height(),
  m_lightmap_uv_right(),
  m_lightmap_uv_bottom()
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


GLLightmap::~GLLightmap()
{
}

void
GLLightmap::start_draw()
{
  glGetIntegerv(GL_VIEWPORT, m_old_viewport); //save viewport
  glViewport(m_old_viewport[0], m_old_viewport[3] - m_lightmap_height + m_old_viewport[1],
             m_lightmap_width, m_lightmap_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

#ifdef GL_VERSION_ES_CM_1_0
  glOrthof(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#else
  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#endif

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void
GLLightmap::end_draw()
{
  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, m_lightmap->get_handle());
  glCopyTexSubImage2D(GL_TEXTURE_2D,
                      0, 0,
                      0, m_old_viewport[0],
                      m_old_viewport[3] - m_lightmap_height + m_old_viewport[1],
                      m_lightmap_width, m_lightmap_height);

  glViewport(m_old_viewport[0], m_old_viewport[1], m_old_viewport[2], m_old_viewport[3]);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#ifdef GL_VERSION_ES_CM_1_0
  glOrthof(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#else
  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
#endif
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_BLEND);
}

void
GLLightmap::render()
{
  // multiple the lightmap with the framebuffer
  glBlendFunc(GL_DST_COLOR, GL_ZERO);

  glBindTexture(GL_TEXTURE_2D, m_lightmap->get_handle());

  float vertices[] = {
    0, 0,
    float(SCREEN_WIDTH), 0,
    float(SCREEN_WIDTH), float(SCREEN_HEIGHT),
    0, float(SCREEN_HEIGHT)
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  float uvs[] = {
    0,                 m_lightmap_uv_bottom,
    m_lightmap_uv_right, m_lightmap_uv_bottom,
    m_lightmap_uv_right, 0,
    0, 0
  };
  glTexCoordPointer(2, GL_FLOAT, 0, uvs);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
GLLightmap::draw_surface(const DrawingRequest& request)
{
  GLPainter::draw_surface(request);
}

void
GLLightmap::draw_surface_part(const DrawingRequest& request)
{
  GLPainter::draw_surface_part(request);
}

void
GLLightmap::draw_gradient(const DrawingRequest& request)
{
  GLPainter::draw_gradient(request);
}

void
GLLightmap::draw_filled_rect(const DrawingRequest& request)
{
  GLPainter::draw_filled_rect(request);
}

void
GLLightmap::draw_inverse_ellipse(const DrawingRequest& request)
{
  GLPainter::draw_inverse_ellipse(request);
}

void
GLLightmap::draw_line(const DrawingRequest& request)
{
  GLPainter::draw_line(request);
}

void
GLLightmap::draw_triangle(const DrawingRequest& request)
{
  GLPainter::draw_triangle(request);
}

void
GLLightmap::clear(const Color& color)
{
  glClearColor(color.red, color.green, color.blue, color.alpha);
  glClear(GL_COLOR_BUFFER_BIT);
}

void
GLLightmap::set_clip_rect(const Rect& rect)
{
  auto window_size = m_video_system.get_window_size();

  glScissor(m_lightmap_width * rect.left / SCREEN_WIDTH,
            window_size.height - (m_lightmap_height * rect.bottom / SCREEN_HEIGHT),
            m_lightmap_width * rect.get_width() / SCREEN_WIDTH,
            m_lightmap_height * rect.get_height() / SCREEN_HEIGHT);

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

  float pixels[3];
  for( int i = 0; i<3; i++)
    pixels[i] = 0.0f; //set to black

  float posX = request.pos.x * m_lightmap_width / SCREEN_WIDTH + m_old_viewport[0];
  float posY = (m_old_viewport[3] * 1.0) + (m_old_viewport[1] * 1.0) - request.pos.y * m_lightmap_height / SCREEN_HEIGHT;
  glReadPixels((GLint) posX, (GLint) posY , 1, 1, GL_RGB, GL_FLOAT, pixels);
  *(getlightrequest->color_ptr) = Color( pixels[0], pixels[1], pixels[2]);
}

/* EOF */
