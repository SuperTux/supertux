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
  m_transformstack(),
  m_transform(),
  m_drawing_requests(),
  m_lightmap_requests(),
  m_requests(&m_drawing_requests),
  m_ambient_color(1.0f, 1.0f, 1.0f, 1.0f),
  m_target(NORMAL),
  m_target_stack(),
  m_obst(),
  m_screenshot_requested(false)
{
  obstack_init(&m_obst);
}

DrawingContext::~DrawingContext()
{
  clear_drawing_requests(m_lightmap_requests);
  clear_drawing_requests(m_drawing_requests);

  obstack_free(&m_obst, NULL);
}

void
DrawingContext::clear_drawing_requests(DrawingRequests& requests_)
{
  for(auto& request : requests_)
  {
    if (request->request_data)
    {
      request->request_data->~DrawingRequestData();
    }
    request->~DrawingRequest();
  }
  requests_.clear();
}

void
DrawingContext::draw_surface(SurfacePtr surface, const Vector& position,
                             float angle, const Color& color, const Blend& blend,
                             int layer)
{
  assert(surface != 0);

  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = SURFACE;
  request->pos = m_transform.apply(position);

  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
     || request->pos.x + surface->get_width() < 0
     || request->pos.y + surface->get_height() < 0)
    return;

  request->layer = layer;
  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;
  request->angle = angle;
  request->color = color;
  request->blend = blend;

  auto surfacerequest = new(m_obst) SurfaceRequest();
  surfacerequest->surface = surface.get();
  request->request_data = surfacerequest;

  m_requests->push_back(request);
}

void
DrawingContext::draw_surface(SurfacePtr surface, const Vector& position,
                             int layer)
{
  draw_surface(surface, position, 0.0f, Color(1.0f, 1.0f, 1.0f), Blend(), layer);
}

void
DrawingContext::draw_surface_part(SurfacePtr surface,
                                  const Rectf& srcrect, const Rectf& dstrect,
                                  int layer)
{
  assert(surface != 0);

  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = SURFACE_PART;
  request->pos = m_transform.apply(dstrect.p1);
  request->layer = layer;
  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto surfacepartrequest = new(m_obst) SurfacePartRequest();
  surfacepartrequest->srcrect = srcrect;
  surfacepartrequest->dstsize = dstrect.get_size();
  surfacepartrequest->surface = surface.get();

  request->request_data = surfacepartrequest;

  m_requests->push_back(request);
}

void
DrawingContext::draw_text(FontPtr font, const std::string& text,
                          const Vector& position, FontAlignment alignment, int layer, Color color)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = TEXT;
  request->pos = m_transform.apply(position);
  request->layer = layer;
  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;
  request->color = color;

  auto textrequest = new(m_obst) TextRequest();
  textrequest->font = font.get();
  textrequest->text = text;
  textrequest->alignment = alignment;
  request->request_data = textrequest;

  m_requests->push_back(request);
}

void
DrawingContext::draw_center_text(FontPtr font, const std::string& text,
                                 const Vector& position, int layer, Color color)
{
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
            ALIGN_CENTER, layer, color);
}

void
DrawingContext::draw_gradient(const Color& top, const Color& bottom, int layer,
                              const GradientDirection& direction, const Rectf& region)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = GRADIENT;
  request->pos = Vector(0,0);
  request->layer = layer;

  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto gradientrequest = new(m_obst) GradientRequest();
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  gradientrequest->direction = direction;
  gradientrequest->region = region;
  request->request_data = gradientrequest;

  m_requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = FILLRECT;
  request->pos = m_transform.apply(topleft);
  request->layer = layer;

  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto fillrectrequest = new(m_obst) FillRectRequest();
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * m_transform.alpha;
  fillrectrequest->radius = 0.0f;
  request->request_data = fillrectrequest;

  m_requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Rectf& rect, const Color& color,
                                 int layer)
{
  draw_filled_rect(rect, color, 0.0f, layer);
}

