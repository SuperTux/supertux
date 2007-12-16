//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <functional>
#include <algorithm>
#include <cassert>
#include <iostream>
//#include <SDL_image.h>
#include <sstream>
#include <iomanip>
#include <physfs.h>

#include "drawing_context.hpp"
//#include "drawing_request.hpp"
//#include "video_systems.hpp"
//#include "renderer.hpp"
//#include "lightmap.hpp"
#include "surface.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
//#include "texture.hpp"
//#include "texture_manager.hpp"
//#include "obstack/obstackpp.hpp"

#include "math/vector.hpp"
#include "math/rect.hpp"

#include <unison/video/Renderers.hpp>

/*static inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}*/

DrawingContext::DrawingContext()
  /*renderer(0), lightmap(0), ambient_color(1.0f, 1.0f, 1.0f, 1.0f), target(NORMAL), screenshot_requested(false)*/
{
  ambient_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
  target = NORMAL;
  screenshot_requested = false;
  draw_target = &normal_list;
  /*requests = &drawing_requests;
  obstack_init(&obst);*/
}

DrawingContext::~DrawingContext()
{
  /*delete renderer;
  delete lightmap;

  obstack_free(&obst, NULL);*/
}

void
DrawingContext::init_renderer()
{
  Unison::Video::Renderers::get().set_renderer(config->video);
  Unison::Video::Window::get().set_logical_size(Unison::Video::Area(SCREEN_WIDTH, SCREEN_HEIGHT));
  Unison::Video::Window::get().open(Unison::Video::Area(config->screenwidth, config->screenheight), config->use_fullscreen);
  lightmap = Unison::Video::Surface(Unison::Video::Area(SCREEN_WIDTH, SCREEN_HEIGHT));
  /*delete renderer;
  delete lightmap;

  renderer = new_renderer();
  lightmap = new_lightmap();*/
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
                             float angle, const Color& color, const Blend& blend,
                             int layer)
{
  assert(surface != 0);

  Unison::Video::RenderOptions options;
  options.color = color.to_unison_color();
  options.alpha = (unsigned char) transform.alpha * 0xff;
  options.blend = blend.to_unison_blend();
  options.h_flip = surface->get_flipx() != (transform.drawing_effect == HORIZONTAL_FLIP);
  options.v_flip = (transform.drawing_effect == VERTICAL_FLIP);

  Vector transformed = transform.apply(position);
  Unison::Video::Point dst_pos((int) transformed.x, (int) transformed.y);

  (*draw_target)[layer].blit_section(surface->get_texture(), dst_pos, options);

  /*DrawingRequest* request = new(obst) DrawingRequest();

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

  requests->push_back(request);*/
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

  Unison::Video::TextureSection texture = surface->get_texture();
  texture.clip_rect.pos.x += (int) source.x;
  texture.clip_rect.pos.y += (int) source.y;
  texture.clip_rect.size.x += (unsigned int) size.x;
  texture.clip_rect.size.y += (unsigned int) size.y;

  Unison::Video::RenderOptions options;
  options.alpha = (unsigned char) transform.alpha * 0xff;
  options.h_flip = surface->get_flipx() != (transform.drawing_effect == HORIZONTAL_FLIP);
  options.v_flip = (transform.drawing_effect == VERTICAL_FLIP);

  Vector transformed = transform.apply(dest);
  Unison::Video::Point dst_pos((int) transformed.x, (int) transformed.y);

  (*draw_target)[layer].blit_section(texture, dst_pos, options);

  /*DrawingRequest* request = new(obst) DrawingRequest();

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

  requests->push_back(request);*/
}

void
DrawingContext::draw_text(const Font* font, const std::string& text,
    const Vector& position, FontAlignment alignment, int layer)
{
  font->draw((*draw_target)[layer], text, transform.apply(position),
      alignment, transform.drawing_effect, transform.alpha);

  /*DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type = TEXT;
  request->pos = transform.apply(position);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  TextRequest* textrequest = new(obst) TextRequest();
  textrequest->font = font;
  textrequest->text = text;
  textrequest->alignment = alignment;
  request->request_data = textrequest;

  requests->push_back(request);*/
}

void
DrawingContext::draw_center_text(const Font* font, const std::string& text,
    const Vector& position, int layer)
{
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
      ALIGN_CENTER, layer);
}

namespace
{
  class GradientRequest : public Unison::Video::DisplayList::Request
  {
    public:
      GradientRequest(const Color &top, const Color &bottom) :
        top(top),
        bottom(bottom)
      {
      }

      void do_request(Unison::Video::Blittable *dst) const
      {
        for(int y = 0;y < SCREEN_HEIGHT;++y)
        {
          Unison::Video::Color color;
          color.red = (Uint8)((((float)(top.red-bottom.red)/(0-SCREEN_HEIGHT)) * y + top.red) * 255);
          color.green = (Uint8)((((float)(top.green-bottom.green)/(0-SCREEN_HEIGHT)) * y + top.green) * 255);
          color.green = (Uint8)((((float)(top.blue-bottom.blue)/(0-SCREEN_HEIGHT)) * y + top.blue) * 255);
          color.alpha = (Uint8)((((float)(top.alpha-bottom.alpha)/(0-SCREEN_HEIGHT)) * y + top.alpha) * 255);
          dst->fill(color, Unison::Video::Rect(0, y, SCREEN_WIDTH, 1));
        }
      }
    private:
      Color top;
      Color bottom;
  };
}

