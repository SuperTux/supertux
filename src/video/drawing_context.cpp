//  $Id: drawing_context.cpp 2334 2005-04-04 16:26:14Z grumbel $
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <SDL_image.h>

#include "drawing_context.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "glutil.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"

#define LIGHTMAP_DIV 4

static inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;
  
  return result;
}

DrawingContext::DrawingContext()
{
  screen = SDL_GetVideoSurface();

  target = NORMAL;
  requests = &drawing_requests;
}

DrawingContext::~DrawingContext()
{
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
    int layer)
{
  if(target != NORMAL) return;

  assert(surface != 0);
  
  DrawingRequest request;

  request.type = SURFACE;
  request.pos = transform.apply(position);

  if(request.pos.x >= SCREEN_WIDTH || request.pos.y >= SCREEN_HEIGHT
      || request.pos.x + surface->get_width() < 0 
      || request.pos.y + surface->get_height() < 0)
    return;

  request.layer = layer;
  request.drawing_effect = transform.drawing_effect;
  request.alpha = transform.alpha;
  request.request_data = const_cast<Surface*> (surface);  

  requests->push_back(request);
}

void
DrawingContext::draw_surface_part(const Surface* surface, const Vector& source,
    const Vector& size, const Vector& dest, int layer)
{
  if(target != NORMAL) return;

  assert(surface != 0);

  DrawingRequest request;

  request.type = SURFACE_PART;
  request.pos = transform.apply(dest);
  request.layer = layer;
  request.drawing_effect = transform.drawing_effect;
  request.alpha = transform.alpha;
  
  SurfacePartRequest* surfacepartrequest = new SurfacePartRequest();
  surfacepartrequest->size = size;
  surfacepartrequest->source = source;
  surfacepartrequest->surface = surface;

  // clip on screen borders
  if(request.pos.x < 0) {
    surfacepartrequest->size.x += request.pos.x;
    if(surfacepartrequest->size.x <= 0)
      return;
    surfacepartrequest->source.x -= request.pos.x;
    request.pos.x = 0;
  }
  if(request.pos.y < 0) {
    surfacepartrequest->size.y += request.pos.y;
    if(surfacepartrequest->size.y <= 0)
      return;
    surfacepartrequest->source.y -= request.pos.y;
    request.pos.y = 0;
  }
  request.request_data = surfacepartrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_text(const Font* font, const std::string& text,
    const Vector& position, FontAlignment alignment, int layer)
{
  if(target != NORMAL) return;

  DrawingRequest request;

  request.type = TEXT;
  request.pos = transform.apply(position);
  request.layer = layer;
  request.drawing_effect = transform.drawing_effect;
  request.alpha = transform.alpha;

  TextRequest* textrequest = new TextRequest;
  textrequest->font = font;
  textrequest->text = text;
  textrequest->alignment = alignment;
  request.request_data = textrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_center_text(const Font* font, const std::string& text,
    const Vector& position, int layer)
{
  if(target != NORMAL) return;

  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
      CENTER_ALLIGN, layer);
}

void
DrawingContext::draw_gradient(const Color& top, const Color& bottom, int layer)
{
  if(target != NORMAL) return;

  DrawingRequest request;

  request.type = GRADIENT;
  request.pos = Vector(0,0);
  request.layer = layer;

  request.drawing_effect = transform.drawing_effect;
  request.alpha = transform.alpha;

  GradientRequest* gradientrequest = new GradientRequest;
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  request.request_data = gradientrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  if(target != NORMAL) return;

  DrawingRequest request;

  request.type = FILLRECT;
  request.pos = transform.apply(topleft);
  request.layer = layer;

  request.drawing_effect = transform.drawing_effect;
  request.alpha = transform.alpha;                    

  FillRectRequest* fillrectrequest = new FillRectRequest;
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  request.request_data = fillrectrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_surface_part(DrawingRequest& request)
{
  SurfacePartRequest* surfacepartrequest
    = (SurfacePartRequest*) request.request_data;

  surfacepartrequest->surface->draw_part(
      surfacepartrequest->source.x, surfacepartrequest->source.y,
      request.pos.x, request.pos.y,
      surfacepartrequest->size.x, surfacepartrequest->size.y,
      request.alpha, request.drawing_effect);

  delete surfacepartrequest;
}

namespace
{
  void fillrect(SDL_Surface* screen, float x, float y, float w, float h, int r, int g, int b, int a)
  {
    if(w < 0) {
      x += w;
      w = -w;
    }
    if(h < 0) {
      y += h;
      h = -h;
    }

    SDL_Rect src, rect;
    SDL_Surface *temp = NULL;

    rect.x = (int)x;
    rect.y = (int)y;
    rect.w = (int)w;
    rect.h = (int)h;

    if(a != 255) {
      temp = SDL_CreateRGBSurface(screen->flags, rect.w, rect.h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

      src.x = 0;
      src.y = 0;
      src.w = rect.w;
      src.h = rect.h;

      SDL_FillRect(temp, &src, SDL_MapRGB(screen->format, r, g, b));
      SDL_SetAlpha(temp, SDL_SRCALPHA, a);
      SDL_BlitSurface(temp,0,screen,&rect);
      SDL_FreeSurface(temp);
    } else {
      SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, r, g, b));
    }
  }
}

void
DrawingContext::draw_gradient(DrawingRequest& request)
{
  GradientRequest* gradientrequest = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  int width = 800;
  int height = 600;
  for(float y = 0; y < height; y += 2) ::fillrect(screen, 0, (int)y, width, 2, (int)(((float)(top.red-bottom.red)/(0-height)) * y + top.red), (int)(((float)(top.green-bottom.green)/(0-height)) * y + top.green), (int)(((float)(top.blue-bottom.blue)/(0-height)) * y + top.blue), 255);

  delete gradientrequest;
}

void
DrawingContext::draw_text(DrawingRequest& request)
{
  TextRequest* textrequest = (TextRequest*) request.request_data;

  textrequest->font->draw(textrequest->text, request.pos,
      textrequest->alignment, request.drawing_effect, request.alpha);

  delete textrequest;
}

void
DrawingContext::draw_filled_rect(DrawingRequest& request)
{
  FillRectRequest* fillrectrequest = (FillRectRequest*) request.request_data;

  float x = request.pos.x;
  float y = request.pos.y;
  float w = fillrectrequest->size.x;
  float h = fillrectrequest->size.y;

  int r = static_cast<int>(fillrectrequest->color.red);
  int g = static_cast<int>(fillrectrequest->color.green);
  int b = static_cast<int>(fillrectrequest->color.blue);
  int a = static_cast<int>(fillrectrequest->color.alpha);

  ::fillrect(screen, x, y, w, h, r, g, b, a);

  delete fillrectrequest;
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

  handle_drawing_requests(drawing_requests);
  drawing_requests.clear();

  SDL_Flip(screen);
}

void
DrawingContext::handle_drawing_requests(DrawingRequests& requests)
{
  std::stable_sort(requests.begin(), requests.end());
  
  for(DrawingRequests::iterator i = requests.begin();
      i != requests.end(); ++i) {
    switch(i->type) {
      case SURFACE:
      {
        const Surface* surface = (const Surface*) i->request_data;
        surface->draw(i->pos.x, i->pos.y, i->alpha, i->drawing_effect);
        break;
      }
      case SURFACE_PART:
        draw_surface_part(*i);
        break;
      case GRADIENT:
        draw_gradient(*i);
        break;
      case TEXT:
        draw_text(*i);
        break;
      case FILLRECT:
        draw_filled_rect(*i);
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
}

