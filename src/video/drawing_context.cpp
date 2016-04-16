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
#include <config.h>

#include "math/sizef.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/obstackpp.hpp"
#include "video/drawing_request.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/texture.hpp"
#include "video/texture_manager.hpp"
#include "video/video_system.hpp"

bool DrawingContext::render_lighting = true;

DrawingContext::DrawingContext(VideoSystem& video_system_) :
  video_system(video_system_),
  transformstack(),
  transform(),
  blend_stack(),
  blend_mode(),
  drawing_requests(),
  lightmap_requests(),
  requests(&drawing_requests),
  ambient_color(1.0f, 1.0f, 1.0f, 1.0f),
  target(NORMAL),
  target_stack(),
  obst(),
  screenshot_requested(false)
{
  obstack_init(&obst);
}

DrawingContext::~DrawingContext()
{
  clear_drawing_requests(lightmap_requests);
  clear_drawing_requests(drawing_requests);

  obstack_free(&obst, NULL);
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

  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type = SURFACE;
  request->pos = transform.apply(position);

  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
     || request->pos.x + surface->get_width() < 0
     || request->pos.y + surface->get_height() < 0)
    return;

  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;
  request->angle = angle;
  request->color = color;
  request->blend = blend;

  auto surfacerequest = new(obst) SurfaceRequest();
  surfacerequest->surface = surface.get();
  request->request_data = surfacerequest;

  requests->push_back(request);
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

  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type = SURFACE_PART;
  request->pos = transform.apply(dstrect.p1);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto surfacepartrequest = new(obst) SurfacePartRequest();
  surfacepartrequest->srcrect = srcrect;
  surfacepartrequest->dstsize = dstrect.get_size();
  surfacepartrequest->surface = surface.get();

  request->request_data = surfacepartrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_text(FontPtr font, const std::string& text,
                          const Vector& position, FontAlignment alignment, int layer, Color color)
{
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type = TEXT;
  request->pos = transform.apply(position);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;
  request->color = color;

  auto textrequest = new(obst) TextRequest();
  textrequest->font = font.get();
  textrequest->text = text;
  textrequest->alignment = alignment;
  request->request_data = textrequest;

  requests->push_back(request);
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
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type = GRADIENT;
  request->pos = Vector(0,0);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto gradientrequest = new(obst) GradientRequest();
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  gradientrequest->direction = direction;
  gradientrequest->region = region;
  request->request_data = gradientrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type = FILLRECT;
  request->pos = transform.apply(topleft);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto fillrectrequest = new(obst) FillRectRequest();
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  fillrectrequest->radius = 0.0f;
  request->request_data = fillrectrequest;

  requests->push_back(request);
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
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type   = FILLRECT;
  request->pos    = transform.apply(rect.p1);
  request->layer  = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto fillrectrequest = new(obst) FillRectRequest;
  fillrectrequest->size = Vector(rect.get_width(), rect.get_height());
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  fillrectrequest->radius = radius;
  request->request_data = fillrectrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer)
{
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type   = INVERSEELLIPSE;
  request->pos    = transform.apply(pos);
  request->layer  = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto ellipse = new(obst)InverseEllipseRequest;

  ellipse->color        = color;
  ellipse->color.alpha  = color.alpha * transform.alpha;
  ellipse->size         = size;
  request->request_data = ellipse;

  requests->push_back(request);
}

void
DrawingContext::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type   = LINE;
  request->pos    = transform.apply(pos1);
  request->layer  = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto line = new(obst) LineRequest;

  line->color        = color;
  line->color.alpha  = color.alpha * transform.alpha;
  line->dest_pos     = transform.apply(pos2);
  request->request_data = line;

  requests->push_back(request);
}

