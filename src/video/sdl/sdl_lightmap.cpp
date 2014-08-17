//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include <iostream>

#include "video/sdl/sdl_lightmap.hpp"
#include "video/sdl/sdl_surface_data.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/sdl/sdl_renderer.hpp"
#include "video/sdl/sdl_painter.hpp"

SDLLightmap::SDLLightmap() :
  renderer(static_cast<SDLRenderer*>(Renderer::instance())->get_sdl_renderer()),
  texture(),
  width(),
  height(),
  LIGHTMAP_DIV()
{
  LIGHTMAP_DIV = 8;

  width = SCREEN_WIDTH;
  height = SCREEN_HEIGHT;

  SDL_Renderer* renderer = static_cast<SDLRenderer*>(Renderer::instance())->get_sdl_renderer();
  texture = SDL_CreateTexture(renderer,
                              SDL_PIXELFORMAT_RGB888,
                              SDL_TEXTUREACCESS_TARGET,
                              width / LIGHTMAP_DIV,
                              height / LIGHTMAP_DIV);
  if (!texture)
  {
    std::stringstream msg;
    msg << "Couldn't create lightmap texture: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
}

SDLLightmap::~SDLLightmap()
{
  SDL_DestroyTexture(texture);
}

void
SDLLightmap::start_draw(const Color &ambient_color)
{
  SDL_SetRenderTarget(renderer, texture);

  Uint8 r = static_cast<Uint8>(ambient_color.red * 255);
  Uint8 g = static_cast<Uint8>(ambient_color.green * 255);
  Uint8 b = static_cast<Uint8>(ambient_color.blue * 255);

  SDL_SetRenderDrawColor(renderer, r, g, b, 255);
  SDL_RenderClear(renderer);
  SDL_RenderSetScale(renderer, 1.0f / LIGHTMAP_DIV, 1.0f / LIGHTMAP_DIV);
}

void
SDLLightmap::end_draw()
{
  SDL_RenderSetScale(renderer, 1.0f, 1.0f);
  SDL_SetRenderTarget(renderer, NULL);
}

void
SDLLightmap::do_draw()
{
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MOD);

  SDL_Rect dst_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.w = width;
  dst_rect.h = height;

  SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
}

void
SDLLightmap::draw_surface(const DrawingRequest& request)
{
  SDLPainter::draw_surface(renderer, request);
}

void
SDLLightmap::draw_surface_part(const DrawingRequest& request)
{
  SDLPainter::draw_surface_part(renderer, request);
}

void
SDLLightmap::draw_gradient(const DrawingRequest& request)
{
  SDLPainter::draw_gradient(renderer, request);
}

void
SDLLightmap::draw_filled_rect(const DrawingRequest& request)
{
  SDLPainter::draw_filled_rect(renderer, request);
}

void
SDLLightmap::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest
    = (GetLightRequest*) request.request_data;

  SDL_Rect rect;
  rect.x = static_cast<int>(request.pos.x * width / SCREEN_WIDTH);
  rect.y = static_cast<int>(request.pos.y * height / SCREEN_HEIGHT);
  rect.w = 1;
  rect.h = 1;

  SDL_SetRenderTarget(renderer, texture);
  Uint8 pixel[4];
  int ret = SDL_RenderReadPixels(renderer, &rect,
                                 SDL_PIXELFORMAT_RGB888,
                                 pixel,
                                 1);
  if (ret != 0)
  {
    log_warning << "failed to read pixels: " << SDL_GetError() << std::endl;
  }
  SDL_SetRenderTarget(renderer, 0);

  *(getlightrequest->color_ptr) = Color(pixel[2] / 255.0f,
                                        pixel[1] / 255.0f,
                                        pixel[0] / 255.0f);
}

/* EOF */
