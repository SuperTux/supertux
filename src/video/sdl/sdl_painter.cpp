//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/sdl/sdl_painter.hpp"

#include <SDL.h>
#include <algorithm>
#include <array>
#include <assert.h>
#include <math.h>

#include "supertux/globals.hpp"
#include "math/util.hpp"
#include "util/log.hpp"
#include "video/drawing_request.hpp"
#include "video/renderer.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/sdl/sdl_video_system.hpp"
#include "video/viewport.hpp"

namespace {

SDL_Rect to_sdl_rect(const Rectf& rect)
{
  SDL_Rect sdl_rect;

  // floorf() here due to int(-0.5) and int(0.5) both rounding to 0,
  // thus creating a jump in coordinates at 0
  sdl_rect.x = static_cast<int>(floorf(rect.get_left()));
  sdl_rect.y = static_cast<int>(floorf(rect.get_top()));

  // roundf() here due to int(rect.get_right()y - rect.get_left()y) being
  // off-by-one due to float errors
  sdl_rect.w = static_cast<int>(roundf(rect.get_width()));
  sdl_rect.h = static_cast<int>(roundf(rect.get_height()));

  return sdl_rect;
}

SDL_BlendMode blend2sdl(const Blend& blend)
{
  if (blend == Blend::NONE)
  {
    return SDL_BLENDMODE_NONE;
  }
  else if (blend == Blend::BLEND)
  {
    return SDL_BLENDMODE_BLEND;
  }
  else if (blend == Blend::ADD)
  {
    return SDL_BLENDMODE_ADD;
  }
  else if (blend == Blend::MOD)
  {
    return SDL_BLENDMODE_MOD;
  }
  else
  {
    log_warning << "unknown blend mode combinations: blend=" << static_cast<int>(blend) << std::endl;
    return SDL_BLENDMODE_BLEND;
  }
}

/* Creates a new rectangle covering the area where srcrect and imgrect
   overlap, in addition create four more rectangles for the areas
   where srcrect is outside of imgrect, some of those rects will be
   empty. The rectangles will be returned in the order inside, top,
   left, right, bottom */
std::tuple<Rect, Rect, Rect, Rect, Rect>
intersect(const Rect& srcrect, const Rect& imgrect)
{
  return std::make_tuple(
    // inside
    Rect(std::max(srcrect.left, imgrect.left), std::max(srcrect.top, imgrect.top),
         std::min(srcrect.right, imgrect.right), std::min(srcrect.bottom, imgrect.bottom)),

    // top
    Rect(srcrect.left, srcrect.top,
         srcrect.right, imgrect.top),

    // left
    Rect(srcrect.left, std::max(srcrect.top, imgrect.top),
         imgrect.left, std::min(srcrect.bottom, imgrect.bottom)),

    // right
    Rect(imgrect.right, std::max(srcrect.top, imgrect.top),
         srcrect.right, std::min(srcrect.bottom, imgrect.bottom)),

    // bottom
    Rect(srcrect.left, imgrect.bottom,
         srcrect.right, srcrect.bottom)
    );
}

/* Map the area covered by inside in srcrect to dstrect */
Rect relative_map(const Rect& inside, const Rect& srcrect, const Rect& dstrect)
{
  assert(srcrect.contains(inside));

  Rect result(dstrect.left + (inside.left - srcrect.left) * dstrect.get_width() / srcrect.get_width(),
              dstrect.top + (inside.top - srcrect.top) * dstrect.get_height() / srcrect.get_height(),
              dstrect.left + (inside.right - srcrect.left) * dstrect.get_width() / srcrect.get_width(),
              dstrect.top + (inside.bottom - srcrect.top) * dstrect.get_height() / srcrect.get_height());

  assert(dstrect.contains(result));

  return result;
}

void render_texture(SDL_Renderer* renderer,
                    SDL_Texture* texture, const Rect& imgrect,
                    const Rect& srcrect, const Rect& dstrect)
{
  assert(imgrect.contains(srcrect.left, srcrect.top));

  if (srcrect.empty() || dstrect.empty())
    return;

  if (imgrect.contains(srcrect))
  {
    SDL_Rect sdl_srcrect = srcrect.to_sdl();
    SDL_Rect sdl_dstrect = dstrect.to_sdl();
    SDL_RenderCopy(renderer, texture, &sdl_srcrect, &sdl_dstrect);
  }
  else
  {
    Rect inside;
    std::array<Rect, 4> rest;
    std::tie(inside, rest[0], rest[1], rest[2], rest[3]) = intersect(srcrect, imgrect);

    render_texture(renderer, texture, imgrect, inside, relative_map(inside, srcrect, dstrect));

    for (const Rect& rect : rest)
    {
      const Rect new_srcrect(math::positive_mod(rect.left, imgrect.get_width()),
                             math::positive_mod(rect.top, imgrect.get_height()),
                             rect.get_size());
      render_texture(renderer, texture, imgrect,
                     new_srcrect, relative_map(rect, srcrect, dstrect));
    }
  }
}

/* A version SDL_RenderCopyEx that supports texture animation as specified by Sampler */
void RenderCopyEx(SDL_Renderer*          renderer,
                  SDL_Texture*           texture,
                  const SDL_Rect*        sdl_srcrect,
                  const SDL_Rect*        sdl_dstrect,
                  const double           angle,
                  const SDL_Point*       center,
                  const SDL_RendererFlip flip,
                  const Sampler& sampler)
{
  Vector animate = sampler.get_animate();
  if (animate.x == 0.0f && animate.y == 0.0f)
  {
    SDL_RenderCopyEx(renderer, texture, sdl_srcrect, sdl_dstrect, angle, nullptr, flip);
  }
  else
  {
    // This part deals with texture animation. Texture animation is
    // accomplished by shifting the srcrect across the input texture.
    // If the srcrect goes out of bounds of the texture, it is broken
    // up into multiple rectangles that wrap around and fall back into
    // the texture space.
    //
    // If a srcrect is passed to SDL that goes out of bounds SDL will
    // clip it to be inside the bounds, without adjusting dstrect,
    // thus result in stretching artifacts.
    //
    // FIXME: Neither flipping nor wrap modes are supported at the
    // moment. wrap is treated as if it was set to 'repeat'.
    int width;
    int height;

    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    animate *= g_game_time;

    int tex_off_x = math::positive_mod(static_cast<int>(animate.x), width);
    int tex_off_y = math::positive_mod(static_cast<int>(animate.y), height);

    if ((tex_off_x == 0 && tex_off_y == 0) ||
        flip ||
        angle != 0.0)
    {
      SDL_RenderCopyEx(renderer, texture, sdl_srcrect, sdl_dstrect, angle, nullptr, flip);
    }
    else
    {
      Rect imgrect(0, 0, Size(width, height));
      Rect srcrect(math::positive_mod(sdl_srcrect->x + tex_off_x, width),
                   math::positive_mod(sdl_srcrect->y + tex_off_y, height),
                   Size(sdl_srcrect->w, sdl_srcrect->h));

      render_texture(renderer, texture, imgrect, srcrect, Rect(*sdl_dstrect));
    }
  }
}

} // namespace

