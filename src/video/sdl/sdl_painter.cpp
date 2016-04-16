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

#include "SDL.h"

#include "math/rectf.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/drawing_request.hpp"
#include "video/sdl/sdl_texture.hpp"

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

void
SDLPainter::draw_surface(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const auto surface = static_cast<const SurfaceRequest*>(request.request_data)->surface;
  std::shared_ptr<SDLTexture> sdltexture = std::dynamic_pointer_cast<SDLTexture>(surface->get_texture());

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = sdltexture->get_image_width();
  dst_rect.h = sdltexture->get_image_height();

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * request.alpha * 255);
  SDL_SetTextureColorMod(sdltexture->get_texture(), r, g, b);
  SDL_SetTextureAlphaMod(sdltexture->get_texture(), a);
  SDL_SetTextureBlendMode(sdltexture->get_texture(), blend2sdl(request.blend));

  SDL_RendererFlip flip = SDL_FLIP_NONE;
  if (surface->get_flipx() || request.drawing_effect & HORIZONTAL_FLIP)
  {
    flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
  }

  if (request.drawing_effect & VERTICAL_FLIP)
  {
    flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
  }

  SDL_RenderCopyEx(renderer, sdltexture->get_texture(), NULL, &dst_rect, request.angle, NULL, flip);
}

void
SDLPainter::draw_surface_part(SDL_Renderer* renderer, const DrawingRequest& request)
{
  //FIXME: support parameters request.blend
  const auto surface = static_cast<const SurfacePartRequest*>(request.request_data);
  const auto surfacepartrequest = static_cast<SurfacePartRequest*>(request.request_data);

  std::shared_ptr<SDLTexture> sdltexture = std::dynamic_pointer_cast<SDLTexture>(surface->surface->get_texture());

  SDL_Rect src_rect;
  src_rect.x = surfacepartrequest->srcrect.p1.x;
  src_rect.y = surfacepartrequest->srcrect.p1.y;
  src_rect.w = surfacepartrequest->srcrect.get_width();
  src_rect.h = surfacepartrequest->srcrect.get_height();

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = surfacepartrequest->dstsize.width;
  dst_rect.h = surfacepartrequest->dstsize.height;

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * request.alpha * 255);
  SDL_SetTextureColorMod(sdltexture->get_texture(), r, g, b);
  SDL_SetTextureAlphaMod(sdltexture->get_texture(), a);
  SDL_SetTextureBlendMode(sdltexture->get_texture(), blend2sdl(request.blend));

  SDL_RendererFlip flip = SDL_FLIP_NONE;
  if (surface->surface->get_flipx() || request.drawing_effect & HORIZONTAL_FLIP)
  {
    flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
  }

  if (request.drawing_effect & VERTICAL_FLIP)
  {
    flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);
  }

  SDL_RenderCopyEx(renderer, sdltexture->get_texture(), &src_rect, &dst_rect, request.angle, NULL, flip);
}

void
SDLPainter::draw_gradient(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const auto gradientrequest = static_cast<GradientRequest*>(request.request_data);
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;
  const GradientDirection& direction = gradientrequest->direction;
  const Rectf& region = gradientrequest->region;

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
      rect.x = region.p1.x;
      rect.y = region.p2.y * i / n;
      rect.w = region.p2.x;
      rect.h = (region.p2.y * (i+1) / n) - rect.y;
    }
    else
    {
      rect.x = region.p2.x * i / n;
      rect.y = region.p1.y;
      rect.w = (region.p2.x * (i+1) / n) - rect.x;
      rect.h = region.p2.y;
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
  }
}

