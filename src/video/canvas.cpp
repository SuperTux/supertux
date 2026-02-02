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
#include <array>

#include "supertux/globals.hpp"
#include "supertux/gameconfig.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_request.hpp"
#include "video/painter.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

Canvas::Canvas(DrawingContext& context) :
  m_context(context),
  m_requests(),
  m_blur(0)
{
}

Canvas::~Canvas()
{
  clear();
}

void
Canvas::clear()
{
  m_requests.clear();
}

void
Canvas::render(Renderer& renderer, Filter filter)
{
  // On a regular level, each frame has around 50-250 requests (before
  // batching it was 1000-3000), the sort comparator function is
  // called approximatly 3-7 times for each request.
  std::stable_sort(m_requests.begin(), m_requests.end(),
                   [](auto& r1, auto& r2){
                     return r1.first < r2.first;
                   });

  Painter& painter = renderer.get_painter();

  for (const auto& [layer, requests] : m_requests)
  {
    if (filter == BELOW_LIGHTMAP && layer >= LAYER_LIGHTMAP)
      continue;
    else if (filter == ABOVE_LIGHTMAP && layer <= LAYER_LIGHTMAP)
      continue;

    for (const DrawingRequest& req : requests)
    {
      painter.set_clip_rect(req.viewport);

      std::visit([&req, &painter](auto&& arg)
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, TextureRequest>)
          painter.draw_texture(req);
        else if constexpr (std::is_same_v<T, GradientRequest>)
          painter.draw_gradient(req);
        else if constexpr (std::is_same_v<T, FillRectRequest>)
          painter.draw_filled_rect(req);
        else if constexpr (std::is_same_v<T, InverseEllipseRequest>)
          painter.draw_inverse_ellipse(req);
        else if constexpr (std::is_same_v<T, LineRequest>)
          painter.draw_line(req);
        else if constexpr (std::is_same_v<T, TriangleRequest>)
          painter.draw_triangle(req);
        else if constexpr (std::is_same_v<T, GetPixelRequest>)
          painter.get_pixel(req);
      }, req.request);
    }
  }

  painter.clear_clip_rect();
}

void
Canvas::find_or_insert_request(int layer, const DrawingRequest& req)
{
  auto res = std::find_if(m_requests.begin(), m_requests.end(),
                          [layer](auto& r1) {
                            return r1.first == layer;
                          });
  if (res != m_requests.end())
    res->second.emplace_back(std::move(req));
  else
  {
    m_requests.emplace_back(std::make_pair(layer, std::vector<DrawingRequest>{req}));
  }
}

void
Canvas::draw_surface(const SurfacePtr& surface,
                     const Vector& position, float angle, const Color& color, const Blend& blend,
                     int layer)
{
  if (!surface) return;

  const auto& cliprect = m_context.get_cliprect();

  // Discard clipped surface.
  if (position.x > cliprect.get_right() ||
     position.y > cliprect.get_bottom() ||
     position.x + static_cast<float>(surface->get_width()) < cliprect.get_left() ||
     position.y + static_cast<float>(surface->get_height()) < cliprect.get_top())
    return;

  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);
  req.flip = m_context.transform().flip ^ surface->get_flip();
  req.blend = blend;

  req.request = TextureRequest{};
  auto&& req_var = std::get<TextureRequest>(req.request);
  req_var.srcrects.emplace_back(Rectf(surface->get_region()));
  req_var.dstrects.emplace_back(Rectf(apply_translate(position) * scale(),
                                 Sizef(static_cast<float>(surface->get_width()) * scale(),
                                       static_cast<float>(surface->get_height()) * scale())));
  req_var.angles.emplace_back(angle);
  req_var.texture = surface->get_texture().get();
  req_var.displacement_texture = surface->get_displacement_texture().get();
  req_var.color = color;

  find_or_insert_request(layer, req);
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

  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);
  req.flip = m_context.transform().flip ^ surface->get_flip();
  req.alpha = m_context.transform().alpha * style.get_alpha();
  req.blend = style.get_blend();

  req.request = TextureRequest{};
  auto&& req_var = std::get<TextureRequest>(req.request);
  req_var.srcrects.emplace_back(srcrect);
  req_var.dstrects.emplace_back(apply_translate(dstrect.p1())*scale(), dstrect.get_size()*scale());
  req_var.angles.emplace_back(0.0f);
  req_var.texture = surface->get_texture().get();
  req_var.displacement_texture = surface->get_displacement_texture().get();
  req_var.color = style.get_color();

  find_or_insert_request(layer, req);
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
  if (!surface) return;

  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);
  req.flip = m_context.transform().flip ^ surface->get_flip();

  req.request = TextureRequest{};
  auto&& req_var = std::get<TextureRequest>(req.request);
  req_var.color = color;

  req_var.srcrects = std::move(srcrects);
  req_var.dstrects = std::move(dstrects);
  req_var.angles = std::move(angles);

  for (auto& dstrect : req_var.dstrects)
  {
    dstrect = Rectf(apply_translate(dstrect.p1())*scale(), dstrect.get_size()*scale());
  }

  req_var.texture = surface->get_texture().get();
  req_var.displacement_texture = surface->get_displacement_texture().get();

  find_or_insert_request(layer, req);
}

