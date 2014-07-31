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
  const Surface* surface = (const Surface*) request.request_data;
  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->get_texture());

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
  if (surface->get_flipx() || request.drawing_effect == HORIZONTAL_FLIP)
  {
    flip = SDL_FLIP_HORIZONTAL;
  }
  else if (request.drawing_effect == VERTICAL_FLIP)
  {
    flip = SDL_FLIP_VERTICAL;
  }

  SDL_RenderCopyEx(renderer, sdltexture->get_texture(), NULL, &dst_rect, request.angle, NULL, flip);
}

void
SDLPainter::draw_surface_part(SDL_Renderer* renderer, const DrawingRequest& request)
{
  //FIXME: support parameters request.blend
  const SurfacePartRequest* surface = (const SurfacePartRequest*) request.request_data;
  const SurfacePartRequest* surfacepartrequest = (SurfacePartRequest*) request.request_data;

  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->surface->get_texture());

  SDL_Rect src_rect;
  src_rect.x = surfacepartrequest->source.x;
  src_rect.y = surfacepartrequest->source.y;
  src_rect.w = surfacepartrequest->size.x;
  src_rect.h = surfacepartrequest->size.y;

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = surfacepartrequest->size.x;
  dst_rect.h = surfacepartrequest->size.y;

  Uint8 r = static_cast<Uint8>(request.color.red * 255);
  Uint8 g = static_cast<Uint8>(request.color.green * 255);
  Uint8 b = static_cast<Uint8>(request.color.blue * 255);
  Uint8 a = static_cast<Uint8>(request.color.alpha * request.alpha * 255);
  SDL_SetTextureColorMod(sdltexture->get_texture(), r, g, b);
  SDL_SetTextureAlphaMod(sdltexture->get_texture(), a);
  SDL_SetTextureBlendMode(sdltexture->get_texture(), blend2sdl(request.blend));

  SDL_RendererFlip flip = SDL_FLIP_NONE;
  if (surface->surface->get_flipx() || request.drawing_effect == HORIZONTAL_FLIP)
  {
    flip = SDL_FLIP_HORIZONTAL;
  }
  else if (request.drawing_effect == VERTICAL_FLIP)
  {
    flip = SDL_FLIP_VERTICAL;
  }

  SDL_RenderCopyEx(renderer, sdltexture->get_texture(), &src_rect, &dst_rect, request.angle, NULL, flip);
}

void
SDLPainter::draw_gradient(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const GradientRequest* gradientrequest 
    = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  // calculate the maximum number of steps needed for the gradient
  int n = static_cast<int>(std::max(std::max(fabsf(top.red - bottom.red),
                                             fabsf(top.green - bottom.green)),
                                    std::max(fabsf(top.blue - bottom.blue),
                                             fabsf(top.alpha - bottom.alpha))) * 255);
  for(int i = 0; i < n; ++i)
  {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = SCREEN_HEIGHT * i / n;
    rect.w = SCREEN_WIDTH;
    rect.h = (SCREEN_HEIGHT * (i+1) / n) - rect.y;

    float p = static_cast<float>(i+1) / static_cast<float>(n);
    Uint8 r = static_cast<Uint8>(((1.0f - p) * top.red + p * bottom.red)  * 255);
    Uint8 g = static_cast<Uint8>(((1.0f - p) * top.green + p * bottom.green) * 255);
    Uint8 b = static_cast<Uint8>(((1.0f - p) * top.blue + p * bottom.blue) * 255);
    Uint8 a = static_cast<Uint8>(((1.0f - p) * top.alpha + p * bottom.alpha) * 255);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
  }
}

void
SDLPainter::draw_filled_rect(SDL_Renderer* renderer, const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

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
  const InverseEllipseRequest* ellipse = (InverseEllipseRequest*)request.request_data;
  
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

/* EOF */
