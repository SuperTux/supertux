//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/ttf_font_manager.hpp"

#include <SDL_ttf.h>
#include <sstream>
#include <iostream>

#include "video/sdl_surface_ptr.hpp"
#include "video/surface.hpp"
#include "video/ttf_font.hpp"
#include "video/video_system.hpp"

TTFFontManager::TTFFontManager() :
  m_cache()
{
}

SurfacePtr
TTFFontManager::create_surface(const TTFFont& font, const std::string& text)
{
  auto key = Key(font.get_ttf_font(), text);
  auto it = m_cache.find(key);
  if (it != m_cache.end())
  {
    return m_cache[key];
  }
  else
  {
    SDLSurfacePtr surface(TTF_RenderUTF8_Blended(font.get_ttf_font(),
                                                 text.c_str(),
                                                 Color::WHITE.to_sdl_color()));
    if (!surface)
    {
      std::ostringstream msg;
      msg << "Couldn't load image '" << text << "' :" << SDL_GetError();
      throw std::runtime_error(msg.str());
    }
    else
    {
      TexturePtr texture = VideoSystem::current()->new_texture(surface.get());
      SurfacePtr result(new Surface(texture));
      m_cache[key] = result;
      return result;
    }
  }
}

/* EOF */
