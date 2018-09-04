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
#include <SDL_ttf.h>
#include <algorithm>

#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/drawing_request.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/sdl/sdl_video_system.hpp"
#include "video/viewport.hpp"

namespace {

SDL_BlendMode blend2sdl(const Blend& blend)
{
  if (blend.sfactor == GL_ONE &&
      blend.dfactor == GL_ZERO)
  {
    return SDL_BLENDMODE_NONE;
  }
  else if (blend.sfactor == GL_SRC_ALPHA &&
           blend.dfactor == GL_ONE_MINUS_SRC_ALPHA)
  {
    return SDL_BLENDMODE_BLEND;
  }
  else if (blend.sfactor == GL_SRC_ALPHA &&
           blend.dfactor == GL_ONE)
  {
    return SDL_BLENDMODE_ADD;
  }
  else if (blend.sfactor == GL_DST_COLOR &&
           blend.dfactor == GL_ZERO)
  {
    return SDL_BLENDMODE_MOD;
  }
  else
  {
    log_warning << "unknown blend mode combinations: sfactor=" << blend.sfactor << " dfactor=" << blend.dfactor << std::endl;
    return SDL_BLENDMODE_BLEND;
  }
}

} // namespace

SDLPainter::SDLPainter(SDLVideoSystem& video_system, SDL_Renderer* renderer) :
  m_video_system(video_system),
  m_renderer(renderer)
{}

void
SDLPainter::draw_texture(const DrawingRequest& request)
{
  const auto& data = static_cast<const TextureRequest&>(request);
  const auto& texture = static_cast<const SDLTexture&>(*data.texture);

  SDL_Rect src_rect;
  src_rect.x = static_cast<int>(data.srcrect.p1.x);
  src_rect.y = static_cast<int>(data.srcrect.p1.y);
  src_rect.w = static_cast<int>(data.srcrect.get_width());
  src_rect.h = static_cast<int>(data.srcrect.get_height());

  SDL_Rect dst_rect;
  dst_rect.x = static_cast<int>(data.dstrect.p1.x);
  dst_rect.y = static_cast<int>(data.dstrect.p1.y);
  dst_rect.w = static_cast<int>(data.dstrect.get_width());
  dst_rect.h = static_cast<int>(data.dstrect.get_height());

  Uint8 r = static_cast<Uint8>(data.color.red * 255);
  Uint8 g = static_cast<Uint8>(data.color.green * 255);
  Uint8 b = static_cast<Uint8>(data.color.blue * 255);
  Uint8 a = static_cast<Uint8>(data.color.alpha * request.alpha * 255);

  SDL_SetTextureColorMod(texture.get_texture(), r, g, b);
  SDL_SetTextureAlphaMod(texture.get_texture(), a);
  SDL_SetTextureBlendMode(texture.get_texture(), blend2sdl(request.blend));

  SDL_RendererFlip flip = SDL_FLIP_NONE;
  if ((request.drawing_effect & HORIZONTAL_FLIP) != 0)
  {
    flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
  }

  if ((request.drawing_effect & VERTICAL_FLIP) != 0)
  {
    flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
  }

  SDL_RenderCopyEx(m_renderer, texture.get_texture(), &src_rect, &dst_rect, request.angle, NULL, flip);
}

