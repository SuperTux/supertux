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

#ifndef HEADER_SUPERTUX_VIDEO_SDL_SDL_VIDEO_SYSTEM_HPP
#define HEADER_SUPERTUX_VIDEO_SDL_SDL_VIDEO_SYSTEM_HPP

#include <memory>
#include <SDL.h>

#include "video/video_system.hpp"

class TextureManager;

class SDLVideoSystem : public VideoSystem
{
private:
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<Lightmap> m_lightmap;
  std::unique_ptr<TextureManager> m_texture_manager;

public:
  SDLVideoSystem();

  Renderer& get_renderer();
  Lightmap& get_lightmap();
  TexturePtr new_texture(SDL_Surface *image) override;
  SurfaceData* new_surface_data(const Surface& surface) override;
  void free_surface_data(SurfaceData* surface_data) override;

private:
  SDLVideoSystem(const SDLVideoSystem&) = delete;
  SDLVideoSystem& operator=(const SDLVideoSystem&) = delete;
};

#endif

/* EOF */