void
DrawingContext::draw_gradient(const Color& top, const Color& bottom, int layer)
{
  (*draw_target)[layer].add_request(new GradientRequest(top, bottom));

  /*DrawingRequest* request = new(obst) DrawingRequest();

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

  requests->push_back(request);*/
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  Vector transformed = transform.apply(topleft);

  Unison::Video::Rect rect;
  rect.pos = Unison::Video::Point((int) transformed.x, (int) transformed.y);
  rect.size.x = (unsigned int) size.x;
  rect.size.y = (unsigned int) size.y;

  (*draw_target)[layer].fill_blend(color.to_unison_color(), rect);

  /*DrawingRequest* request = new(obst) DrawingRequest();

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
  request->request_data = fillrectrequest;

  requests->push_back(request);*/
}

void
DrawingContext::draw_filled_rect(const Rect& rect, const Color& color,
                                 int layer)
{
  Vector transformed = transform.apply(rect.p1);

  Unison::Video::Rect unison_rect;
  unison_rect.pos.x = (int) transformed.x;
  unison_rect.pos.y = (int) transformed.y;
  unison_rect.size.x = (unsigned int) rect.get_width();
  unison_rect.size.y = (unsigned int) rect.get_height();

  (*draw_target)[layer].fill_blend(color.to_unison_color(), unison_rect);

  /*DrawingRequest* request = new(obst) DrawingRequest();

  request->target = target;
  request->type = FILLRECT;
  request->pos = transform.apply(rect.p1);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  FillRectRequest* fillrectrequest = new(obst) FillRectRequest;
  fillrectrequest->size = Vector(rect.get_width(), rect.get_height());
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  request->request_data = fillrectrequest;

  requests->push_back(request);*/
}

void
DrawingContext::get_light(const Vector& position, Color* color)
{
  if( ambient_color.red == 1.0f && ambient_color.green == 1.0f
      && ambient_color.blue  == 1.0f ) {
    *color = Color( 1.0f, 1.0f, 1.0f);
    return;
  }

  /*DrawingRequest* request = new(obst) DrawingRequest();
  request->target = target;
  request->type = GETLIGHT;
  request->pos = transform.apply(position);*/

  //There is no light offscreen.
  if(position.x >= SCREEN_WIDTH || position.y >= SCREEN_HEIGHT
      || position.x < 0 || position.y < 0){
    *color = Color( 0, 0, 0);
    return;
  }

  Vector transformed = transform.apply(position);
  Unison::Video::Point pos((int) transformed.x, (int) transformed.y);
  get_light_requests.push_back(std::make_pair(pos, color));

  /*request->layer = LAYER_GUI; //make sure all get_light requests are handled last.
  GetLightRequest* getlightrequest = new(obst) GetLightRequest();
  getlightrequest->color_ptr = color;
  request->request_data = getlightrequest;
  lightmap_requests.push_back(request);*/
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
    lightmap.fill(ambient_color.to_unison_color());
    lightmap.draw(lightmap_list);
    //lightmap->start_draw(ambient_color);
    //handle_drawing_requests(lightmap_requests);
    //lightmap->end_draw();
  }

  Unison::Video::Window::get().draw(normal_list);

  //handle_drawing_requests(drawing_requests);
  if(use_lightmap) {
    Unison::Video::Window::get().blit(lightmap, Unison::Video::Point(), Unison::Video::Rect(), Unison::Video::BLEND_MOD);
    //lightmap->do_draw();
  }
  //obstack_free(&obst, NULL);
  //obstack_init(&obst);

  // if a screenshot was requested, take one
  if (screenshot_requested) {
    // FIXME renderer->do_take_screenshot();
    screenshot_requested = false;
  }

  //renderer->flip();
  Unison::Video::Window::get().flip();

  normal_list.clear();
  lightmap_list.clear();
}

/*class RequestPtrCompare
  :  public std::binary_function<const DrawingRequest*,
  std::vector<std::pair<Unison::Video::Point, Color *> > get_light_requests;
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
                  textrequest->alignment, request.drawing_effect, request.alpha);
            }
            break;
          case FILLRECT:
            renderer->draw_filled_rect(request);
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
                  textrequest->alignment, request.drawing_effect, request.alpha);
            }
            break;
          case FILLRECT:
            lightmap->draw_filled_rect(request);
            break;
          case GETLIGHT:
            lightmap->get_light(request);
            break;
        }
        break;
    }
  }
  requests.clear();
}*/

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
    draw_target = &lightmap_list;
  } else {
    assert(target == NORMAL);
    draw_target = &normal_list;
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

