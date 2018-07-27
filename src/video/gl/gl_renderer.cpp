//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//	Updated by GiBy 2013 for SDL2 <giby_the_kid@yahoo.fr>
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

#include "video/gl/gl_renderer.hpp"

#include "math/rect.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/gl/gl_painter.hpp"
#include "video/gl/gl_video_system.hpp"
#include "video/glutil.hpp"
#include "video/util.hpp"

GLRenderer::GLRenderer(GLVideoSystem& video_system) :
  m_video_system(video_system),
  m_viewport(),
  m_scale(1.0f, 1.0f)
{
}

GLRenderer::~GLRenderer()
{
}

void
GLRenderer::start_draw()
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
GLRenderer::end_draw()
{
}

void
GLRenderer::draw_surface(const DrawingRequest& request)
{
  GLPainter::draw_surface(request);
}

void
GLRenderer::draw_surface_part(const DrawingRequest& request)
{
  GLPainter::draw_surface_part(request);
}

void
GLRenderer::draw_gradient(const DrawingRequest& request)
{
  GLPainter::draw_gradient(request);
}

void
GLRenderer::draw_filled_rect(const DrawingRequest& request)
{
  GLPainter::draw_filled_rect(request);
}

void
GLRenderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  GLPainter::draw_inverse_ellipse(request);
}

void
GLRenderer::draw_line(const DrawingRequest& request)
{
  GLPainter::draw_line(request);
}

void
GLRenderer::draw_triangle(const DrawingRequest& request)
{
  GLPainter::draw_triangle(request);
}

void
GLRenderer::clear(const Color& color)
{
  glClearColor(color.red, color.green, color.blue, color.alpha);
  glClear(GL_COLOR_BUFFER_BIT);
}

Vector
GLRenderer::to_logical(int physical_x, int physical_y) const
{
  return Vector(static_cast<float>(physical_x - m_viewport.x) / m_scale.x,
                static_cast<float>(physical_y - m_viewport.y) / m_scale.y);
}

void
GLRenderer::set_clip_rect(const Rect& rect)
{
  auto window_size = m_video_system.get_window_size();

  glScissor(window_size.width * rect.left / SCREEN_WIDTH,
            window_size.height - (window_size.height * rect.bottom / SCREEN_HEIGHT),
            window_size.width * rect.get_width() / SCREEN_WIDTH,
            window_size.height * rect.get_height() / SCREEN_HEIGHT);
  glEnable(GL_SCISSOR_TEST);
}

void
GLRenderer::clear_clip_rect()
{
  glDisable(GL_SCISSOR_TEST);
}

void
GLRenderer::set_viewport(const SDL_Rect& viewport, const Vector& scale)
{
  m_viewport = viewport;
  m_scale = scale;

  glViewport(m_viewport.x, m_viewport.y, m_viewport.w, m_viewport.h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0,
          m_viewport.w / m_scale.x,
          m_viewport.h / m_scale.y,
          0,
          -1,
          1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);
  check_gl_error("Setting up view matrices");
}

/* EOF */
