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

SDLLightmap::SDLLightmap() :
  renderer(static_cast<SDLRenderer*>(Renderer::instance())->get_sdl_renderer()),
  width(),
  height(),
  LIGHTMAP_DIV()
{
  LIGHTMAP_DIV = 8;

  width = 800; //screen->w / LIGHTMAP_DIV;
  height = 600; //screen->h / LIGHTMAP_DIV;

  SDL_Renderer* renderer = static_cast<SDLRenderer*>(Renderer::instance())->get_sdl_renderer();
  texture = SDL_CreateTexture(renderer,
                              SDL_PIXELFORMAT_RGB888,
                              SDL_TEXTUREACCESS_TARGET,
                              width, height);
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
}

void
SDLLightmap::end_draw()
{
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
  //FIXME: support parameters request.alpha, request.angle, request.blend
  const Surface* surface = (const Surface*) request.request_data;
  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->get_texture());

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = sdltexture->get_image_width();
  dst_rect.h = sdltexture->get_image_height();

  SDL_SetTextureBlendMode(sdltexture->get_texture(), SDL_BLENDMODE_ADD);
  SDL_RenderCopy(renderer, sdltexture->get_texture(), NULL, &dst_rect);
}

void
SDLLightmap::draw_surface_part(const DrawingRequest& request)
{
  //FIXME: support parameters request.alpha, request.angle, request.blend
  const Surface* surface = (const Surface*) request.request_data;
  boost::shared_ptr<SDLTexture> sdltexture = boost::dynamic_pointer_cast<SDLTexture>(surface->get_texture());

  SDL_Rect dst_rect;
  dst_rect.x = request.pos.x;
  dst_rect.y = request.pos.y;
  dst_rect.w = sdltexture->get_image_width();
  dst_rect.h = sdltexture->get_image_height();

  SDL_SetTextureBlendMode(sdltexture->get_texture(), SDL_BLENDMODE_ADD);
  SDL_RenderCopy(renderer, sdltexture->get_texture(), NULL, &dst_rect);
}

void
SDLLightmap::draw_gradient(const DrawingRequest& request)
{
  log_info << "draw_gradient" << std::endl;
}

void
SDLLightmap::draw_filled_rect(const DrawingRequest& request)
{
  log_info << "draw_filled_rect" << std::endl;

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

  log_info << fillrectrequest->color.red << " " << fillrectrequest->color.green << std::endl;

  if((rect.w != 0) && (rect.h != 0))
  {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
  }
}

void
SDLLightmap::get_light(const DrawingRequest& request) const
{
#if OLD_SDL1
  const GetLightRequest* getlightrequest 
    = (GetLightRequest*) request.request_data;

  int x = (int) (request.pos.x * width / SCREEN_WIDTH);
  int y = (int) (request.pos.y * height / SCREEN_HEIGHT);
  int loc = y * width + x;
  *(getlightrequest->color_ptr) = Color(((float)red_channel[loc])/255, ((float)green_channel[loc])/255, ((float)blue_channel[loc])/255);
#endif
}

/* EOF */
