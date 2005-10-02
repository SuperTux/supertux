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
#include <GL/gl.h>
#include <GL/glu.h>

#include "drawing_context.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "glutil.hpp"
#include "texture.hpp"

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

  lightmap_width = screen->w / LIGHTMAP_DIV;
  lightmap_height = screen->h / LIGHTMAP_DIV;
  int width = next_po2(lightmap_width);
  int height = next_po2(lightmap_height);

  lightmap.reset(new Texture(width, height, GL_RGB));

  lightmap_uv_right = static_cast<float>(lightmap_width) / static_cast<float>(width);
  lightmap_uv_bottom = static_cast<float>(lightmap_height) / static_cast<float>(height);

  requests = &drawing_requests;
}

DrawingContext::~DrawingContext()
{
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
    int layer)
{
  assert(surface != 0);
  
  DrawingRequest request;

  request.type = SURFACE;
  request.pos = transform.apply(position);

  if(request.pos.x >= SCREEN_WIDTH || request.pos.y >= SCREEN_HEIGHT
      || request.pos.x + surface->w < 0 || request.pos.y + surface->h < 0)
    return;

  request.layer = layer;
  request.drawing_effect = transform.drawing_effect;
  request.zoom = transform.zoom;
  request.alpha = transform.alpha;
  request.request_data = const_cast<Surface*> (surface);  

  requests->push_back(request);
}

void
DrawingContext::draw_surface_part(const Surface* surface, const Vector& source,
    const Vector& size, const Vector& dest, int layer)
{
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
  DrawingRequest request;

  request.type = TEXT;
  request.pos = transform.apply(position);
  request.layer = layer;
  request.drawing_effect = transform.drawing_effect;
  request.zoom = transform.zoom;
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
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
      CENTER_ALLIGN, layer);
}

void
DrawingContext::draw_gradient(Color top, Color bottom, int layer)
{
  DrawingRequest request;

  request.type = GRADIENT;
  request.pos = Vector(0,0);
  request.layer = layer;

  request.drawing_effect = transform.drawing_effect;
  request.zoom = transform.zoom;
  request.alpha = transform.alpha;

  GradientRequest* gradientrequest = new GradientRequest;
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  request.request_data = gradientrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
        Color color, int layer)
{
  DrawingRequest request;

  request.type = FILLRECT;
  request.pos = transform.apply(topleft);
  request.layer = layer;

  request.drawing_effect = transform.drawing_effect;
  request.zoom = transform.zoom;
  request.alpha = transform.alpha;                    

  FillRectRequest* fillrectrequest = new FillRectRequest;
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha
      = (int) ((float) fillrectrequest->color.alpha 
              * ((float) transform.alpha / 255.0));
  request.request_data = fillrectrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_surface_part(DrawingRequest& request)
{
  SurfacePartRequest* surfacepartrequest
    = (SurfacePartRequest*) request.request_data;

  surfacepartrequest->surface->impl->draw_part(
      surfacepartrequest->source.x, surfacepartrequest->source.y,
      request.pos.x, request.pos.y,
      surfacepartrequest->size.x, surfacepartrequest->size.y, request.alpha,
      request.drawing_effect);

  delete surfacepartrequest;
}

void
DrawingContext::draw_gradient(DrawingRequest& request)
{
  GradientRequest* gradientrequest = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;
  
  glBegin(GL_QUADS);
  glColor3ub(top.red, top.green, top.blue);
  glVertex2f(0, 0);
  glVertex2f(SCREEN_WIDTH, 0);
  glColor3ub(bottom.red, bottom.green, bottom.blue);
  glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
  glVertex2f(0, SCREEN_HEIGHT);
  glEnd();

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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4ub(fillrectrequest->color.red, fillrectrequest->color.green,
             fillrectrequest->color.blue, fillrectrequest->color.alpha);
  
  glBegin(GL_POLYGON);
  glVertex2f(x, y);
  glVertex2f(x+w, y);
  glVertex2f(x+w, y+h);
  glVertex2f(x, y+h);
  glEnd();
  glDisable(GL_BLEND);

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

  bool use_lightmap = lightmap_requests.size() != 0;
  
  // PART1: create lightmap
  if(use_lightmap) {
    glViewport(0, screen->h - lightmap_height, lightmap_width, lightmap_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();               
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    handle_drawing_requests(lightmap_requests);
    lightmap_requests.clear();
  
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lightmap->handle);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, screen->h - lightmap_height, lightmap_width, lightmap_height);

    glViewport(0, 0, screen->w, screen->h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();               
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);    
    glLoadIdentity();
  }

  //glClear(GL_COLOR_BUFFER_BIT);
  handle_drawing_requests(drawing_requests);
  drawing_requests.clear();

  if(use_lightmap) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    //glDisable(GL_BLEND);
    //glColor4f((float) rand() / (float) RAND_MAX, .22, .88, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);    

    glBindTexture(GL_TEXTURE_2D, lightmap->handle);
    glBegin(GL_QUADS);

    glTexCoord2f(0, lightmap_uv_bottom);
    glVertex2f(0, 0);

    glTexCoord2f(lightmap_uv_right, lightmap_uv_bottom);
    glVertex2f(SCREEN_WIDTH, 0);

    glTexCoord2f(lightmap_uv_right, 0);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);

    glTexCoord2f(0, 0);
    glVertex2f(0, SCREEN_HEIGHT);
    
    glEnd();
  }

  assert_gl("drawing");

  SDL_GL_SwapBuffers();
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

        if(i->zoom != 1.0)
          surface->impl->draw_stretched(i->pos.x * i->zoom, i->pos.y * i->zoom,
                         (int)(surface->w * i->zoom), (int)(surface->h * i->zoom),
                         i->alpha, i->drawing_effect);
        else
          surface->impl->draw(i->pos.x, i->pos.y, i->alpha, i->drawing_effect);
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
DrawingContext::set_drawing_effect(uint32_t effect)
{
  transform.drawing_effect = effect;
}

uint32_t
DrawingContext::get_drawing_effect() const
{
  return transform.drawing_effect;
}

void
DrawingContext::set_zooming(float zoom)
{
  transform.zoom = zoom;
}

void
DrawingContext::set_alpha(uint8_t alpha)
{
  transform.alpha = alpha;
}

uint8_t
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
  if(target == LIGHTMAP)
    requests = &lightmap_requests;
  else
    requests = &drawing_requests;
}

