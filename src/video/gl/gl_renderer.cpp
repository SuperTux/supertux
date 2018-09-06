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
#include "video/gl/gl_context.hpp"
#include "video/gl/gl_program.hpp"
#include "video/gl/gl_vertex_arrays.hpp"
#include "video/gl/gl_video_system.hpp"
#include "video/glutil.hpp"

GLRenderer::GLRenderer(GLVideoSystem& video_system) :
  m_video_system(video_system),
  m_painter(m_video_system)
{
}

GLRenderer::~GLRenderer()
{
}

void
GLRenderer::start_draw()
{
  GLContext& context = m_video_system.get_context();
  context.bind();

  assert_gl("");

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  //glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  context.blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  const Viewport& viewport = m_video_system.get_viewport();
  const Rect& rect = viewport.get_rect();

  glViewport(rect.left, rect.top, rect.get_width(), rect.get_height());

  m_video_system.get_context().ortho(static_cast<float>(viewport.get_screen_width()),
                                     static_cast<float>(viewport.get_screen_height()));

  // clear the screen to get rid of lightmap remains
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  assert_gl("");
}

void
GLRenderer::end_draw()
{
}

void
GLRenderer::clear(const Color& color)
{
  glClearColor(color.red, color.green, color.blue, color.alpha);
  glClear(GL_COLOR_BUFFER_BIT);
}

void
GLRenderer::set_clip_rect(const Rect& clip_rect)
{
  auto window_size = m_video_system.get_window_size();

  const Viewport& viewport = m_video_system.get_viewport();

  glScissor(window_size.width * clip_rect.left / viewport.get_screen_width(),
            window_size.height - (window_size.height * clip_rect.bottom / viewport.get_screen_height()),
            window_size.width * clip_rect.get_width() / viewport.get_screen_width(),
            window_size.height * clip_rect.get_height() / viewport.get_screen_height());
  glEnable(GL_SCISSOR_TEST);
}

void
GLRenderer::clear_clip_rect()
{
  glDisable(GL_SCISSOR_TEST);
}

/* EOF */
