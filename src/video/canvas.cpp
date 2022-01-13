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
#include "video/painter.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

Canvas::Canvas(DrawingContext& context, obstack& obst) :
  m_context(context),
  m_obst(obst),
  m_requests()
{
  m_requests.reserve(500);
}

Canvas::~Canvas()
{
  clear();
}

void
Canvas::clear()
{
  for (auto& request : m_requests)
  {
    request->~DrawingRequest();
  }
  m_requests.clear();
}

void
Canvas::render(Renderer& renderer, Filter filter)
{
  // On a regular level, each frame has around 50-250 requests (before
  // batching it was 1000-3000), the sort comparator function is
  // called approximatly 3-7 times for each request.
  std::stable_sort(m_requests.begin(), m_requests.end(),
                   [](const DrawingRequest* r1, const DrawingRequest* r2){
                     return r1->layer < r2->layer;
                   });

  Painter& painter = renderer.get_painter();

  for (const auto& i : m_requests) {
    const DrawingRequest& request = *i;

    if (filter == BELOW_LIGHTMAP && request.layer >= LAYER_LIGHTMAP)
      continue;
    else if (filter == ABOVE_LIGHTMAP && request.layer <= LAYER_LIGHTMAP)
      continue;

    switch (request.type) {
      case TEXTURE:
        painter.draw_texture(static_cast<const TextureRequest&>(request));
        break;

      case GRADIENT:
        painter.draw_gradient(static_cast<const GradientRequest&>(request));
        break;

      case FILLRECT:
        painter.draw_filled_rect(static_cast<const FillRectRequest&>(request));
        break;

      case INVERSEELLIPSE:
        painter.draw_inverse_ellipse(static_cast<const InverseEllipseRequest&>(request));
        break;

      case LINE:
        painter.draw_line(static_cast<const LineRequest&>(request));
        break;

      case TRIANGLE:
        painter.draw_triangle(static_cast<const TriangleRequest&>(request));
        break;

      case GETPIXEL:
        painter.get_pixel(static_cast<const GetPixelRequest&>(request));
        break;
    }
  }
}

void
Canvas::draw_surface(const SurfacePtr& surface,
                     const Vector& position, float angle, const Color& color, const Blend& blend,
                     int layer)
{
  if (!surface) return;

  const auto& cliprect = m_context.get_cliprect();

  // discard clipped surface
  if (position.x > cliprect.get_right() ||
     position.y > cliprect.get_bottom() ||
     position.x + static_cast<float>(surface->get_width()) < cliprect.get_left() ||
     position.y + static_cast<float>(surface->get_height()) < cliprect.get_top())
    return;

  auto request = new(m_obst) TextureRequest();

  request->type = TEXTURE;
  request->layer = layer;
  request->flip = m_context.transform().flip ^ surface->get_flip();
  request->alpha = m_context.transform().alpha;
  request->blend = blend;

  request->srcrects.emplace_back(Rectf(surface->get_region()));
  request->dstrects.emplace_back(Rectf(apply_translate(position) * scale(),
                                 Sizef(static_cast<float>(surface->get_width()) * scale(),
                                       static_cast<float>(surface->get_height()) * scale())));
  request->angles.emplace_back(angle);
  request->repeats.emplace_back(Size(1, 1));
  request->texture = surface->get_texture().get();
  request->displacement_texture = surface->get_displacement_texture().get();
  request->color = color;

  m_requests.push_back(request);
}

void
Canvas::draw_surface(const SurfacePtr& surface, const Vector& position, int layer)
{
  draw_surface(surface, position, 0.0f, Color(1.0f, 1.0f, 1.0f), Blend(), layer);
}

void
Canvas::draw_surface_scaled(const SurfacePtr& surface, const Rectf& dstrect,
                            int layer, const PaintStyle& style)
{
  draw_surface_part(surface, Rectf(0.0f, 0.0f, static_cast<float>(surface->get_width()), static_cast<float>(surface->get_height())),
                    dstrect, layer, style);
}

void
Canvas::draw_surface_part(const SurfacePtr& surface, const Rectf& srcrect, const Rectf& dstrect,
                          int layer, const PaintStyle& style)
{
  if (!surface) return;

  auto request = new(m_obst) TextureRequest();

  request->type = TEXTURE;
  request->layer = layer;
  request->flip = m_context.transform().flip ^ surface->get_flip();
  request->alpha = m_context.transform().alpha * style.get_alpha();
  request->blend = style.get_blend();

  request->srcrects.emplace_back(srcrect);
  request->dstrects.emplace_back(apply_translate(dstrect.p1())*scale(), dstrect.get_size()*scale());
  request->angles.emplace_back(0.0f);
  request->repeats.emplace_back(Size(1, 1));
  request->texture = surface->get_texture().get();
  request->displacement_texture = surface->get_displacement_texture().get();
  request->color = style.get_color();

  m_requests.push_back(request);
}

void
Canvas::draw_surface_batch(const SurfacePtr& surface,
                           std::vector<Rectf> srcrects,
                           std::vector<Rectf> dstrects,
                           const Color& color,
                           int layer)
{
  const size_t len = srcrects.size();
  draw_surface_batch(surface,
                     std::move(srcrects),
                     std::move(dstrects),
                     std::vector<float>(len, 0.0f),
                     color, layer);
}

