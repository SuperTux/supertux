#include <assert.h>
#include "drawing_context.h"

#include <iostream>
#include <algorithm>
#include "texture.h"
#include "globals.h"
#include "font.h"

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
  request.layer = layer;
  request.request_data = const_cast<Surface*> (surface);
  request.pos = transform.apply(position);
  request.drawing_effect = drawing_effect;

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_surface_part(const Surface* surface, const Vector& source,
    const Vector& size, const Vector& dest, int layer, uint32_t drawing_effect)
{
  assert(surface != 0);

  DrawingRequest request;

  request.type = SURFACE_PART;
  request.layer = layer;
  request.pos = transform.apply(dest);
  request.drawing_effect = drawing_effect;
  
  SurfacePartRequest* surfacepartrequest = new SurfacePartRequest();
  surfacepartrequest->size = size;
  surfacepartrequest->source = source;
  surfacepartrequest->surface = surface;
  request.request_data = surfacepartrequest;

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_text(Font* font, const std::string& text,
    const Vector& position, int layer)
{
  DrawingRequest request;

  request.type = TEXT;
  request.layer = layer;
  request.pos = transform.apply(position);

  TextRequest* textrequest = new TextRequest;
  textrequest->font = font;
  textrequest->text = text;
  request.request_data = textrequest;

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_text_center(Font* font, const std::string& text,
    const Vector& position, int layer)
{
  DrawingRequest request;

  request.type = TEXT;
  request.layer = layer;
  request.pos = transform.apply(position) + Vector(screen->w/2 - 
      font->get_text_width(text)/2, 0);

  TextRequest* textrequest = new TextRequest;
  textrequest->font = font;
  textrequest->text = text;
  request.request_data = textrequest;

  drawingrequests.push_back(request);
}

void
DrawingContext::draw_gradient(Color top, Color bottom, int layer)
{
  DrawingRequest request;

  request.type = GRADIENT;
  request.layer = layer;
  request.pos = Vector(0,0);

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
  request.layer = layer;
  request.pos = topleft;

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
      surfacepartrequest->size.x, surfacepartrequest->size.y, 255,
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
      glVertex2f(screen->w, 0);
      glColor3ub(bottom.red, bottom.green, bottom.blue);
      glVertex2f(screen->w, screen->h);
      glVertex2f(0, screen->h);
      glEnd();
    }
  else
  {
#endif
    float redstep = (float(bottom.red)-float(top.red)) / float(screen->h);
    float greenstep = (float(bottom.green)-float(top.green)) / float(screen->h);
    float bluestep = (float(bottom.blue) - float(top.blue)) / float(screen->h);

    for(float y = 0; y < screen->h; y += 2)
      fillrect(0, (int)y, screen->w, 2,
          int(float(top.red) + redstep * y),
          int(float(top.green) + greenstep * y),
          int(float(top.blue) + bluestep * y), 255);
#ifndef NOOPENGL

    }
#endif

  delete gradientrequest;
}

void
DrawingContext::draw_text(DrawingRequest& request)
{
  TextRequest* textrequest = (TextRequest*) request.request_data;
  
  textrequest->font->draw(textrequest->text, request.pos);

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
  std::stable_sort(drawingrequests.begin(), drawingrequests.end());

  for(DrawingRequests::iterator i = drawingrequests.begin();
      i != drawingrequests.end(); ++i) {
    switch(i->type) {
      case SURFACE:
      {
        const Surface* surface = (const Surface*) i->request_data;
        surface->impl->draw(i->pos.x, i->pos.y, 255, i->drawing_effect);
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

