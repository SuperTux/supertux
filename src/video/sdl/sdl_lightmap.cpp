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
  m_renderer(static_cast<SDLRenderer&>(VideoSystem::current()->get_renderer()).get_sdl_renderer()),
  m_texture(),
  m_width(),
  m_height(),
  m_LIGHTMAP_DIV()
{
  m_LIGHTMAP_DIV = 8;

  m_width = SCREEN_WIDTH;
  m_height = SCREEN_HEIGHT;

  m_texture = SDL_CreateTexture(m_renderer,
                                SDL_PIXELFORMAT_RGB888,
                                SDL_TEXTUREACCESS_TARGET,
                                m_width / m_LIGHTMAP_DIV,
                                m_height / m_LIGHTMAP_DIV);
  if (!m_texture)
  {
    std::stringstream msg;
    msg << "Couldn't create lightmap texture: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
}

SDLLightmap::~SDLLightmap()
{
  SDL_DestroyTexture(m_texture);
}

void
SDLLightmap::start_draw(const Color &ambient_color)
{
  SDL_SetRenderTarget(m_renderer, m_texture);

  Uint8 r = static_cast<Uint8>(ambient_color.red * 255);
  Uint8 g = static_cast<Uint8>(ambient_color.green * 255);
  Uint8 b = static_cast<Uint8>(ambient_color.blue * 255);

  SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
  SDL_RenderClear(m_renderer);
  SDL_RenderSetScale(m_renderer, 1.0f / m_LIGHTMAP_DIV, 1.0f / m_LIGHTMAP_DIV);
}

void
SDLLightmap::end_draw()
{
  SDL_RenderSetScale(m_renderer, 1.0f, 1.0f);
  SDL_SetRenderTarget(m_renderer, NULL);
}

void
SDLLightmap::do_draw()
{
  SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_MOD);

  SDL_Rect dst_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.w = m_width;
  dst_rect.h = m_height;

  SDL_RenderCopy(m_renderer, m_texture, NULL, &dst_rect);
}

void
SDLLightmap::draw_surface(const DrawingRequest& request)
{
  SDLPainter::draw_surface(m_renderer, request);
}

void
SDLLightmap::draw_surface_part(const DrawingRequest& request)
{
  SDLPainter::draw_surface_part(m_renderer, request);
}

void
SDLLightmap::draw_gradient(const DrawingRequest& request)
{
  SDLPainter::draw_gradient(m_renderer, request);
}

void
SDLLightmap::draw_filled_rect(const DrawingRequest& request)
{
  SDLPainter::draw_filled_rect(m_renderer, request);
}

void
SDLLightmap::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest
    = static_cast<GetLightRequest*>(request.request_data);

  SDL_Rect rect;
  rect.x = static_cast<int>(request.pos.x * m_width / SCREEN_WIDTH);
  rect.y = static_cast<int>(request.pos.y * m_height / SCREEN_HEIGHT);
  rect.w = 1;
  rect.h = 1;

  SDL_SetRenderTarget(m_renderer, m_texture);
  Uint8 pixel[4];
  int ret = SDL_RenderReadPixels(m_renderer, &rect,
                                 SDL_PIXELFORMAT_RGB888,
                                 pixel,
                                 1);
  if (ret != 0)
  {
    log_warning << "failed to read pixels: " << SDL_GetError() << std::endl;
  }
  SDL_SetRenderTarget(m_renderer, 0);

  *(getlightrequest->color_ptr) = Color(pixel[2] / 255.0f,
                                        pixel[1] / 255.0f,
                                        pixel[0] / 255.0f);
}

/* EOF */