SDLPainter::SDLPainter(SDLVideoSystem& video_system, Renderer& renderer, SDL_Renderer* sdl_renderer) :
  m_video_system(video_system),
  m_renderer(renderer),
  m_sdl_renderer(sdl_renderer),
  m_cliprect()
{}

void
SDLPainter::draw_texture(const TextureRequest& request)
{
  const auto& texture = static_cast<const SDLTexture&>(*request.texture);

  assert(request.srcrects.size() == request.dstrects.size());
  assert(request.srcrects.size() == request.angles.size());

  for (size_t i = 0; i < request.srcrects.size(); ++i)
  {
    const SDL_Rect& src_rect = to_sdl_rect(request.srcrects[i]);
    const SDL_Rect& dst_rect = to_sdl_rect(request.dstrects[i]);

    Uint8 r = static_cast<Uint8>(request.color.red * 255);
    Uint8 g = static_cast<Uint8>(request.color.green * 255);
    Uint8 b = static_cast<Uint8>(request.color.blue * 255);
    Uint8 a = static_cast<Uint8>(request.color.alpha * request.alpha * 255);

    SDL_SetTextureColorMod(texture.get_texture(), r, g, b);
    SDL_SetTextureAlphaMod(texture.get_texture(), a);
    SDL_SetTextureBlendMode(texture.get_texture(), blend2sdl(request.blend));

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if ((request.flip & HORIZONTAL_FLIP) != 0)
    {
      flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    }

    if ((request.flip & VERTICAL_FLIP) != 0)
    {
      flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
    }

    RenderCopyEx(m_sdl_renderer, texture.get_texture(),
                 &src_rect, &dst_rect,
                 static_cast<double>(request.angles[i]), nullptr, flip,
                 texture.get_sampler());
  }
}

