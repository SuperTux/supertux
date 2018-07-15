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

#include "video/sdl/sdl_texture.hpp"

#include <SDL.h>

#include "video/sdl/sdl_renderer.hpp"
#include "video/video_system.hpp"

SDLTexture::SDLTexture(SDL_Surface* image) :
  m_texture(),
  m_width(),
  m_height()
{
  m_texture = SDL_CreateTextureFromSurface(static_cast<SDLRenderer&>(VideoSystem::current()->get_renderer()).get_sdl_renderer(),
                                           image);
  if (!m_texture)
  {
    std::ostringstream msg;
    msg << "couldn't create texture: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  m_width = image->w;
  m_height = image->h;
}

SDLTexture::~SDLTexture()
{
  SDL_DestroyTexture(m_texture);
}

/* EOF */
