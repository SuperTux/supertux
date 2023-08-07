//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2023 Vankata453
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

#include <optional>
#include <sstream>

#include "sprite/sprite.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/string_util.hpp"

std::unique_ptr<SpriteData> SpriteManager::s_dummy_sprite_data = nullptr;

SpriteManager::SpriteManager() :
  m_sprites(),
  m_load_successful(false)
{
  if (!s_dummy_sprite_data)
    s_dummy_sprite_data.reset(new SpriteData());
}

SpritePtr
SpriteManager::create(const std::string& name)
{
  Sprites::iterator i = m_sprites.find(name);
  SpriteData* data;
  if (i == m_sprites.end())
  {
    // Try loading the sprite file.
    try
    {
      data = load(name);
    }
    catch (const std::exception& err)
    {
      log_warning << "Error loading sprite '" << name << "', using dummy texture: " << err.what() << std::endl;
      m_load_successful = false;
      return SpritePtr(new Sprite(*s_dummy_sprite_data)); // Return a dummy sprite.
    }
  }
  else
  {
    data = i->second.get();
  }

  m_load_successful = true;
  return SpritePtr(new Sprite(*data));
}

SpriteData*
SpriteManager::load(const std::string& filename)
{
  std::unique_ptr<SpriteData> sprite_data;

  if (StringUtil::has_suffix(filename, ".sprite"))
  {
    std::optional<ReaderDocument> doc;
    try
    {
      doc = ReaderDocument::from_file(filename);
    }
    catch (const std::exception& err)
    {
      std::ostringstream msg;
      msg << "Parse error when trying to load sprite '" << filename
          << "': " << err.what();
      throw std::runtime_error(msg.str());
    }
    auto root = doc->get_root();

    if (root.get_name() != "supertux-sprite")
    {
      std::ostringstream msg;
      msg << "'" << filename << "' is not a supertux-sprite file";
      throw std::runtime_error(msg.str());
    }
    else
    {
      sprite_data = std::make_unique<SpriteData>(root.get_mapping());
    }
  }
  else
  {
    sprite_data = std::make_unique<SpriteData>(filename);
  }

  m_sprites[filename] = std::move(sprite_data);
  return m_sprites[filename].get();
}

/* EOF */