void
SDLPainter::draw_texture_batch(const DrawingRequest& request)
{
  const auto& data = static_cast<const TextureBatchRequest&>(request);
  const auto& texture = static_cast<const SDLTexture&>(*data.texture);

  assert(data.srcrects.size() == data.dstrects.size());

  for(size_t i = 0; i < data.srcrects.size(); ++i)
  {
    SDL_Rect src_rect;
    src_rect.x = static_cast<int>(data.srcrects[i].p1.x);
    src_rect.y = static_cast<int>(data.srcrects[i].p1.y);
    src_rect.w = static_cast<int>(data.srcrects[i].get_width());
    src_rect.h = static_cast<int>(data.srcrects[i].get_height());

    SDL_Rect dst_rect;
    dst_rect.x = static_cast<int>(data.dstrects[i].p1.x);
    dst_rect.y = static_cast<int>(data.dstrects[i].p1.y);
    dst_rect.w = static_cast<int>(data.dstrects[i].get_width());
    dst_rect.h = static_cast<int>(data.dstrects[i].get_height());

    Uint8 r = static_cast<Uint8>(data.color.red * 255);
    Uint8 g = static_cast<Uint8>(data.color.green * 255);
    Uint8 b = static_cast<Uint8>(data.color.blue * 255);
    Uint8 a = static_cast<Uint8>(data.color.alpha * request.alpha * 255);

    SDL_SetTextureColorMod(texture.get_texture(), r, g, b);
    SDL_SetTextureAlphaMod(texture.get_texture(), a);
    SDL_SetTextureBlendMode(texture.get_texture(), blend2sdl(request.blend));

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if ((request.drawing_effect & HORIZONTAL_FLIP) != 0)
    {
      flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    }

    if ((request.drawing_effect & VERTICAL_FLIP) != 0)
    {
      flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
    }

    SDL_RenderCopyEx(m_renderer, texture.get_texture(), &src_rect, &dst_rect, request.angle, NULL, flip);
  }
}

