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

CharacterRegistry::CharacterRegistry() :
  m_default_character_id("tux")
{
  initialize_profiles();
}

void CharacterRegistry::initialize_profiles()
{
  const struct {
    const char* id;
    const char* name;
    const char* sprite;
    float max_speed;
    float multiplier;
    float accel;
    float decel;
    float jump;
    float fall;
    float width;
    float height;
  } characters[] = {
    {"tux", "Tux", "tux.sprite", 230.0f, 1.0f, 200.0f, 200.0f, 8.4f, 20.0f, 31.8f, 63.8f},
    {"penny", "Penny", "penny.sprite", 230.0f, 1.0f, 200.0f, 200.0f, 8.4f, 20.0f, 31.8f, 63.8f},
    {"larry", "Larry", "larry.sprite", 230.0f, 1.0f, 200.0f, 200.0f, 8.4f, 20.0f, 31.8f, 63.8f}
  };

  for (const auto& ch : characters) {
    CharacterProfile profile;
    profile.id = ch.id;
    profile.name = ch.name;
    profile.sprite_path = ch.sprite;
    profile.max_speed = ch.max_speed;
    profile.max_speed_multiplier = ch.multiplier;
    profile.acceleration = ch.accel;
    profile.deceleration = ch.decel;
    profile.jump_force = ch.jump;
    profile.fall_acceleration = ch.fall;
    profile.hitbox_width = ch.width;
    profile.hitbox_height = ch.height;
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

