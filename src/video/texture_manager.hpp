//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __IMAGE_TEXTURE_MANAGER_HPP__
#define __IMAGE_TEXTURE_MANAGER_HPP__

#include <config.h>

#include "glutil.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>

class Texture;
namespace GL { class Texture; }

class TextureManager
{
public:
  TextureManager();
  ~TextureManager();

  Texture* get(const std::string& filename);

#ifdef HAVE_OPENGL
  void register_texture(GL::Texture* texture);
  void remove_texture(GL::Texture* texture);

  void save_textures();
  void reload_textures();
#endif

private:
  friend class Texture;
  void release(Texture* texture);

  typedef std::map<std::string, Texture*> ImageTextures;
  ImageTextures image_textures;

  Texture* create_image_texture(const std::string& filename);

#ifdef HAVE_OPENGL
  typedef std::set<GL::Texture*> Textures;
  Textures textures;

  struct SavedTexture
  {
    GL::Texture* texture;
    GLint width;
    GLint height;
    char* pixels;
    GLint border;

    GLint min_filter;
    GLint mag_filter;
    GLint wrap_s;
    GLint wrap_t;
  };
  std::vector<SavedTexture> saved_textures;

  void save_texture(GL::Texture* texture);
#endif
};

extern TextureManager* texture_manager;

#endif