void
SDLPainter::draw_gradient(const GradientRequest& request)
{
  const Color& top = request.top;
  const Color& bottom = request.bottom;
  const GradientDirection& direction = request.direction;
  const Rectf& region = request.region;

  // calculate the maximum number of steps needed for the gradient
  int n = static_cast<int>(std::max(std::max(fabsf(top.red - bottom.red),
                                             fabsf(top.green - bottom.green)),
                                    std::max(fabsf(top.blue - bottom.blue),
                                             fabsf(top.alpha - bottom.alpha))) * 255);
  n = std::max(n, 1);
  for (int i = 0; i < n; ++i)
  {
    SDL_Rect rect;

    if (direction == VERTICAL || direction == VERTICAL_SECTOR)
    {
      rect.x = static_cast<int>(region.get_left());
      rect.y = static_cast<int>(region.get_top() + (region.get_bottom() - region.get_top()) * static_cast<float>(i) / static_cast<float>(n));
      rect.w = static_cast<int>(region.get_right() - region.get_left());
      rect.h = static_cast<int>(ceilf((region.get_bottom() - region.get_top()) / static_cast<float>(n)));
    }
    else
    {
      rect.x = static_cast<int>(region.get_left() + (region.get_right() - region.get_left()) * static_cast<float>(i) / static_cast<float>(n));
      rect.y = static_cast<int>(region.get_top());
      rect.w = static_cast<int>(ceilf((region.get_right() - region.get_left()) / static_cast<float>(n)));
      rect.h = static_cast<int>(region.get_bottom() - region.get_top());
    }

    float p = static_cast<float>(i+1) / static_cast<float>(n);
    Uint8 r, g, b, a;

    if ( direction == HORIZONTAL_SECTOR || direction == VERTICAL_SECTOR)
    {
        float begin_percentage = region.get_left() * -1 / region.get_right();
        r = static_cast<Uint8>(((1.0f - begin_percentage - p) * top.red + (p + begin_percentage) * bottom.red)  * 255);
        g = static_cast<Uint8>(((1.0f - begin_percentage - p) * top.green + (p + begin_percentage) * bottom.green) * 255);
        b = static_cast<Uint8>(((1.0f - begin_percentage - p) * top.blue + (p + begin_percentage) * bottom.blue) * 255);
        a = static_cast<Uint8>(((1.0f - begin_percentage - p) * top.alpha + (p + begin_percentage) * bottom.alpha) * 255);
    }
    else
    {
        r = static_cast<Uint8>(((1.0f - p) * top.red + p * bottom.red)  * 255);
        g = static_cast<Uint8>(((1.0f - p) * top.green + p * bottom.green) * 255);
        b = static_cast<Uint8>(((1.0f - p) * top.blue + p * bottom.blue) * 255);
        a = static_cast<Uint8>(((1.0f - p) * top.alpha + p * bottom.alpha) * 255);
    }

    SDL_SetRenderDrawBlendMode(m_sdl_renderer, blend2sdl(request.blend));
    SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, a);
    SDL_RenderFillRect(m_sdl_renderer, &rect);
  }
}

