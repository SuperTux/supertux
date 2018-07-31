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
#include "util/log.hpp"
#include "util/obstackpp.hpp"
#include "video/drawing_request.hpp"
#include "video/lightmap.hpp"
#include "video/painter.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

Canvas::Canvas(DrawingTarget target, DrawingContext& context, obstack& obst) :
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
Canvas::render(VideoSystem& video_system, Filter filter)
{
  // On a regular level, each frame has around 1000-3000 requests, the
  // sort comparator function is called approximatly 7 times for each request.
  std::stable_sort(m_requests.begin(), m_requests.end(),
                   [](const DrawingRequest* r1, const DrawingRequest* r2){
                     return r1->layer < r2->layer;
                   });

  Renderer& renderer = video_system.get_renderer();
  Lightmap& lightmap = video_system.get_lightmap();

  Painter& painter = (m_target == DrawingTarget::LIGHTMAP) ?
    lightmap.get_painter() :
    renderer.get_painter();

  for(const auto& i : m_requests) {
    const DrawingRequest& request = *i;

    if (filter == BELOW_LIGHTMAP && request.layer >= LAYER_LIGHTMAP)
      continue;
    else if (filter == ABOVE_LIGHTMAP && request.layer <= LAYER_LIGHTMAP)
      continue;

    switch(request.type) {
      case SURFACE:
        painter.draw_surface(request);
        break;

      case SURFACE_PART:
        painter.draw_surface_part(request);
        break;

      case GRADIENT:
        painter.draw_gradient(request);
        break;

      case TEXT:
        {
          const auto textrequest = static_cast<TextRequest*>(request.request_data);
          textrequest->font->draw(painter, textrequest->text, request.pos,
                                  textrequest->alignment, request.drawing_effect, request.color, request.alpha);
        }
        break;

      case FILLRECT:
        painter.draw_filled_rect(request);
        break;

      case INVERSEELLIPSE:
        painter.draw_inverse_ellipse(request);
        break;

      case LINE:
        painter.draw_line(request);
        break;

      case TRIANGLE:
        painter.draw_triangle(request);
        break;

      case GETLIGHT:
        // FIXME: turn this into a generic get_pixel that works on Renderer as well
        lightmap.get_light(request);
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

  const auto& cliprect = m_context.get_cliprect();

  // discard clipped surface
  if(position.x > cliprect.get_right() ||
     position.y > cliprect.get_bottom() ||
     position.x + static_cast<float>(surface->get_width()) < cliprect.get_left() ||
     position.y + static_cast<float>(surface->get_height()) < cliprect.get_top())
    return;

  request->type = SURFACE;
  request->pos = apply_translate(position);
  request->layer = layer;
  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;
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

  request->type = SURFACE_PART;
  request->pos = apply_translate(dstrect.p1);
  request->layer = layer;
  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

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

  request->type = TEXT;
  request->pos = apply_translate(position);
  request->layer = layer;
  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;
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
  draw_text(font, text, Vector(position.x + static_cast<float>(m_context.get_width()) / 2.0f, position.y),
            ALIGN_CENTER, layer, color);
}

void
Canvas::draw_gradient(const Color& top, const Color& bottom, int layer,
                      const GradientDirection& direction, const Rectf& region)
{
  auto request = new(m_obst) DrawingRequest();

  request->type = GRADIENT;
  request->pos = Vector(0,0);
  request->layer = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

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

  request->type = FILLRECT;
  request->pos = apply_translate(topleft);
  request->layer = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  auto fillrectrequest = new(m_obst) FillRectRequest();
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * m_context.transform().alpha;
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

  request->type   = FILLRECT;
  request->pos    = apply_translate(rect.p1);
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  auto fillrectrequest = new(m_obst) FillRectRequest;
  fillrectrequest->size = Vector(rect.get_width(), rect.get_height());
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * m_context.transform().alpha;
  fillrectrequest->radius = radius;
  request->request_data = fillrectrequest;

  m_requests.push_back(request);
}

void
Canvas::draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->type   = INVERSEELLIPSE;
  request->pos    = apply_translate(pos);
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  auto ellipse = new(m_obst)InverseEllipseRequest;

  ellipse->color        = color;
  ellipse->color.alpha  = color.alpha * m_context.transform().alpha;
  ellipse->size         = size;
  request->request_data = ellipse;

  m_requests.push_back(request);
}

void
Canvas::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->type   = LINE;
  request->pos    = apply_translate(pos1);
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  auto line = new(m_obst) LineRequest;

  line->color        = color;
  line->color.alpha  = color.alpha * m_context.transform().alpha;
  line->dest_pos     = apply_translate(pos2);
  request->request_data = line;

  m_requests.push_back(request);
}

void
Canvas::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto request = new(m_obst) DrawingRequest();

  request->type   = TRIANGLE;
  request->pos    = apply_translate(pos1);
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  auto triangle = new(m_obst) TriangleRequest;

  triangle->color        = color;
  triangle->color.alpha  = color.alpha * m_context.transform().alpha;
  triangle->pos2         = apply_translate(pos2);
  triangle->pos3         = apply_translate(pos3);
  request->request_data = triangle;

  m_requests.push_back(request);
}

Vector
Canvas::apply_translate(const Vector& pos) const
{
  return m_context.transform().apply(pos) + Vector(static_cast<float>(m_context.get_viewport().left),
                                                   static_cast<float>(m_context.get_viewport().top));
}

/* EOF */