void
Canvas::draw_surface_batch(const SurfacePtr& surface,
                           std::vector<Rectf> srcrects,
                           std::vector<Rectf> dstrects,
                           std::vector<float> angles,
                           const Color& color,
                           int layer)
{
  const size_t len = srcrects.size();
  draw_surface_batch(surface,
                     std::move(srcrects),
                     std::move(dstrects),
                     std::move(angles),
                     std::vector<Size>(len, Size(1, 1)),
                     color, layer);
}

void
Canvas::draw_surface_batch(const SurfacePtr& surface,
                           std::vector<Rectf> srcrects,
                           std::vector<Rectf> dstrects,
                           std::vector<Size> repeats,
                           const Color& color,
                           int layer)
{
  const size_t len = srcrects.size();
  draw_surface_batch(surface,
                     std::move(srcrects),
                     std::move(dstrects),
                     std::vector<float>(len, 0.0f),
                     std::move(repeats),
                     color, layer);
}

void
Canvas::draw_surface_batch(const SurfacePtr& surface,
                           std::vector<Rectf> srcrects,
                           std::vector<Rectf> dstrects,
                           std::vector<float> angles,
                           std::vector<Size> repeats,
                           const Color& color,
                           int layer)
{
  if (!surface) return;

  auto request = new(m_obst) TextureRequest();

  request->type = TEXTURE;
  request->layer = layer;
  request->flip = m_context.transform().flip ^ surface->get_flip();
  request->alpha = m_context.transform().alpha;
  request->color = color;

  request->srcrects = std::move(srcrects);
  request->dstrects = std::move(dstrects);
  request->angles = std::move(angles);
  request->repeats = std::move(repeats);

  for (auto& dstrect : request->dstrects)
  {
    dstrect = Rectf(apply_translate(dstrect.p1())*scale(), dstrect.get_size()*scale());
  }

  request->texture = surface->get_texture().get();
  request->displacement_texture = surface->get_displacement_texture().get();

  m_requests.push_back(request);
}

void
Canvas::draw_text(const FontPtr& font, const std::string& text,
                  const Vector& pos, FontAlignment alignment, int layer, const Color& color)
{
  font->draw_text(*this, text, pos, alignment, layer, color);
}

void
Canvas::draw_center_text(const FontPtr& font, const std::string& text,
                         const Vector& position, int layer, const Color& color)
{
  draw_text(font, text, Vector(position.x + static_cast<float>(m_context.get_width()) / 2.0f, position.y),
            ALIGN_CENTER, layer, color);
}

void
Canvas::draw_gradient(const Color& top, const Color& bottom, int layer,
                      const GradientDirection& direction, const Rectf& region,
                      const Blend& blend)
{
  auto request = new(m_obst) GradientRequest();

  request->type = GRADIENT;
  request->layer = layer;

  request->flip = m_context.transform().flip;
  request->alpha = m_context.transform().alpha;
  request->blend = blend;

  request->top = top;
  request->bottom = bottom;
  request->direction = direction;
  request->region = Rectf(apply_translate(region.p1())*scale(),
                          apply_translate(region.p2())*scale());

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

  request->flip = m_context.transform().flip;
  request->alpha = m_context.transform().alpha;

  request->rect = Rectf(apply_translate(rect.p1())*scale(),
                        rect.get_size()*scale());
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

  request->flip = m_context.transform().flip;
  request->alpha = m_context.transform().alpha;

  request->pos          = apply_translate(pos)*scale();
  request->color        = color;
  request->color.alpha  = color.alpha * m_context.transform().alpha;
  request->size         = size*scale();

  m_requests.push_back(request);
}

void
Canvas::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto request = new(m_obst) LineRequest;

  request->type   = LINE;
  request->layer  = layer;

  request->flip = m_context.transform().flip;
  request->alpha = m_context.transform().alpha;

  request->pos          = apply_translate(pos1)*scale();
  request->color        = color;
  request->color.alpha  = color.alpha * m_context.transform().alpha;
  request->dest_pos     = apply_translate(pos2)*scale();

  m_requests.push_back(request);
}

void
Canvas::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto request = new(m_obst) TriangleRequest;

  request->type   = TRIANGLE;
  request->layer  = layer;

  request->flip = m_context.transform().flip;
  request->alpha = m_context.transform().alpha;

  request->pos1 = apply_translate(pos1)*scale();
  request->pos2 = apply_translate(pos2)*scale();
  request->pos3 = apply_translate(pos3)*scale();
  request->color = color;
  request->color.alpha = color.alpha * m_context.transform().alpha;

  m_requests.push_back(request);
}

void
Canvas::get_pixel(const Vector& position, const std::shared_ptr<Color>& color_out)
{
  assert(color_out);

  Vector pos = apply_translate(position)*scale();

  // There is no light offscreen.
  if (pos.x >= static_cast<float>(m_context.get_viewport().get_width()) ||
      pos.y >= static_cast<float>(m_context.get_viewport().get_height()) ||
      pos.x < 0.0f ||
      pos.y < 0.0f)
  {
    *color_out = Color(0, 0, 0);
    return;
  }

  auto request = new(m_obst) GetPixelRequest();

  request->layer = LAYER_GETPIXEL;
  request->pos = pos;
  request->color_ptr = color_out;

  m_requests.push_back(request);
}

Vector
Canvas::apply_translate(const Vector& pos) const
{
  Vector translation = m_context.transform().translation;
  return (pos - translation) + Vector(static_cast<float>(m_context.get_viewport().left),
                                      static_cast<float>(m_context.get_viewport().top));
}

float
Canvas::scale() const
{
  return m_context.transform().scale;
}

/* EOF */
