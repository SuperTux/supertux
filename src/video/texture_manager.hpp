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

#ifndef HEADER_SUPERTUX_VIDEO_TEXTURE_MANAGER_HPP
#define HEADER_SUPERTUX_VIDEO_TEXTURE_MANAGER_HPP

#include <config.h>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "util/currenton.hpp"
#include "video/glutil.hpp"
#include "video/texture_ptr.hpp"

class Texture;
class GLTexture;
class Rect;
struct SDL_Surface;

class TextureManager : public Currenton<TextureManager>
{
public:
  friend class Texture;

public:
  TextureManager();
  ~TextureManager();

  TexturePtr get(const std::string& filename);
  TexturePtr get(const std::string& filename, const Rect& rect);

private:
  void reap_cache_entry(const std::string& filename);

  TexturePtr create_image_texture(const std::string& filename, const Rect& rect);

  /** on failure a dummy texture is returned and no exception is thrown */
  TexturePtr create_image_texture(const std::string& filename);

  /** throw an exception on error */
  TexturePtr create_image_texture_raw(const std::string& filename);
  TexturePtr create_image_texture_raw(const std::string& filename, const Rect& rect);

  TexturePtr create_dummy_texture();

private:
  std::map<std::string, std::weak_ptr<Texture> > m_image_textures;
  std::map<std::string, SDL_Surface*> m_surfaces;
};

#endif

/* EOF */
