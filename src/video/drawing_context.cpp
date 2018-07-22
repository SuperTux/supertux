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

#include "video/drawing_context.hpp"

#include <algorithm>

#include "supertux/globals.hpp"
#include "util/obstackpp.hpp"
#include "video/drawing_request.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

bool DrawingContext::render_lighting = true;

DrawingContext::DrawingContext(VideoSystem& video_system_) :
  m_video_system(video_system_),
  m_obst(),
  m_drawing_requests(NORMAL, *this, m_obst),
  m_lightmap_requests(LIGHTMAP, *this, m_obst),
  m_ambient_color(1.0f, 1.0f, 1.0f, 1.0f),
  m_transformstack(),
  m_transform()
{
  obstack_init(&m_obst);
}

DrawingContext::~DrawingContext()
{
  m_lightmap_requests.clear();
  m_drawing_requests.clear();
  obstack_free(&m_obst, NULL);
}

void
DrawingContext::get_light(const Vector& position, Color* color_out)
{
  if (m_ambient_color.red == 1.0f &&
      m_ambient_color.green == 1.0f &&
      m_ambient_color.blue == 1.0f)
  {
    *color_out = Color( 1.0f, 1.0f, 1.0f);
    return;
  }

  auto request = new(m_obst) DrawingRequest();
  request->target = LIGHTMAP; // FIXME: request->target is likely redundant now - grumbel: 22. Jul 2018
  request->type = GETLIGHT;
  request->pos = m_transform.apply(position);

  //There is no light offscreen.
  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
     || request->pos.x < 0 || request->pos.y < 0){
    *color_out = Color( 0, 0, 0);
    return;
  }

  request->layer = LAYER_GUI; //make sure all get_light requests are handled last.
  auto getlightrequest = new(m_obst) GetLightRequest();
  getlightrequest->color_ptr = color_out;
  request->request_data = getlightrequest;
  m_lightmap_requests.get_requests().push_back(request);
}

void
DrawingContext::do_drawing()
{
  assert(m_transformstack.empty());
  m_transformstack.clear();

  //Use Lightmap if ambient color is not white.
  bool use_lightmap = ( m_ambient_color.red != 1.0f ||
                        m_ambient_color.green != 1.0f ||
                        m_ambient_color.blue != 1.0f );

  // PART1: create lightmap
  if(use_lightmap) {
    auto& lightmap = m_video_system.get_lightmap();

    lightmap.start_draw(m_ambient_color);
    m_lightmap_requests.render(m_video_system);
    lightmap.end_draw();

    if (render_lighting) {
      auto request = new(m_obst) DrawingRequest();
      request->target = NORMAL;
      request->type = DRAW_LIGHTMAP;
      request->layer = LAYER_HUD - 1;
      m_drawing_requests.get_requests().push_back(request);
    }
  }

  Renderer& renderer = m_video_system.get_renderer();
  renderer.start_draw();
  m_drawing_requests.render(m_video_system);
  renderer.end_draw();

  m_lightmap_requests.clear();
  m_drawing_requests.clear();

  obstack_free(&m_obst, NULL);
  obstack_init(&m_obst);

  renderer.flip();
}

void
DrawingContext::set_ambient_color(Color ambient_color)
{
  m_ambient_color = ambient_color;
}

Rectf
DrawingContext::get_cliprect() const
{
  return Rectf(get_translation().x, get_translation().y,
               get_translation().x + SCREEN_WIDTH,
               get_translation().y + SCREEN_HEIGHT);
}

void
DrawingContext::set_drawing_effect(DrawingEffect effect)
{
  m_transform.drawing_effect = effect;
}

DrawingEffect
DrawingContext::get_drawing_effect() const
{
  return m_transform.drawing_effect;
}

void
DrawingContext::set_alpha(float alpha)
{
  m_transform.alpha = alpha;
}

float
DrawingContext::get_alpha() const
{
  return m_transform.alpha;
}

void
DrawingContext::push_transform()
{
  m_transformstack.push_back(m_transform);
}

void
DrawingContext::pop_transform()
{
  assert(!m_transformstack.empty());

  m_transform = m_transformstack.back();
  m_transformstack.pop_back();
}

/* EOF */