void
SDLPainter::draw_filled_rect(const FillRectRequest& request)
{
  SDL_Rect rect = to_sdl_rect(request.rect);

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * 255);

  int radius = std::min(std::min(rect.h / 2, rect.w / 2),
                        static_cast<int>(request.radius));

  if (radius)
  {
    int slices = radius;

    // rounded top and bottom parts
    std::vector<SDL_Rect> rects;
    rects.reserve(2*slices + 1);
    for (int i = 0; i < slices; ++i)
    {
      float p = (static_cast<float>(i) + 0.5f) / static_cast<float>(slices);
      int xoff = radius - static_cast<int>(sqrtf(1.0f - p * p) * static_cast<float>(radius));

      SDL_Rect tmp;
      tmp.x = rect.x + xoff;
      tmp.y = rect.y + (radius - i);
      tmp.w = rect.w - 2*(xoff);
      tmp.h = 1;
      rects.push_back(tmp);

      SDL_Rect tmp2;
      tmp2.x = rect.x + xoff;
      tmp2.y = rect.y + rect.h - radius + i;
      tmp2.w = rect.w - 2*xoff;
      tmp2.h = 1;

      if (tmp2.y != tmp.y)
      {
        rects.push_back(tmp2);
      }
    }

    if (2*radius < rect.h)
    {
      // center rectangle
      SDL_Rect tmp;
      tmp.x = rect.x;
      tmp.y = rect.y + radius + 1;
      tmp.w = rect.w;
      tmp.h = rect.h - 2*radius - 1;
      rects.push_back(tmp);
    }

    SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, a);
    SDL_RenderFillRects(m_sdl_renderer, &*rects.begin(), static_cast<int>(rects.size()));
  }
  else
  {
    if ((rect.w != 0) && (rect.h != 0))
    {
      SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, a);
      SDL_RenderFillRect(m_sdl_renderer, &rect);
    }
  }
}

void
SDLPainter::draw_inverse_ellipse(const InverseEllipseRequest& request)
{
  float x = request.pos.x;
  float w = request.size.x;
  float h = request.size.y;

  int top = static_cast<int>(request.pos.y - (h / 2));

  const Viewport& viewport = m_video_system.get_viewport();

  const int max_slices = 256;
  SDL_Rect rects[2*max_slices+2];
  int slices = std::min(static_cast<int>(request.size.y), max_slices);
  for (int i = 0; i < slices; ++i)
  {
    float p = ((static_cast<float>(i) + 0.5f) / static_cast<float>(slices)) * 2.0f - 1.0f;
    int xoff = static_cast<int>(sqrtf(1.0f - p*p) * w / 2);

    SDL_Rect& left  = rects[2*i+0];
    SDL_Rect& right = rects[2*i+1];

    left.x = 0;
    left.y = top + (i * static_cast<int>(h) / slices);
    left.w = static_cast<int>(x) - xoff;
    left.h = top + ((i+1) * static_cast<int>(h) / slices) - left.y;

    right.x = static_cast<int>(x) + xoff;
    right.y = left.y;
    right.w = viewport.get_screen_width() - right.x;
    right.h = left.h;
  }

  SDL_Rect& top_rect = rects[2*slices+0];
  SDL_Rect& bottom_rect = rects[2*slices+1];

  top_rect.x = 0;
  top_rect.y = 0;
  top_rect.w = viewport.get_screen_width();
  top_rect.h = top;

  bottom_rect.x = 0;
  bottom_rect.y = top + static_cast<int>(h);
  bottom_rect.w = viewport.get_screen_width();
  bottom_rect.h = viewport.get_screen_height() - bottom_rect.y;

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * 255);

  SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, a);
  SDL_RenderFillRects(m_sdl_renderer, rects, 2*slices+2);
}

void
SDLPainter::draw_line(const LineRequest& request)
{
  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * 255);

  int x1 = static_cast<int>(request.pos.x);
  int y1 = static_cast<int>(request.pos.y);
  int x2 = static_cast<int>(request.dest_pos.x);
  int y2 = static_cast<int>(request.dest_pos.y);

  SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, a);
  SDL_RenderDrawLine(m_sdl_renderer, x1, y1, x2, y2);
}

namespace {

using Edge = std::pair<Vector, Vector>;

Edge
make_edge(int x1, int y1, int x2, int y2)
{
  if (y1 < y2)
  {
    return Edge(Vector(static_cast<float>(x1), static_cast<float>(y1)),
                Vector(static_cast<float>(x2), static_cast<float>(y2)));
  }
  else
  {
    return Edge(Vector(static_cast<float>(x2), static_cast<float>(y2)),
                Vector(static_cast<float>(x1), static_cast<float>(y1)));
  }
}

void
draw_span_between_edges(SDL_Renderer* renderer, const Edge& e1, const Edge& e2)
{
  // calculate difference between the y coordinates
  // of the first edge and return if 0
  float e1ydiff = static_cast<float>(e1.second.y - e1.first.y);
  if (e1ydiff == 0.0f)
    return;

  // calculate difference between the y coordinates
  // of the second edge and return if 0
  float e2ydiff = static_cast<float>(e2.second.y - e2.first.y);
  if (e2ydiff == 0.0f)
    return;

  float e1xdiff = e1.second.x - e1.first.x;
  float e2xdiff = e2.second.x - e2.first.x;
  float factor1 = (e2.first.y - e1.first.y) / e1ydiff;
  float factorStep1 = 1.0f / e1ydiff;
  float factor2 = 0.0f;
  float factorStep2 = 1.0f / e2ydiff;

  for (int y = static_cast<int>(e2.first.y); y < static_cast<int>(e2.second.y); y++) {
    SDL_RenderDrawLine(renderer,
                       static_cast<int>(e1.first.x + e1xdiff * factor1), y,
                       static_cast<int>(e2.first.x + e2xdiff * factor2), y);
    factor1 += factorStep1;
    factor2 += factorStep2;
  }
}

} //namespace