void
SDLPainter::draw_filled_rect(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const auto fillrectrequest = static_cast<FillRectRequest*>(request.request_data);

  SDL_Rect rect;
  rect.x = request.pos.x;
  rect.y = request.pos.y;
  rect.w = fillrectrequest->size.x;
  rect.h = fillrectrequest->size.y;

  Uint8 r = static_cast<Uint8>(fillrectrequest->color.red * 255);
  Uint8 g = static_cast<Uint8>(fillrectrequest->color.green * 255);
  Uint8 b = static_cast<Uint8>(fillrectrequest->color.blue * 255);
  Uint8 a = static_cast<Uint8>(fillrectrequest->color.alpha * 255);

  int radius = std::min(std::min(rect.h / 2, rect.w / 2),
                        static_cast<int>(fillrectrequest->radius));

  if (radius)
  {
    int slices = radius;

    // rounded top and bottom parts
    std::vector<SDL_Rect> rects;
    rects.reserve(2*slices + 1);
    for(int i = 0; i < slices; ++i)
    {
      float p = (static_cast<float>(i) + 0.5f) / static_cast<float>(slices);
      int xoff = radius - static_cast<int>(sqrtf(1.0f - p*p) * radius);

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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRects(renderer, &*rects.begin(), rects.size());
  }
  else
  {
    if((rect.w != 0) && (rect.h != 0))
    {
      SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(renderer, r, g, b, a);
      SDL_RenderFillRect(renderer, &rect);
    }
  }
}

void
SDLPainter::draw_inverse_ellipse(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const auto ellipse = static_cast<InverseEllipseRequest*>(request.request_data);

  float x = request.pos.x;
  float w = ellipse->size.x;
  float h = ellipse->size.y;

  int top = request.pos.y - (h / 2);

  const int max_slices = 256;
  SDL_Rect rects[2*max_slices+2];
  int slices = std::min(static_cast<int>(ellipse->size.y), max_slices);
  for(int i = 0; i < slices; ++i)
  {
    float p = ((static_cast<float>(i) + 0.5f) / static_cast<float>(slices)) * 2.0f - 1.0f;
    int xoff = static_cast<int>(sqrtf(1.0f - p*p) * w / 2);

    SDL_Rect& left  = rects[2*i+0];
    SDL_Rect& right = rects[2*i+1];

    left.x = 0;
    left.y = top + (i * h / slices);
    left.w = x - xoff;
    left.h = (top + ((i+1) * h / slices)) - left.y;

    right.x = x + xoff;
    right.y = left.y;
    right.w = SCREEN_WIDTH - right.x;
    right.h = left.h;
  }

  SDL_Rect& top_rect = rects[2*slices+0];
  SDL_Rect& bottom_rect = rects[2*slices+1];

  top_rect.x = 0;
  top_rect.y = 0;
  top_rect.w = SCREEN_WIDTH;
  top_rect.h = top;

  bottom_rect.x = 0;
  bottom_rect.y = top + h;
  bottom_rect.w = SCREEN_WIDTH;
  bottom_rect.h = SCREEN_HEIGHT - bottom_rect.y;

  Uint8 r = static_cast<Uint8>(ellipse->color.red * 255);
  Uint8 g = static_cast<Uint8>(ellipse->color.green * 255);
  Uint8 b = static_cast<Uint8>(ellipse->color.blue * 255);
  Uint8 a = static_cast<Uint8>(ellipse->color.alpha * 255);

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_RenderFillRects(renderer, rects, 2*slices+2);
}

void
SDLPainter::draw_text(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const TextRequest* textrequest = static_cast<TextRequest*>(request.request_data);

  auto font = textrequest->font->get_ttf_font();
  int line_height = textrequest->font->get_height();
  int shadow_size = textrequest->font->get_shadow_size();

  // 2 pixel shadow looks "weird" on the menu items, but only in SDL renderer
  if(shadow_size > 1)
    shadow_size -= 1;

  int last_pos = 0;
  int last_y = request.pos.y;
  for(size_t i = 0; i < textrequest->text.length(); i++)
  {
    if(textrequest->text[i] != '\n' /* new line */ && i != textrequest->text.length() - 1 /* end of string */)
    {
      continue;
    }
    std::string str;
    if(textrequest->text[i] == '\n')
      str = textrequest->text.substr(last_pos, i - last_pos);
    else
      str = textrequest->text.substr(last_pos, i + 1);

    last_pos = i + 1;

    auto texture = std::dynamic_pointer_cast<SDLTexture>(
      TextureManager::current()->get(font, str, request.color));

    SDL_Rect dst_rect;
    dst_rect.x = request.pos.x;
    dst_rect.y = last_y;
    dst_rect.w = texture->get_texture_width();
    dst_rect.h = texture->get_texture_height();

    if(textrequest->alignment == ALIGN_CENTER)
      dst_rect.x -= texture->get_texture_width() / 2;
    else if(textrequest->alignment == ALIGN_RIGHT)
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
    SDL_SetTextureAlphaMod(texture->get_texture(), request.alpha * 0.45 * 255);
    SDL_RenderCopyEx(renderer, texture->get_texture(), NULL, &dst_shadow_rect, request.angle, NULL, flip);

    SDL_SetTextureColorMod(texture->get_texture(), 255, 255, 255);
    SDL_SetTextureAlphaMod(texture->get_texture(), request.alpha * 255);
    SDL_RenderCopyEx(renderer, texture->get_texture(), NULL, &dst_rect, request.angle, NULL, flip);

    last_y += line_height;
  }
}

void
SDLPainter::draw_line(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const auto linerequest = static_cast<LineRequest*>(request.request_data);

  Uint8 r = static_cast<Uint8>(linerequest->color.red * 255);
  Uint8 g = static_cast<Uint8>(linerequest->color.green * 255);
  Uint8 b = static_cast<Uint8>(linerequest->color.blue * 255);
  Uint8 a = static_cast<Uint8>(linerequest->color.alpha * 255);

  int x1 = request.pos.x;
  int y1 = request.pos.y;
  int x2 = linerequest->dest_pos.x;
  int y2 = linerequest->dest_pos.y;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);
  SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

namespace {

Rectf
make_edge(int x1, int y1, int x2, int y2)
{
  if(y1 < y2) {
    return Rectf(Vector(x1, y1), Vector(x2, y2));
  } else {
    return Rectf(Vector(x2, y2), Vector(x1, y1));
  }
}

void
draw_span_between_edges(SDL_Renderer* renderer, const Rectf& e1, const Rectf& e2)
{
  // calculate difference between the y coordinates
  // of the first edge and return if 0
  float e1ydiff = (float)(e1.p2.y - e1.p1.y);
  if(e1ydiff == 0.0f)
    return;

  // calculate difference between the y coordinates
  // of the second edge and return if 0
  float e2ydiff = (float)(e2.p2.y - e2.p1.y);
  if(e2ydiff == 0.0f)
    return;

  float e1xdiff = e1.p2.x - e1.p1.x;
  float e2xdiff = e2.p2.x - e2.p1.x;
  float factor1 = (e2.p1.y - e1.p1.y) / e1ydiff;
  float factorStep1 = 1.0f / e1ydiff;
  float factor2 = 0.0f;
  float factorStep2 = 1.0f / e2ydiff;

  for(int y = e2.p1.y; y < e2.p2.y; y++) {
    SDL_RenderDrawLine(renderer, e1.p1.x + e1xdiff * factor1, y, e2.p1.x + e2xdiff * factor2, y);
    factor1 += factorStep1;
    factor2 += factorStep2;
  }
}

} //namespace

void
SDLPainter::draw_triangle(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const auto trianglerequest = static_cast<TriangleRequest*>(request.request_data);

  Uint8 r = static_cast<Uint8>(trianglerequest->color.red * 255);
  Uint8 g = static_cast<Uint8>(trianglerequest->color.green * 255);
  Uint8 b = static_cast<Uint8>(trianglerequest->color.blue * 255);
  Uint8 a = static_cast<Uint8>(trianglerequest->color.alpha * 255);

  int x1 = request.pos.x;
  int y1 = request.pos.y;
  int x2 = trianglerequest->pos2.x;
  int y2 = trianglerequest->pos2.y;
  int x3 = trianglerequest->pos3.x;
  int y3 = trianglerequest->pos3.y;

  Rectf edges[3];
  edges[0] = make_edge(x1, y1, x2, y2);
  edges[1] = make_edge(x2, y2, x3, y3);
  edges[2] = make_edge(x3, y3, x1, y1);

  int maxLength = 0;
  int longEdge = 0;

  // find edge with the greatest length in the y axis
  for(int i = 0; i < 3; i++) {
    int length = edges[i].p2.y - edges[i].p1.y;
    if(length > maxLength) {
      maxLength = length;
      longEdge = i;
    }
  }
  int shortEdge1 = (longEdge + 1) % 3;
  int shortEdge2 = (longEdge + 2) % 3;

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);

  draw_span_between_edges(renderer, edges[longEdge], edges[shortEdge1]);
  draw_span_between_edges(renderer, edges[longEdge], edges[shortEdge2]);
}

/* EOF */