void
DrawingContext::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto request = new(obst) DrawingRequest();

  request->target = target;
  request->type   = TRIANGLE;
  request->pos    = transform.apply(pos1);
  request->layer  = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  auto triangle = new(obst) TriangleRequest;

  triangle->color        = color;
  triangle->color.alpha  = color.alpha * transform.alpha;
  triangle->pos2         = transform.apply(pos2);
  triangle->pos3         = transform.apply(pos3);
  request->request_data = triangle;

  requests->push_back(request);
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
  if( ambient_color.red == 1.0f && ambient_color.green == 1.0f
      && ambient_color.blue  == 1.0f ) {
    *color = Color( 1.0f, 1.0f, 1.0f);
    return;
  }

  auto request = new(obst) DrawingRequest();
  request->target = target;
  request->type = GETLIGHT;
  request->pos = transform.apply(position);

  //There is no light offscreen.
  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
     || request->pos.x < 0 || request->pos.y < 0){
    *color = Color( 0, 0, 0);
    return;
  }

  request->layer = LAYER_GUI; //make sure all get_light requests are handled last.
  auto getlightrequest = new(obst) GetLightRequest();
  getlightrequest->color_ptr = color;
  request->request_data = getlightrequest;
  lightmap_requests.push_back(request);
}

void
DrawingContext::do_drawing()
{
  assert(transformstack.empty());
  assert(target_stack.empty());
  transformstack.clear();
  target_stack.clear();

  //Use Lightmap if ambient color is not white.
  bool use_lightmap = ( ambient_color.red != 1.0f ||
                        ambient_color.green != 1.0f ||
                        ambient_color.blue != 1.0f );

  // PART1: create lightmap
  if(use_lightmap) {
    auto& lightmap = video_system.get_lightmap();

    lightmap.start_draw(ambient_color);
    handle_drawing_requests(lightmap_requests);
    lightmap.end_draw();

    if (render_lighting) {
      auto request = new(obst) DrawingRequest();
      request->target = NORMAL;
      request->type = DRAW_LIGHTMAP;
      request->layer = LAYER_HUD - 1;
      drawing_requests.push_back(request);
    }
  }

  Renderer& renderer = video_system.get_renderer();
  renderer.start_draw();
  handle_drawing_requests(drawing_requests);
  renderer.end_draw();

  clear_drawing_requests(lightmap_requests);
  clear_drawing_requests(drawing_requests);

  obstack_free(&obst, NULL);
  obstack_init(&obst);

  // if a screenshot was requested, take one
  if (screenshot_requested) {
    renderer.do_take_screenshot();
    screenshot_requested = false;
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

  Renderer& renderer = video_system.get_renderer();
  Lightmap& lightmap = video_system.get_lightmap();

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
            renderer.draw_text(request);
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
            renderer.draw_text(request);
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
  transformstack.push_back(transform);
}

void
DrawingContext::pop_transform()
{
  assert(!transformstack.empty());

  transform = transformstack.back();
  transformstack.pop_back();
}

void
DrawingContext::set_drawing_effect(DrawingEffect effect)
{
  transform.drawing_effect = effect;
}

DrawingEffect
DrawingContext::get_drawing_effect() const
{
  return transform.drawing_effect;
}

void
DrawingContext::set_alpha(float alpha)
{
  transform.alpha = alpha;
}

float
DrawingContext::get_alpha() const
{
  return transform.alpha;
}

void
DrawingContext::push_target()
{
  target_stack.push_back(target);
}

void
DrawingContext::pop_target()
{
  set_target(target_stack.back());
  target_stack.pop_back();
}

void
DrawingContext::set_target(Target target_)
{
  this->target = target_;
  if(target_ == LIGHTMAP) {
    requests = &lightmap_requests;
  } else {
    assert(target_ == NORMAL);
    requests = &drawing_requests;
  }
}

void
DrawingContext::set_ambient_color( Color new_color )
{
  ambient_color = new_color;
}

void
DrawingContext::take_screenshot()
{
  screenshot_requested = true;
}

/* EOF */