Rectf
Canvas::draw_text(const FontPtr& font, const std::string& text,
                  const Vector& pos, FontAlignment alignment, int layer, const Color& color)
{
  // FIXME: Font viewport.
  return font->draw_text(*this, text, pos, alignment, layer, color);
}

Rectf
Canvas::draw_center_text(const FontPtr& font, const std::string& text,
                         const Vector& position, int layer, const Color& color)
{
  return draw_text(font, text, Vector(position.x + static_cast<float>(m_context.get_width()) / 2.0f, position.y),
                   ALIGN_CENTER, layer, color);
}

void
Canvas::draw_gradient(const Color& top, const Color& bottom, int layer,
                      const GradientDirection& direction, const Rectf& region,
                      const Blend& blend)
{
  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);
  req.blend = blend;

  req.request = GradientRequest{};
  auto&& req_var = std::get<GradientRequest>(req.request);
  req_var.top = top;
  req_var.bottom = bottom;
  req_var.direction = direction;
  req_var.region = Rectf(apply_translate(region.p1())*scale(),
                         apply_translate(region.p2())*scale());

  find_or_insert_request(layer, req);
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
  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);

  req.request = FillRectRequest{};
  auto&& req_var = std::get<FillRectRequest>(req.request);
  req_var.rect = Rectf(apply_translate(rect.p1())*scale(),
                        rect.get_size()*scale());
  req_var.color = color;
  req_var.color.alpha = color.alpha * m_context.transform().alpha;
  req_var.radius = radius;
  req_var.blur = g_config->fancy_gfx ? m_blur : 0;

  find_or_insert_request(layer, req);
}

void
Canvas::draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer)
{
  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);

  req.request = InverseEllipseRequest{};
  auto&& req_var = std::get<InverseEllipseRequest>(req.request);
  req_var.pos          = apply_translate(pos)*scale();
  req_var.color        = color;
  req_var.color.alpha  = color.alpha * m_context.transform().alpha;
  req_var.size         = size*scale();

  find_or_insert_request(layer, req);
}

void
Canvas::draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer)
{
  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);

  req.request = LineRequest{};
  auto&& req_var = std::get<LineRequest>(req.request);
  req_var.pos          = apply_translate(pos1)*scale();
  req_var.color        = color;
  req_var.color.alpha  = color.alpha * m_context.transform().alpha;
  req_var.dest_pos     = apply_translate(pos2)*scale();

  find_or_insert_request(layer, req);
}

void
Canvas::draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer)
{
  auto req = DrawingRequest(m_context.transform());

  req.layer = std::min(layer, m_context.transform().max_layer);

  req.request = TriangleRequest{};
  auto&& req_var = std::get<TriangleRequest>(req.request);
  req_var.pos1 = apply_translate(pos1)*scale();
  req_var.pos2 = apply_translate(pos2)*scale();
  req_var.pos3 = apply_translate(pos3)*scale();
  req_var.color = color;
  req_var.color.alpha = color.alpha * m_context.transform().alpha;

  find_or_insert_request(layer, req);
}

void
Canvas::draw_hexagon(const Vector& pos, float radius, const Color& color,
  int layer)
{
  float radius2 = radius * sqrtf(0.8f);
  float x_off_small = radius * sqrtf(0.2f);
  std::array<Vector, 6> offsets{
    Vector(-x_off_small, -radius2),
    Vector(x_off_small, -radius2),
    Vector(-radius, 0),
    Vector(radius, 0),
    Vector(-x_off_small, radius2),
    Vector(x_off_small, radius2),
  };
  for (size_t i = 0; i < offsets.size() - 2; ++i) {
    draw_triangle(pos + offsets[i], pos + offsets[i + 1], pos + offsets[i + 2],
      color, layer);
  }
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

  auto req = DrawingRequest(m_context.transform());

  req.layer = LAYER_GETPIXEL;
  req.request = GetPixelRequest{};
  auto&& req_var = std::get<GetPixelRequest>(req.request);
  req_var.pos = pos;
  req_var.color_ptr = color_out;

  find_or_insert_request(LAYER_GETPIXEL, req);
}

Vector
Canvas::apply_translate(const Vector& pos) const
{
  const Vector& translation = m_context.transform().translation;
  return (pos - translation) + Vector(static_cast<float>(m_context.get_viewport().left),
                                      static_cast<float>(m_context.get_viewport().top));
}

float
Canvas::scale() const
{
  return m_context.transform().scale;
}
