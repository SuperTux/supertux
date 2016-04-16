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

#include <SDL_ttf.h>
#include <SDL_video.h>

#include <config.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "util/currenton.hpp"
#include "video/color.hpp"
#include "video/glutil.hpp"
#include "video/texture_ptr.hpp"

class Texture;
class GLTexture;
class Rect;

class TextureManager : public Currenton<TextureManager>
{
public:
  TextureManager();
  ~TextureManager();

  TexturePtr get(const std::string& filename);
  TexturePtr get(const std::string& filename, const Rect& rect);
  TexturePtr get(TTF_Font* font, const std::string& text, const Color& color = Color::BLACK);

#ifdef HAVE_OPENGL
  void register_texture(GLTexture* texture);
  void remove_texture(GLTexture* texture);

  void save_textures();
  void reload_textures();
#endif

private:
  friend class Texture;

  typedef std::map<std::string, std::weak_ptr<Texture> > ImageTextures;
  ImageTextures m_image_textures;
  typedef std::map<std::string, std::shared_ptr<Texture> > FontTextures;
  FontTextures m_font_textures;

  typedef std::map<std::string, SDL_Surface*> Surfaces;
  Surfaces m_surfaces;

private:
  void reap_cache_entry(const std::string& filename);

  TexturePtr create_image_texture(const std::string& filename, const Rect& rect);

  /** on failure a dummy texture is returned and no exception is thrown */
  TexturePtr create_image_texture(const std::string& filename);

  /** throw an exception on error */
  TexturePtr create_image_texture_raw(const std::string& filename);
  TexturePtr create_image_texture_raw(const std::string& filename, const Rect& rect);

  TexturePtr create_text_texture(TTF_Font* font, const std::string& text, const Color& color);

  TexturePtr create_dummy_texture();

#ifdef HAVE_OPENGL
private:
  typedef std::set<GLTexture*> Textures;
  Textures m_textures;

  struct SavedTexture
  {
    GLTexture* texture;
    GLint width;
    GLint height;
    char* pixels;
    GLint border;

    GLint min_filter;
    GLint mag_filter;
    GLint wrap_s;
    GLint wrap_t;
  };
  std::vector<SavedTexture> m_saved_textures;

private:
  void save_texture(GLTexture* texture);
#endif
};

#endif

/* EOF */
