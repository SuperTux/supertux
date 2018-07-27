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

#include "video/sdl/sdl_lightmap.hpp"

#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/drawing_request.hpp"
#include "video/sdl/sdl_renderer.hpp"
#include "video/sdl/sdl_painter.hpp"
#include "video/video_system.hpp"

SDLLightmap::SDLLightmap(SDL_Renderer* renderer) :
  m_renderer(renderer),
  m_texture(),
  m_width(),
  m_height(),
  m_LIGHTMAP_DIV(),
  m_cliprect()
{
  //m_LIGHTMAP_DIV = 5;
  m_LIGHTMAP_DIV = 1;

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
SDLLightmap::start_draw()
{
  SDL_SetRenderTarget(m_renderer, m_texture);
  SDL_RenderSetScale(m_renderer, 1.0f / m_LIGHTMAP_DIV, 1.0f / m_LIGHTMAP_DIV);
}

void
SDLLightmap::end_draw()
{
  SDL_RenderSetScale(m_renderer, 1.0f, 1.0f);
  SDL_SetRenderTarget(m_renderer, NULL);
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
SDLLightmap::draw_inverse_ellipse(const DrawingRequest& request)
{
  SDLPainter::draw_inverse_ellipse(m_renderer, request);
}

void
SDLLightmap::draw_line(const DrawingRequest& request)
{
  SDLPainter::draw_line(m_renderer, request);
}

void
SDLLightmap::draw_triangle(const DrawingRequest& request)
{
  SDLPainter::draw_triangle(m_renderer, request);
}

void
SDLLightmap::clear(const Color& color)
{
  SDL_SetRenderDrawColor(m_renderer, color.r8(), color.g8(), color.b8(), color.a8());

  if (m_cliprect)
  {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    SDL_RenderFillRect(m_renderer, &*m_cliprect);
  }
  else
  {
    // This ignores the cliprect:
    SDL_RenderClear(m_renderer);
  }
}

void
SDLLightmap::set_clip_rect(const Rect& rect)
{
  m_cliprect = SDL_Rect{ rect.left,
                         rect.top,
                         rect.get_width(),
                         rect.get_height() };

  int ret = SDL_RenderSetClipRect(m_renderer, &*m_cliprect);
  if (ret < 0)
  {
    log_warning << "SDLLightmap::set_clip_rect(): SDL_RenderSetClipRect() failed: " << SDL_GetError() << std::endl;
  }
}

void
SDLLightmap::clear_clip_rect()
{
  m_cliprect.reset();

  int ret = SDL_RenderSetClipRect(m_renderer, nullptr);
  if (ret < 0)
  {
    log_warning << "SDLLightmap::clear_clip_rect(): SDL_RenderSetClipRect() failed: " << SDL_GetError() << std::endl;
  }
}

void
SDLLightmap::get_light(const DrawingRequest& request) const
{
  const auto getlightrequest
    = static_cast<GetLightRequest*>(request.request_data);

  SDL_Rect rect;
  rect.x = static_cast<int>(request.pos.x / m_LIGHTMAP_DIV);
  rect.y = static_cast<int>(request.pos.y / m_LIGHTMAP_DIV);
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

  *(getlightrequest->color_ptr) = Color::from_rgb888(pixel[2], pixel[1], pixel[0]);
}

void
SDLLightmap::render()
{
  SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_MOD);

  SDL_Rect dst_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.w = m_width;
  dst_rect.h = m_height;

  SDL_RenderCopy(m_renderer, m_texture, NULL, &dst_rect);
}

/* EOF */