void
DrawingContext::draw_filled_rect(const Rectf& rect, const Color& color, float radius, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = FILLRECT;
  request->pos    = m_transform.apply(rect.p1);
  request->layer  = layer;

  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto fillrectrequest = new(m_obst) FillRectRequest;
  fillrectrequest->size = Vector(rect.get_width(), rect.get_height());
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * m_transform.alpha;
  fillrectrequest->radius = radius;
  request->request_data = fillrectrequest;

  m_requests->push_back(request);
}

void
DrawingContext::draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = INVERSEELLIPSE;
  request->pos    = m_transform.apply(pos);
  request->layer  = layer;

  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto ellipse = new(m_obst)InverseEllipseRequest;

  ellipse->color        = color;
  ellipse->color.alpha  = color.alpha * m_transform.alpha;
  ellipse->size         = size;
  request->request_data = ellipse;

  m_requests->push_back(request);
}

void
DrawingContext::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = LINE;
  request->pos    = m_transform.apply(pos1);
  request->layer  = layer;

  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto line = new(m_obst) LineRequest;

  line->color        = color;
  line->color.alpha  = color.alpha * m_transform.alpha;
  line->dest_pos     = m_transform.apply(pos2);
  request->request_data = line;

  m_requests->push_back(request);
}

void
DrawingContext::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = TRIANGLE;
  request->pos    = m_transform.apply(pos1);
  request->layer  = layer;

  request->drawing_effect = m_transform.drawing_effect;
  request->alpha = m_transform.alpha;

  auto triangle = new(m_obst) TriangleRequest;

  triangle->color        = color;
  triangle->color.alpha  = color.alpha * m_transform.alpha;
  triangle->pos2         = m_transform.apply(pos2);
  triangle->pos3         = m_transform.apply(pos3);
  request->request_data = triangle;

  m_requests->push_back(request);
}

Rectf
DrawingContext::get_cliprect() const
{
  return Rectf(get_translation().x, get_translation().y,
               get_translation().x + SCREEN_WIDTH,
               get_translation().y + SCREEN_HEIGHT);
}

void
DrawingContext::get_light(const Vector& position, Color* color)
{
  if (m_ambient_color.red == 1.0f &&
      m_ambient_color.green == 1.0f &&
      m_ambient_color.blue == 1.0f)
  {
    *color = Color( 1.0f, 1.0f, 1.0f);
    return;
  }

  auto request = new(m_obst) DrawingRequest();
  request->target = m_target;
  request->type = GETLIGHT;
  request->pos = m_transform.apply(position);

  //There is no light offscreen.
  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
     || request->pos.x < 0 || request->pos.y < 0){
    *color = Color( 0, 0, 0);
    return;
  }

  request->layer = LAYER_GUI; //make sure all get_light requests are handled last.
  auto getlightrequest = new(m_obst) GetLightRequest();
  getlightrequest->color_ptr = color;
  request->request_data = getlightrequest;
  m_lightmap_requests.push_back(request);
}

void
DrawingContext::do_drawing()
{
  assert(m_transformstack.empty());
  assert(m_target_stack.empty());
  m_transformstack.clear();
  m_target_stack.clear();

  //Use Lightmap if ambient color is not white.
  bool use_lightmap = ( m_ambient_color.red != 1.0f ||
                        m_ambient_color.green != 1.0f ||
                        m_ambient_color.blue != 1.0f );

  // PART1: create lightmap
  if(use_lightmap) {
    auto& lightmap = m_video_system.get_lightmap();

    lightmap.start_draw(m_ambient_color);
    handle_drawing_requests(m_lightmap_requests);
    lightmap.end_draw();

    if (render_lighting) {
      auto request = new(m_obst) DrawingRequest();
      request->target = NORMAL;
      request->type = DRAW_LIGHTMAP;
      request->layer = LAYER_HUD - 1;
      m_drawing_requests.push_back(request);
    }
  }

  Renderer& renderer = m_video_system.get_renderer();
  renderer.start_draw();
  handle_drawing_requests(m_drawing_requests);
  renderer.end_draw();

  clear_drawing_requests(m_lightmap_requests);
  clear_drawing_requests(m_drawing_requests);

  obstack_free(&m_obst, NULL);
  obstack_init(&m_obst);

  // if a screenshot was requested, take one
  if (m_screenshot_requested) {
    renderer.do_take_screenshot();
    m_screenshot_requested = false;
  }

  renderer.flip();
}

