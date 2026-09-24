//  SuperTux
//  Copyright (C) 2024
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

#include "object/character_registry.hpp"
#include "object/character_profile.hpp"
#include "util/log.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

#include <stdexcept>

CharacterRegistry::CharacterRegistry() :
  m_default_character_id("tux")
{
  load_profiles("characters.stlst");
}

void CharacterRegistry::load_profiles(const std::string& filename)
{
  auto doc = ReaderDocument::from_file(filename);
  auto root = doc.get_root();
  if (root.get_name() != "supertux-playerlist")
  {
    throw std::runtime_error("File is not a supertux-playerlist file: " + filename);
  }

  auto collection = root.get_collection();
  for (const auto& obj : collection.get_objects())
  {
    if (obj.get_name() != "player")
    {
      log_warning << "Unknown token '" << obj.get_name() << "' in player list" << std::endl;
      continue;
    }

    CharacterProfile profile;
    auto mapping = obj.get_mapping();
    mapping.get("id", profile.id);
    mapping.get("name", profile.name);
    mapping.get("sprite", profile.sprite_path);
    mapping.get("max-speed", profile.max_speed);
    mapping.get("max-speed-multiplier", profile.max_speed_multiplier);
    mapping.get("acceleration", profile.acceleration);
    mapping.get("deceleration", profile.deceleration);
    mapping.get("jump-force", profile.jump_force);
    mapping.get("fall-acceleration", profile.fall_acceleration);
    mapping.get("hitbox-width", profile.hitbox_width);
    mapping.get("hitbox-height", profile.hitbox_height);

    if (profile.id.empty())
    {
      log_warning << "Skipping player entry with empty id in " << filename << std::endl;
      continue;
    }

    m_profiles[profile.id] = profile;
  }
}

const CharacterProfile& CharacterRegistry::get_profile(const std::string& character_id) const
{
  auto it = m_profiles.find(character_id);
  if (it != m_profiles.end())
  {
    return it->second;
  }

  return get_default_character();
}

std::vector<std::string> CharacterRegistry::get_character_ids() const
{
  std::vector<std::string> ids;
  for (const auto& pair : m_profiles)
  {
    ids.push_back(pair.first);
  }
  return ids;
}

bool CharacterRegistry::has_character(const std::string& character_id) const
{
  return m_profiles.find(character_id) != m_profiles.end();
}

const CharacterProfile& CharacterRegistry::get_default_character() const
{
  return m_profiles.at(m_default_character_id);
}

CharacterType CharacterRegistry::string_to_character(const std::string& id)
{
  if (id == "penny") return CharacterType::PENNY;
  if (id == "larry") return CharacterType::LARRY;
  return CharacterType::TUX;
}

std::string CharacterRegistry::character_to_string(CharacterType type)
{
  switch(type) {
    case CharacterType::PENNY: return "penny";
    case CharacterType::LARRY: return "larry";
    case CharacterType::TUX: return "tux";
  }
  return "tux";
}

