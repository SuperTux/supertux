//  $Id$
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

#include "drawing_context.h"
#include "surface.h"
#include "app/globals.h"
#include "font.h"

using namespace SuperTux;

DrawingContext::DrawingContext()
{
}

DrawingContext::~DrawingContext()
{
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
    int layer, uint32_t drawing_effect)
{
  assert(surface != 0);
  
  DrawingRequest request;

  request.type = SURFACE;
  request.pos = transform.apply(position);

  if(request.pos.x >= SCREEN_WIDTH || request.pos.y >= SCREEN_HEIGHT
      || request.pos.x + surface->w < 0 || request.pos.y + surface->h < 0)
    return;

  request.layer = layer;
  request.drawing_effect = transform.drawing_effect | drawing_effect;
  request.zoom = transform.zoom;
  request.alpha = transform.alpha;
  request.request_data = const_cast<Surface*> (surface);  

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_surface_part(const Surface* surface, const Vector& source,
    const Vector& size, const Vector& dest, int layer, uint32_t drawing_effect)
{
  assert(surface != 0);

  DrawingRequest request;

  request.type = SURFACE_PART;
  request.pos = transform.apply(dest);
  request.layer = layer;
  request.drawing_effect = transform.drawing_effect | drawing_effect;
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

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_text(const Font* font, const std::string& text,
    const Vector& position, FontAlignment alignment, int layer,
    uint32_t drawing_effect)
{
  DrawingRequest request;

  request.type = TEXT;
  request.pos = transform.apply(position);
  request.layer = layer;
  request.drawing_effect = transform.drawing_effect | drawing_effect;
  request.zoom = transform.zoom;
  request.alpha = transform.alpha;

  TextRequest* textrequest = new TextRequest;
  textrequest->font = font;
  textrequest->text = text;
  textrequest->alignment = alignment;
  request.request_data = textrequest;

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_center_text(const Font* font, const std::string& text,
    const Vector& position, int layer, uint32_t drawing_effect)
{
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
      CENTER_ALLIGN, layer, drawing_effect);
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

  drawingrequests.push_back(request);
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
  request.request_data = fillrectrequest;

  drawingrequests.push_back(request);
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
  
#ifndef NOOPENGL
  if(use_gl)
    {
      glBegin(GL_QUADS);
      glColor3ub(top.red, top.green, top.blue);
      glVertex2f(0, 0);
      glVertex2f(SCREEN_WIDTH, 0);
      glColor3ub(bottom.red, bottom.green, bottom.blue);
      glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
      glVertex2f(0, SCREEN_HEIGHT);
      glEnd();
    }
  else
  {
#endif
    if(&top == &bottom)
      {
      fillrect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, top.red, top.green, top.blue);
      }
    else
      {
      float redstep = (float(bottom.red)-float(top.red)) / float(SCREEN_HEIGHT);
      float greenstep = (float(bottom.green)-float(top.green)) / float(SCREEN_HEIGHT);
      float bluestep = (float(bottom.blue) - float(top.blue)) / float(SCREEN_HEIGHT);

      for(float y = 0; y < SCREEN_HEIGHT; y += 2)
        fillrect(0, (int)y, SCREEN_WIDTH, 2,
            int(float(top.red) + redstep * y),
            int(float(top.green) + greenstep * y),
            int(float(top.blue) + bluestep * y), 255);
      }
#ifndef NOOPENGL

    }
#endif

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

#ifndef NOOPENGL
  if(use_gl)
    {
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
    }
  else
    {
#endif
      SDL_Rect src, rect;
      SDL_Surface *temp = NULL;
                                                                                
      rect.x = (int)x;
      rect.y = (int)y;
      rect.w = (int)w;
      rect.h = (int)h;
                                                                                
      if(fillrectrequest->color.alpha != 255)
        {
          temp = SDL_CreateRGBSurface(screen->flags, rect.w, rect.h, screen->format->BitsPerPixel,
                                      screen->format->Rmask,
                                      screen->format->Gmask,
                                      screen->format->Bmask,
                                      screen->format->Amask);
                                                                                
                                                                                
          src.x = 0;
          src.y = 0;
          src.w = rect.w;
          src.h = rect.h;
                                                                                
          SDL_FillRect(temp, &src, SDL_MapRGB(screen->format, 
                fillrectrequest->color.red, fillrectrequest->color.green,
                fillrectrequest->color.blue));
                                                                                
          SDL_SetAlpha(temp, SDL_SRCALPHA, fillrectrequest->color.alpha);
                                                                                
          SDL_BlitSurface(temp,0,screen,&rect);
                                                                                
          SDL_FreeSurface(temp);
        }
      else
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 
              fillrectrequest->color.red, fillrectrequest->color.green,
              fillrectrequest->color.blue));
                                                                                
#ifndef NOOPENGL
                                                                                
    }
#endif

  delete fillrectrequest;
}

void
DrawingContext::do_drawing()
{
#ifdef DEBUG
  assert(transformstack.empty());
#endif
  transformstack.clear();
    
  std::stable_sort(drawingrequests.begin(), drawingrequests.end());

  for(DrawingRequests::iterator i = drawingrequests.begin();
      i != drawingrequests.end(); ++i) {
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

  // update screen
  if(use_gl)
    SDL_GL_SwapBuffers();
  else
    SDL_Flip(screen);

  drawingrequests.clear();
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
DrawingContext::set_drawing_effect(int effect)
{
  transform.drawing_effect = effect;
}

void
DrawingContext::set_zooming(float zoom)
{
  transform.zoom = zoom;
}

void
DrawingContext::set_alpha(int alpha)
{
  transform.alpha = alpha;
}
