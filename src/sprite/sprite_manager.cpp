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

#include "sprite/sprite_manager.hpp"

#include <sstream>

#include "sprite/sprite.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/string_util.hpp"
#include "video/texture_manager.hpp"

SpriteManager::SpriteManager() :
  sprites()
{
}

SpritePtr
SpriteManager::create(const std::string& name)
{
  Sprites::iterator i = sprites.find(name);
  SpriteData* data;
  if (i == sprites.end())
  {
    // Try loading the sprite file
    try
    {
      data = load(name);
      if (!data)
      {
        log_warning << "Sprite '" << name << "' not found. Using dummy texture." << std::endl;
        return create(TextureManager::s_dummy_texture); // Load the missing dummy texture
      }
    }
    catch (std::exception& err)
    {
      log_warning << "Error loading sprite '" << name << "', using dummy texture: " << err.what() << std::endl;
      return create(TextureManager::s_dummy_texture); // Load the missing dummy texture
    }
  }
  else
  {
    data = i->second.get();
  }

  return SpritePtr(new Sprite(*data));
}

SpriteData*
SpriteManager::load(const std::string& filename)
{
  ReaderDocument doc = [filename](){
    try {
      if (StringUtil::has_suffix(filename, ".sprite")) {
        return ReaderDocument::from_file(filename);
      } else {
        std::stringstream text;
        text << "(supertux-sprite (action "
             << "(name \"default\") "
             << "(images \"" << FileSystem::basename(filename) << "\")))";
        return ReaderDocument::from_stream(text, filename);
      }
    } catch(const std::exception& e) {
      std::ostringstream msg;
      msg << "Parse error when trying to load sprite '" << filename
      << "': " << e.what() << "\n";
      throw std::runtime_error(msg.str());
    }
  }();

  auto root = doc.get_root();

  if (root.get_name() != "supertux-sprite") {
    std::ostringstream msg;
    msg << "'" << filename << "' is not a supertux-sprite file";
    throw std::runtime_error(msg.str());
  } else {
    auto data = std::make_unique<SpriteData>(root.get_mapping());
    sprites[filename] = std::move(data);

    return sprites[filename].get();
  }
}

/* EOF */
