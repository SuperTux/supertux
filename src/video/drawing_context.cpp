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
#include <config.h>
#include <algorithm>

#include "video/drawing_context.hpp"

#include "video/drawing_request.hpp"
#include "video/video_systems.hpp"
#include "video/renderer.hpp"
#include "video/lightmap.hpp"
#include "video/surface.hpp"
#include "supertux/main.hpp"
#include "supertux/gameconfig.hpp"
#include "video/texture.hpp"
#include "video/texture_manager.hpp"
#include "obstack/obstackpp.hpp"

static inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}

DrawingContext::DrawingContext() :
  renderer(0), lightmap(0), ambient_color(1.0f, 1.0f, 1.0f, 1.0f), target(NORMAL), screenshot_requested(false)
{
  requests = &drawing_requests;
  obstack_init(&obst);
}

DrawingContext::~DrawingContext()
{
  delete renderer;
  delete lightmap;

  obstack_free(&obst, NULL);
}

void
DrawingContext::init_renderer()
{
  delete renderer;
  delete lightmap;

  renderer = new_renderer();
  lightmap = new_lightmap();
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
                             float angle, const Color& color, const Blend& blend,
                             int layer)
{
  assert(surface != 0);

  DrawingRequest* request = new(obst) DrawingRequest();

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

  request->request_data = const_cast<Surface*> (surface);

  requests->push_back(request);
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
    int layer)
{
  draw_surface(surface, position, 0.0f, Color(1.0f, 1.0f, 1.0f), Blend(), layer);
}

void
DrawingContext::draw_surface_part(const Surface* surface, const Vector& source,
    const Vector& size, const Vector& dest, int layer)
{
  assert(surface != 0);

  DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type = SURFACE_PART;
  request->pos = transform.apply(dest);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  SurfacePartRequest* surfacepartrequest = new(obst) SurfacePartRequest();
  surfacepartrequest->size = size;
  surfacepartrequest->source = source;
  surfacepartrequest->surface = surface;

  // clip on screen borders
  if(request->pos.x < 0) {
    surfacepartrequest->size.x += request->pos.x;
    if(surfacepartrequest->size.x <= 0)
      return;
    surfacepartrequest->source.x -= request->pos.x;
    request->pos.x = 0;
  }
  if(request->pos.y < 0) {
    surfacepartrequest->size.y += request->pos.y;
    if(surfacepartrequest->size.y <= 0)
      return;
    surfacepartrequest->source.y -= request->pos.y;
    request->pos.y = 0;
  }
  request->request_data = surfacepartrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_text(const Font* font, const std::string& text,
    const Vector& position, FontAlignment alignment, int layer, Color color)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type = TEXT;
  request->pos = transform.apply(position);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;
  request->color = color;

  TextRequest* textrequest = new(obst) TextRequest();
  textrequest->font = font;
  textrequest->text = text;
  textrequest->alignment = alignment;
  request->request_data = textrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_center_text(const Font* font, const std::string& text,
    const Vector& position, int layer, Color color)
{
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
      ALIGN_CENTER, layer, color);
}

void
DrawingContext::draw_gradient(const Color& top, const Color& bottom, int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type = GRADIENT;
  request->pos = Vector(0,0);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  GradientRequest* gradientrequest = new(obst) GradientRequest();
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  request->request_data = gradientrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type = FILLRECT;
  request->pos = transform.apply(topleft);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  FillRectRequest* fillrectrequest = new(obst) FillRectRequest();
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  fillrectrequest->radius = 0.0f;
  request->request_data = fillrectrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Rect& rect, const Color& color,
                                 int layer)
{
  draw_filled_rect(rect, color, 0.0f, layer);
}

void
DrawingContext::draw_filled_rect(const Rect& rect, const Color& color, float radius, int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type   = FILLRECT;
  request->pos    = transform.apply(rect.p1);
  request->layer  = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  FillRectRequest* fillrectrequest = new(obst) FillRectRequest;
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
  DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type   = INVERSEELLIPSE;
  request->pos    = transform.apply(pos);
  request->layer  = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  InverseEllipseRequest* ellipse = new(obst)InverseEllipseRequest;
  
  ellipse->color        = color;
  ellipse->color.alpha  = color.alpha * transform.alpha;
  ellipse->size         = size;
  request->request_data = ellipse;

  requests->push_back(request);     
}