void
SDLPainter::draw_triangle(const TriangleRequest& request)
{
  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * 255);

  int x1 = static_cast<int>(request.pos1.x);
  int y1 = static_cast<int>(request.pos1.y);
  int x2 = static_cast<int>(request.pos2.x);
  int y2 = static_cast<int>(request.pos2.y);
  int x3 = static_cast<int>(request.pos3.x);
  int y3 = static_cast<int>(request.pos3.y);

  std::array<Edge, 3> edges{
    make_edge(x1, y1, x2, y2),
    make_edge(x2, y2, x3, y3),
    make_edge(x3, y3, x1, y1)
  };

  int maxLength = 0;
  int longEdge = 0;

  // find edge with the greatest length in the y axis
  for (int i = 0; i < 3; i++) {
    int length = static_cast<int>(edges[i].second.y - edges[i].first.y);
    if (length > maxLength) {
      maxLength = length;
      longEdge = i;
    }
  }
  int shortEdge1 = (longEdge + 1) % 3;
  int shortEdge2 = (longEdge + 2) % 3;

  SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, a);

  draw_span_between_edges(m_sdl_renderer, edges[longEdge], edges[shortEdge1]);
  draw_span_between_edges(m_sdl_renderer, edges[longEdge], edges[shortEdge2]);
}

void
SDLPainter::clear(const Color& color)
{
  SDL_SetRenderDrawColor(m_sdl_renderer, color.r8(), color.g8(), color.b8(), color.a8());

  if (m_cliprect)
  {
    SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(m_sdl_renderer, &*m_cliprect);
  }
  else
  {
    // This ignores the cliprect:
    SDL_RenderClear(m_sdl_renderer);
  }
}

void
SDLPainter::set_clip_rect(const Rect& rect)
{
  m_cliprect = SDL_Rect{ rect.left,
                         rect.top,
                         rect.get_width(),
                         rect.get_height() };

  int ret = SDL_RenderSetClipRect(m_sdl_renderer, &*m_cliprect);
  if (ret < 0)
  {
    log_warning << "SDLPainter::set_clip_rect(): SDL_RenderSetClipRect() failed: " << SDL_GetError() << std::endl;
  }
}

void
SDLPainter::clear_clip_rect()
{
  m_cliprect.reset();

  int ret = SDL_RenderSetClipRect(m_sdl_renderer, nullptr);
  if (ret < 0)
  {
    log_warning << "SDLPainter::clear_clip_rect(): SDL_RenderSetClipRect() failed: " << SDL_GetError() << std::endl;
  }
}

void
SDLPainter::get_pixel(const GetPixelRequest& request) const
{
  const Rect& rect = m_renderer.get_rect();
  const Size& logical_size = m_renderer.get_logical_size();

  SDL_Rect srcrect;
  srcrect.x = rect.left + static_cast<int>(request.pos.x * static_cast<float>(rect.get_width()) / static_cast<float>(logical_size.width));
  srcrect.y = rect.top + static_cast<int>(request.pos.y * static_cast<float>(rect.get_height()) / static_cast<float>(logical_size.height));
  srcrect.w = 1;
  srcrect.h = 1;

  Uint8 pixel[4];
  int ret = SDL_RenderReadPixels(m_sdl_renderer, &srcrect,
                                 SDL_PIXELFORMAT_RGB888,
                                 pixel,
                                 1);
  if (ret != 0)
  {
    log_warning << "failed to read pixels: " << SDL_GetError() << std::endl;
  }

  *(request.color_ptr) = Color::from_rgb888(pixel[2], pixel[1], pixel[0]);
}

/* EOF */
