//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/canvas.hpp"

#include <algorithm>

#include "supertux/globals.hpp"
#include "util/obstackpp.hpp"
#include "video/drawing_request.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

Canvas::Canvas(Target target, DrawingContext& context, obstack& obst) :
  m_target(target),
  m_context(context),
  m_obst(obst),
  m_requests()
{
}

Canvas::~Canvas()
{
  clear();
}

void
Canvas::clear()
{
  for(auto& request : m_requests)
  {
    if (request->request_data)
    {
      request->request_data->~DrawingRequestData();
    }
    request->~DrawingRequest();
  }
  m_requests.clear();
}

void
Canvas::render(VideoSystem& video_system)
{
  auto& requests_ = m_requests; // FIXME: grumbel 22. Jul 2018

  // On a regular level, each frame has around 1000-3000 requests
  std::stable_sort(requests_.begin(), requests_.end(),
                   [](const DrawingRequest* r1, const DrawingRequest* r2){
                     return r1->layer < r2->layer;
                   });

  Renderer& renderer = video_system.get_renderer();
  Lightmap& lightmap = video_system.get_lightmap();

  for(auto i = requests_.begin(); i != requests_.end(); ++i) {
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
Canvas::draw_surface(SurfacePtr surface, const Vector& position,
                             float angle, const Color& color, const Blend& blend,
                             int layer)
{
  assert(surface != 0);

  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = SURFACE;
  request->pos = m_context.get_transform().apply(position);

  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
     || request->pos.x + surface->get_width() < 0
     || request->pos.y + surface->get_height() < 0)
    return;

  request->layer = layer;
  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;
  request->angle = angle;
  request->color = color;
  request->blend = blend;

  auto surfacerequest = new(m_obst) SurfaceRequest();
  surfacerequest->surface = surface.get();
  request->request_data = surfacerequest;

  m_requests.push_back(request);
}

void
Canvas::draw_surface(SurfacePtr surface, const Vector& position, int layer)
{
  draw_surface(surface, position, 0.0f, Color(1.0f, 1.0f, 1.0f), Blend(), layer);
}

void
Canvas::draw_surface_part(SurfacePtr surface,
                          const Rectf& srcrect, const Rectf& dstrect,
                          int layer)
{
  assert(surface != 0);

  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = SURFACE_PART;
  request->pos = m_context.get_transform().apply(dstrect.p1);
  request->layer = layer;
  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto surfacepartrequest = new(m_obst) SurfacePartRequest();
  surfacepartrequest->srcrect = srcrect;
  surfacepartrequest->dstsize = dstrect.get_size();
  surfacepartrequest->surface = surface.get();

  request->request_data = surfacepartrequest;

  m_requests.push_back(request);
}

void
Canvas::draw_text(FontPtr font, const std::string& text,
                          const Vector& position, FontAlignment alignment, int layer, Color color)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = TEXT;
  request->pos = m_context.get_transform().apply(position);
  request->layer = layer;
  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;
  request->color = color;

  auto textrequest = new(m_obst) TextRequest();
  textrequest->font = font.get();
  textrequest->text = text;
  textrequest->alignment = alignment;
  request->request_data = textrequest;

  m_requests.push_back(request);
}

void
Canvas::draw_center_text(FontPtr font, const std::string& text,
                                 const Vector& position, int layer, Color color)
{
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
            ALIGN_CENTER, layer, color);
}

void
Canvas::draw_gradient(const Color& top, const Color& bottom, int layer,
                              const GradientDirection& direction, const Rectf& region)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = GRADIENT;
  request->pos = Vector(0,0);
  request->layer = layer;

  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto gradientrequest = new(m_obst) GradientRequest();
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  gradientrequest->direction = direction;
  gradientrequest->region = region;
  request->request_data = gradientrequest;

  m_requests.push_back(request);
}

void
Canvas::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type = FILLRECT;
  request->pos = m_context.get_transform().apply(topleft);
  request->layer = layer;

  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto fillrectrequest = new(m_obst) FillRectRequest();
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * m_context.get_transform().alpha;
  fillrectrequest->radius = 0.0f;
  request->request_data = fillrectrequest;

  m_requests.push_back(request);
}

void
Canvas::draw_filled_rect(const Rectf& rect, const Color& color,
                                 int layer)
{
  draw_filled_rect(rect, color, 0.0f, layer);
}

void
Canvas::draw_filled_rect(const Rectf& rect, const Color& color, float radius, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = FILLRECT;
  request->pos    = m_context.get_transform().apply(rect.p1);
  request->layer  = layer;

  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto fillrectrequest = new(m_obst) FillRectRequest;
  fillrectrequest->size = Vector(rect.get_width(), rect.get_height());
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * m_context.get_transform().alpha;
  fillrectrequest->radius = radius;
  request->request_data = fillrectrequest;

  m_requests.push_back(request);
}

void
Canvas::draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = INVERSEELLIPSE;
  request->pos    = m_context.get_transform().apply(pos);
  request->layer  = layer;

  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto ellipse = new(m_obst)InverseEllipseRequest;

  ellipse->color        = color;
  ellipse->color.alpha  = color.alpha * m_context.get_transform().alpha;
  ellipse->size         = size;
  request->request_data = ellipse;

  m_requests.push_back(request);
}

void
Canvas::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = LINE;
  request->pos    = m_context.get_transform().apply(pos1);
  request->layer  = layer;

  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto line = new(m_obst) LineRequest;

  line->color        = color;
  line->color.alpha  = color.alpha * m_context.get_transform().alpha;
  line->dest_pos     = m_context.get_transform().apply(pos2);
  request->request_data = line;

  m_requests.push_back(request);
}

void
Canvas::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->target = m_target;
  request->type   = TRIANGLE;
  request->pos    = m_context.get_transform().apply(pos1);
  request->layer  = layer;

  request->drawing_effect = m_context.get_transform().drawing_effect;
  request->alpha = m_context.get_transform().alpha;

  auto triangle = new(m_obst) TriangleRequest;

  triangle->color        = color;
  triangle->color.alpha  = color.alpha * m_context.get_transform().alpha;
  triangle->pos2         = m_context.get_transform().apply(pos2);
  triangle->pos3         = m_context.get_transform().apply(pos3);
  request->request_data = triangle;

  m_requests.push_back(request);
}

/* EOF */