void
DrawingContext::get_light(const Vector& position, Color* color)
{
  if( ambient_color.red == 1.0f && ambient_color.green == 1.0f
      && ambient_color.blue  == 1.0f ) {
    *color = Color( 1.0f, 1.0f, 1.0f);
    return;
  }

  DrawingRequest* request = new(obst) DrawingRequest();
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
  GetLightRequest* getlightrequest = new(obst) GetLightRequest();
  getlightrequest->color_ptr = color;
  request->request_data = getlightrequest;
  lightmap_requests.push_back(request);
}

void
DrawingContext::do_drawing()
{
#ifdef DEBUG
  assert(transformstack.empty());
  assert(target_stack.empty());
#endif
  transformstack.clear();
  target_stack.clear();

  //Use Lightmap if ambient color is not white.
  bool use_lightmap = ( ambient_color.red != 1.0f   || ambient_color.green != 1.0f ||
                        ambient_color.blue  != 1.0f );

  // PART1: create lightmap
  if(use_lightmap) {
    lightmap->start_draw(ambient_color);
    handle_drawing_requests(lightmap_requests);
    lightmap->end_draw();

    DrawingRequest* request = new(obst) DrawingRequest();
    request->target = NORMAL;
    request->type = DRAW_LIGHTMAP;
    request->layer = LAYER_HUD - 1;
    drawing_requests.push_back(request);
  }
  lightmap_requests.clear();

  handle_drawing_requests(drawing_requests);
  drawing_requests.clear();
  obstack_free(&obst, NULL);
  obstack_init(&obst);

  // if a screenshot was requested, take one
  if (screenshot_requested) {
    renderer->do_take_screenshot();
    screenshot_requested = false;
  }

  renderer->flip();
}

class RequestPtrCompare
  :  public std::binary_function<const DrawingRequest*,
                                 const DrawingRequest*, 
                                 bool>
{
public:
  bool operator()(const DrawingRequest* r1, const DrawingRequest* r2) const
  {
    return *r1 < *r2;
  }
};

void
DrawingContext::handle_drawing_requests(DrawingRequests& requests)
{
  std::stable_sort(requests.begin(), requests.end(), RequestPtrCompare());

  DrawingRequests::const_iterator i;
  for(i = requests.begin(); i != requests.end(); ++i) {
    const DrawingRequest& request = **i;

    switch(request.target) {
      case NORMAL:
        switch(request.type) {
          case SURFACE:
            renderer->draw_surface(request);
            break;
          case SURFACE_PART:
            renderer->draw_surface_part(request);
            break;
          case GRADIENT:
            renderer->draw_gradient(request);
            break;
          case TEXT:
            {
              const TextRequest* textrequest = (TextRequest*) request.request_data;
              textrequest->font->draw(renderer, textrequest->text, request.pos,
                  textrequest->alignment, request.drawing_effect, request.color, request.alpha);
            }
            break;
          case FILLRECT:
            renderer->draw_filled_rect(request);
            break;
          case INVERSEELLIPSE:
            renderer->draw_inverse_ellipse(request);
            break;
          case DRAW_LIGHTMAP:
            lightmap->do_draw();
            break;
          case GETLIGHT:
            lightmap->get_light(request);
            break;
        }
        break;
      case LIGHTMAP:
        switch(request.type) {
          case SURFACE:
            lightmap->draw_surface(request);
            break;
          case SURFACE_PART:
            lightmap->draw_surface_part(request);
            break;
          case GRADIENT:
            lightmap->draw_gradient(request);
            break;
          case TEXT:
            {
              const TextRequest* textrequest = (TextRequest*) request.request_data;
              textrequest->font->draw(renderer, textrequest->text, request.pos,
                  textrequest->alignment, request.drawing_effect, request.color, request.alpha);
            }
            break;
          case FILLRECT:
            lightmap->draw_filled_rect(request);
            break;
          case INVERSEELLIPSE:
            assert(!"InverseEllipse doesn't make sense on the lightmap");
            break;
          case DRAW_LIGHTMAP:
            lightmap->do_draw();
            break;
          case GETLIGHT:
            lightmap->get_light(request);
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
DrawingContext::set_target(Target target)
{
  this->target = target;
  if(target == LIGHTMAP) {
    requests = &lightmap_requests;
  } else {
    assert(target == NORMAL);
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