void
SDLPainter::draw_gradient(const DrawingRequest& request)
{
  const auto& data = static_cast<const GradientRequest&>(request);
  const Color& top = data.top;
  const Color& bottom = data.bottom;
  const GradientDirection& direction = data.direction;
  const Rectf& region = data.region;

  // calculate the maximum number of steps needed for the gradient
  int n = static_cast<int>(std::max(std::max(fabsf(top.red - bottom.red),
                                             fabsf(top.green - bottom.green)),
                                    std::max(fabsf(top.blue - bottom.blue),
                                             fabsf(top.alpha - bottom.alpha))) * 255);
  n = std::max(n, 1);
  for(int i = 0; i < n; ++i)
  {
    SDL_Rect rect;
    if(direction == VERTICAL || direction == VERTICAL_SECTOR)
    {
      rect.x = static_cast<int>(region.p1.x);
      rect.y = static_cast<int>(region.p2.y * static_cast<float>(i) / static_cast<float>(n));
      rect.w = static_cast<int>(region.p2.x);
      rect.h = static_cast<int>((region.p2.y * static_cast<float>(i+1) / static_cast<float>(n)) - static_cast<float>(rect.y));
    }
    else
    {
      rect.x = static_cast<int>(region.p2.x * static_cast<float>(i) / static_cast<float>(n));
      rect.y = static_cast<int>(region.p1.y);
      rect.w = static_cast<int>((region.p2.x * static_cast<float>(i+1) / static_cast<float>(n)) - static_cast<float>(rect.x));
      rect.h = static_cast<int>(region.p2.y);
    }

    float p = static_cast<float>(i+1) / static_cast<float>(n);
    Uint8 r, g, b, a;

    if( direction == HORIZONTAL_SECTOR || direction == VERTICAL_SECTOR)
    {
        float begin_percentage = region.p1.x * -1 / region.p2.x;
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

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderFillRect(m_renderer, &rect);
  }
}

void
SDLPainter::draw_filled_rect(const DrawingRequest& request)
{
  const auto& data = static_cast<const FillRectRequest&>(request);

  SDL_Rect rect;
  rect.x = static_cast<int>(data.pos.x);
  rect.y = static_cast<int>(data.pos.y);
  rect.w = static_cast<int>(data.size.x);
  rect.h = static_cast<int>(data.size.y);

  Uint8 r = static_cast<Uint8>(data.color.red * 255);
  Uint8 g = static_cast<Uint8>(data.color.green * 255);
  Uint8 b = static_cast<Uint8>(data.color.blue * 255);
  Uint8 a = static_cast<Uint8>(data.color.alpha * 255);

  int radius = std::min(std::min(rect.h / 2, rect.w / 2),
                        static_cast<int>(data.radius));

  if (radius)
  {
    int slices = radius;

    // rounded top and bottom parts
    std::vector<SDL_Rect> rects;
    rects.reserve(2*slices + 1);
    for(int i = 0; i < slices; ++i)
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

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderFillRects(m_renderer, &*rects.begin(), static_cast<int>(rects.size()));
  }
  else
  {
    if((rect.w != 0) && (rect.h != 0))
    {
      SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
      SDL_RenderFillRect(m_renderer, &rect);
    }
  }
}

void
SDLPainter::draw_inverse_ellipse(const DrawingRequest& request)
{
  const auto& data = static_cast<const InverseEllipseRequest&>(request);

  float x = data.pos.x;
  float w = data.size.x;
  float h = data.size.y;

  int top = static_cast<int>(data.pos.y - (h / 2));

  const Viewport& viewport = m_video_system.get_viewport();

  const int max_slices = 256;
  SDL_Rect rects[2*max_slices+2];
  int slices = std::min(static_cast<int>(data.size.y), max_slices);
  for(int i = 0; i < slices; ++i)
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

  Uint8 r = static_cast<Uint8>(data.color.red * 255);
  Uint8 g = static_cast<Uint8>(data.color.green * 255);
  Uint8 b = static_cast<Uint8>(data.color.blue * 255);
  Uint8 a = static_cast<Uint8>(data.color.alpha * 255);

  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
  SDL_RenderFillRects(m_renderer, rects, 2*slices+2);
}

void
SDLPainter::draw_text(const DrawingRequest& request)
{
  const auto& data = static_cast<const TextRequest&>(request);

  auto font = data.font->get_ttf_font();
  int line_height = static_cast<int>(data.font->get_height());
  int shadow_size = static_cast<int>(data.font->get_shadow_size());

  // 2 pixel shadow looks "weird" on the menu items, but only in SDL renderer
  if(shadow_size > 1)
    shadow_size -= 1;

  int last_pos = 0;
  int last_y = static_cast<int>(data.pos.y);
  for(size_t i = 0; i < data.text.length(); i++)
  {
    if(data.text[i] != '\n' /* new line */ && i != data.text.length() - 1 /* end of string */)
    {
      continue;
    }
    std::string str;
    if(data.text[i] == '\n')
      str = data.text.substr(last_pos, i - last_pos);
    else
      str = data.text.substr(last_pos, i + 1);

    last_pos = static_cast<int>(i + 1);

    if(!data.font->has_all_glyphs(str))
    {
      font = Resources::get_fallback_font(data.font)->get_ttf_font();
    }
    else
    {
      font = data.font->get_ttf_font();
    }

    auto texture = std::dynamic_pointer_cast<SDLTexture>(
      TextureManager::current()->get(font, str, data.color));

    SDL_Rect dst_rect;
    dst_rect.x = static_cast<int>(data.pos.x);
    dst_rect.y = last_y;
    dst_rect.w = texture->get_texture_width();
    dst_rect.h = texture->get_texture_height();

    if(data.alignment == ALIGN_CENTER)
      dst_rect.x -= texture->get_texture_width() / 2;
    else if(data.alignment == ALIGN_RIGHT)
      dst_rect.x -= texture->get_texture_width();

    SDL_Rect dst_shadow_rect = dst_rect;
    dst_shadow_rect.x += shadow_size;
    dst_shadow_rect.y += shadow_size;
    dst_shadow_rect.w += shadow_size;
    dst_shadow_rect.h += shadow_size;

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (request.drawing_effect & HORIZONTAL_FLIP)
    {
      flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    }

    if (request.drawing_effect & VERTICAL_FLIP)
    {
      flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
    }

    SDL_SetTextureColorMod(texture->get_texture(), 0, 0, 0);
    SDL_SetTextureAlphaMod(texture->get_texture(), static_cast<Uint8>(request.alpha * 0.45 * 255));
    SDL_RenderCopyEx(m_renderer, texture->get_texture(), NULL, &dst_shadow_rect, request.angle, NULL, flip);

    SDL_SetTextureColorMod(texture->get_texture(), 255, 255, 255);
    SDL_SetTextureAlphaMod(texture->get_texture(), static_cast<Uint8>(request.alpha * 255));
    SDL_RenderCopyEx(m_renderer, texture->get_texture(), NULL, &dst_rect, request.angle, NULL, flip);

    last_y += line_height;
  }
}

void
SDLPainter::draw_line(const DrawingRequest& request)
{
  const auto& data = static_cast<const LineRequest&>(request);

  Uint8 r = static_cast<Uint8>(data.color.red * 255);
  Uint8 g = static_cast<Uint8>(data.color.green * 255);
  Uint8 b = static_cast<Uint8>(data.color.blue * 255);
  Uint8 a = static_cast<Uint8>(data.color.alpha * 255);

  int x1 = static_cast<int>(data.pos.x);
  int y1 = static_cast<int>(data.pos.y);
  int x2 = static_cast<int>(data.dest_pos.x);
  int y2 = static_cast<int>(data.dest_pos.y);

  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
  SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
}

namespace {

Rectf
make_edge(int x1, int y1, int x2, int y2)
{
  if(y1 < y2)
  {
    return Rectf(Vector(static_cast<float>(x1), static_cast<float>(y1)),
                 Vector(static_cast<float>(x2), static_cast<float>(y2)));
  }
  else
  {
    return Rectf(Vector(static_cast<float>(x2), static_cast<float>(y2)),
                 Vector(static_cast<float>(x1), static_cast<float>(y1)));
  }
}

void
draw_span_between_edges(SDL_Renderer* renderer, const Rectf& e1, const Rectf& e2)
{
  // calculate difference between the y coordinates
  // of the first edge and return if 0
  float e1ydiff = static_cast<float>(e1.p2.y - e1.p1.y);
  if(e1ydiff == 0.0f)
    return;

  // calculate difference between the y coordinates
  // of the second edge and return if 0
  float e2ydiff = static_cast<float>(e2.p2.y - e2.p1.y);
  if(e2ydiff == 0.0f)
    return;

  float e1xdiff = e1.p2.x - e1.p1.x;
  float e2xdiff = e2.p2.x - e2.p1.x;
  float factor1 = (e2.p1.y - e1.p1.y) / e1ydiff;
  float factorStep1 = 1.0f / e1ydiff;
  float factor2 = 0.0f;
  float factorStep2 = 1.0f / e2ydiff;

  for(int y = static_cast<int>(e2.p1.y); y < static_cast<int>(e2.p2.y); y++) {
    SDL_RenderDrawLine(renderer,
                       static_cast<int>(e1.p1.x + e1xdiff * factor1), y,
                       static_cast<int>(e2.p1.x + e2xdiff * factor2), y);
    factor1 += factorStep1;
    factor2 += factorStep2;
  }
}

} //namespace

void
SDLPainter::draw_triangle(const DrawingRequest& request)
{
  const auto& data = static_cast<const TriangleRequest&>(request);

  Uint8 r = static_cast<Uint8>(data.color.red * 255);
  Uint8 g = static_cast<Uint8>(data.color.green * 255);
  Uint8 b = static_cast<Uint8>(data.color.blue * 255);
  Uint8 a = static_cast<Uint8>(data.color.alpha * 255);

  int x1 = static_cast<int>(data.pos1.x);
  int y1 = static_cast<int>(data.pos1.y);
  int x2 = static_cast<int>(data.pos2.x);
  int y2 = static_cast<int>(data.pos2.y);
  int x3 = static_cast<int>(data.pos3.x);
  int y3 = static_cast<int>(data.pos3.y);

  Rectf edges[3];
  edges[0] = make_edge(x1, y1, x2, y2);
  edges[1] = make_edge(x2, y2, x3, y3);
  edges[2] = make_edge(x3, y3, x1, y1);

  int maxLength = 0;
  int longEdge = 0;

  // find edge with the greatest length in the y axis
  for(int i = 0; i < 3; i++) {
    int length = static_cast<int>(edges[i].p2.y - edges[i].p1.y);
    if(length > maxLength) {
      maxLength = length;
      longEdge = i;
    }
  }
  int shortEdge1 = (longEdge + 1) % 3;
  int shortEdge2 = (longEdge + 2) % 3;

  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(m_renderer, r, g, b, a);

  draw_span_between_edges(m_renderer, edges[longEdge], edges[shortEdge1]);
  draw_span_between_edges(m_renderer, edges[longEdge], edges[shortEdge2]);
}

/* EOF */