class RequestPtrCompare
{
public:
  bool operator()(const DrawingRequest* r1, const DrawingRequest* r2) const
  {
    return *r1 < *r2;
  }
};

void
DrawingContext::handle_drawing_requests(DrawingRequests& requests_)
{
  std::stable_sort(requests_.begin(), requests_.end(), RequestPtrCompare());

  Renderer& renderer = m_video_system.get_renderer();
  Lightmap& lightmap = m_video_system.get_lightmap();

  DrawingRequests::const_iterator i;
  for(i = requests_.begin(); i != requests_.end(); ++i) {
    const DrawingRequest& request = **i;

    switch(request.target) {
      case NORMAL:
        switch(request.type) {
          case SURFACE:
            renderer.draw_surface(request);
            break;
          case SURFACE_PART:
            renderer.draw_surface_part(request);
            break;
          case GRADIENT:
            renderer.draw_gradient(request);
            break;
          case TEXT:
          {
            const auto textrequest = static_cast<TextRequest*>(request.request_data);
            textrequest->font->draw(&renderer, textrequest->text, request.pos,
                                    textrequest->alignment, request.drawing_effect, request.color, request.alpha);
          }
          break;
          case FILLRECT:
            renderer.draw_filled_rect(request);
            break;
          case INVERSEELLIPSE:
            renderer.draw_inverse_ellipse(request);
            break;
          case DRAW_LIGHTMAP:
            lightmap.do_draw();
            break;
          case GETLIGHT:
            lightmap.get_light(request);
            break;
          case LINE:
            renderer.draw_line(request);
            break;
          case TRIANGLE:
            renderer.draw_triangle(request);
            break;
        }
        break;
      case LIGHTMAP:
        switch(request.type) {
          case SURFACE:
            lightmap.draw_surface(request);
            break;
          case SURFACE_PART:
            lightmap.draw_surface_part(request);
            break;
          case GRADIENT:
            lightmap.draw_gradient(request);
            break;
          case TEXT:
          {
            const auto textrequest = static_cast<TextRequest*>(request.request_data);
            textrequest->font->draw(&renderer, textrequest->text, request.pos,
                                    textrequest->alignment, request.drawing_effect, request.color, request.alpha);
          }
          break;
          case FILLRECT:
            lightmap.draw_filled_rect(request);
            break;
          case INVERSEELLIPSE:
            assert(!"InverseEllipse doesn't make sense on the lightmap");
            break;
          case DRAW_LIGHTMAP:
            lightmap.do_draw();
            break;
          case GETLIGHT:
            lightmap.get_light(request);
            break;
          case LINE:
            lightmap.draw_line(request);
            break;
          case TRIANGLE:
            lightmap.draw_triangle(request);
            break;
        }
        break;
    }
  }
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
DrawingContext::push_target()
{
  m_target_stack.push_back(m_target);
}

void
DrawingContext::pop_target()
{
  set_target(m_target_stack.back());
  m_target_stack.pop_back();
}

void
DrawingContext::set_target(Target target_)
{
  m_target = target_;

  if(m_target == LIGHTMAP) {
    m_requests = &m_lightmap_requests;
  } else {
    assert(m_target == NORMAL);
    m_requests = &m_drawing_requests;
  }
}

void
DrawingContext::set_ambient_color( Color new_color )
{
  m_ambient_color = new_color;
}

void
DrawingContext::take_screenshot()
{
  m_screenshot_requested = true;
}

/* EOF */
