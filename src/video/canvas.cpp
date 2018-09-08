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

namespace {

DrawingEffect effect_from_surface(const Surface& surface)
{
  if (surface.get_flipx()) {
    return HORIZONTAL_FLIP;
  } else {
    return 0;
  }
}

} // namespace

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
    request->~DrawingRequest();
  }
  m_requests.clear();
}

void
Canvas::render(VideoSystem& video_system, Filter filter)
{
  // On a regular level, each frame has around 50-250 requests (before
  // batching it was 1000-3000), the sort comparator function is
  // called approximatly 3-7 times for each request.
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
      case TEXTURE:
        painter.draw_texture(request);
        break;

      case TEXTURE_BATCH:
        painter.draw_texture_batch(request);
        break;

      case GRADIENT:
        painter.draw_gradient(request);
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

  auto request = new(m_obst) TextureRequest();

  const auto& cliprect = m_context.get_cliprect();

  // discard clipped surface
  if(position.x > cliprect.get_right() ||
     position.y > cliprect.get_bottom() ||
     position.x + static_cast<float>(surface->get_width()) < cliprect.get_left() ||
     position.y + static_cast<float>(surface->get_height()) < cliprect.get_top())
    return;

  request->type = TEXTURE;
  request->layer = layer;
  request->drawing_effect = m_context.transform().drawing_effect ^ effect_from_surface(*surface);
  request->alpha = m_context.transform().alpha;
  request->angle = angle;
  request->blend = blend;

  request->srcrect = Rectf(0, 0, static_cast<float>(surface->get_width()), static_cast<float>(surface->get_height()));
  request->dstrect = Rectf(apply_translate(position), Size(surface->get_width(), surface->get_height()));
  request->texture = surface->get_texture().get();
  request->color = color;

  m_requests.push_back(request);
}

void
Canvas::draw_surface(SurfacePtr surface, const Vector& position, int layer)
{
  draw_surface(surface, position, 0.0f, Color(1.0f, 1.0f, 1.0f), Blend(), layer);
}

void
Canvas::draw_surface_part(SurfacePtr surface, const Rectf& srcrect, const Rectf& dstrect,
                          int layer, const PaintStyle& style)
{
  assert(surface != 0);

  auto request = new(m_obst) TextureRequest();

  request->type = TEXTURE;
  request->layer = layer;
  request->drawing_effect = m_context.transform().drawing_effect ^ effect_from_surface(*surface);
  request->alpha = m_context.transform().alpha * style.get_alpha();
  request->blend = style.get_blend();

  request->srcrect = srcrect;
  request->dstrect = Rectf(apply_translate(dstrect.p1), dstrect.get_size());
  request->texture = surface->get_texture().get();
  request->color = style.get_color();

  m_requests.push_back(request);
}

void
Canvas::draw_surface_batch(SurfacePtr surface,
                           const std::vector<Rectf>& srcrects,
                           const std::vector<Rectf>& dstrects,
                           const Color& color,
                           int layer)
{
  assert(surface != 0);

  auto request = new(m_obst) TextureBatchRequest();

  request->type = TEXTURE_BATCH;
  request->layer = layer;
  request->drawing_effect = m_context.transform().drawing_effect ^ effect_from_surface(*surface);
  request->alpha = m_context.transform().alpha;
  request->color = color;

  request->srcrects = srcrects;
  request->dstrects = dstrects;
  for(auto& dstrect : request->dstrects)
  {
    dstrect = Rectf(apply_translate(dstrect.p1), dstrect.get_size());
  }

  request->texture = surface->get_texture().get();

  m_requests.push_back(request);
}

void
Canvas::draw_text(FontPtr font, const std::string& text,
                  const Vector& pos, FontAlignment alignment, int layer, const Color& color)
{
  if (false)
  {
#if 0
    auto request = new(m_obst) TextRequest();

    request->type = TEXT;
    request->layer = layer;
    request->drawing_effect = m_context.transform().drawing_effect;
    request->alpha = m_context.transform().alpha;

    request->pos = apply_translate(position);
    request->font = font.get();
    request->text = text;
    request->alignment = alignment;
    request->color = color;

    m_requests.push_back(request);
#endif
  }
  else
  {
    font->draw_text(*this, text, pos, alignment, layer, color);
  }
}

void
Canvas::draw_center_text(FontPtr font, const std::string& text,
                         const Vector& position, int layer, const Color& color)
{
  draw_text(font, text, Vector(position.x + static_cast<float>(m_context.get_width()) / 2.0f, position.y),
            ALIGN_CENTER, layer, color);
}

void
Canvas::draw_gradient(const Color& top, const Color& bottom, int layer,
                      const GradientDirection& direction, const Rectf& region)
{
  auto request = new(m_obst) GradientRequest();

  request->type = GRADIENT;
  request->layer = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  request->top = top;
  request->bottom = bottom;
  request->direction = direction;
  request->region = Rectf(apply_translate(region.p1),
                                  apply_translate(region.p2));

  m_requests.push_back(request);
}

void
Canvas::draw_filled_rect(const Vector& topleft, const Vector& size,
                         const Color& color, int layer)
{
  auto request = new(m_obst) FillRectRequest();

  request->type = FILLRECT;
  request->layer = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  request->pos = apply_translate(topleft);
  request->size = size;
  request->color = color;
  request->color.alpha = color.alpha * m_context.transform().alpha;
  request->radius = 0.0f;

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
  auto request = new(m_obst) FillRectRequest;

  request->type   = FILLRECT;
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  request->pos = apply_translate(rect.p1);
  request->size = Vector(rect.get_width(), rect.get_height());
  request->color = color;
  request->color.alpha = color.alpha * m_context.transform().alpha;
  request->radius = radius;

  m_requests.push_back(request);
}

void
Canvas::draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer)
{
  auto request = new(m_obst) InverseEllipseRequest;

  request->type   = INVERSEELLIPSE;
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  request->pos          = apply_translate(pos);
  request->color        = color;
  request->color.alpha  = color.alpha * m_context.transform().alpha;
  request->size         = size;

  m_requests.push_back(request);
}

void
Canvas::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto request = new(m_obst) LineRequest;

  request->type   = LINE;
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  request->pos          = apply_translate(pos1);
  request->color        = color;
  request->color.alpha  = color.alpha * m_context.transform().alpha;
  request->dest_pos     = apply_translate(pos2);

  m_requests.push_back(request);
}

void
Canvas::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto request = new(m_obst) TriangleRequest;

  request->type   = TRIANGLE;
  request->layer  = layer;

  request->drawing_effect = m_context.transform().drawing_effect;
  request->alpha = m_context.transform().alpha;

  request->pos1 = apply_translate(pos1);
  request->pos2 = apply_translate(pos2);
  request->pos3 = apply_translate(pos3);
  request->color = color;
  request->color.alpha = color.alpha * m_context.transform().alpha;

  m_requests.push_back(request);
}

Vector
Canvas::apply_translate(const Vector& pos) const
{
  return m_context.transform().apply(pos) + Vector(static_cast<float>(m_context.get_viewport().left),
                                                   static_cast<float>(m_context.get_viewport().top));
}

/* EOF */
