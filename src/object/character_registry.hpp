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

#pragma once

#include "object/character_profile.hpp"
#include "util/currenton.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * Central registry for all character profiles.
 */
class CharacterRegistry final : public Currenton<CharacterRegistry>
{
public:
  CharacterRegistry();

  /**
   * Get a character profile by ID
   * @param character_id The character ID (e.g., "tux", "penny", "larry")
   * @return Reference to the CharacterProfile (always valid, returns fallback if not found)
   */
  const CharacterProfile& get_profile(const std::string& character_id) const;

  /**
   * Get all registered character IDs
   * @return Vector of character ID strings
   */
  std::vector<std::string> get_character_ids() const;

  /**
   * Check if a character exists
   * @param character_id The character ID to check
   * @return true if character is registered, false otherwise
   */
  bool has_character(const std::string& character_id) const;

  /**
   * Get the default/fallback character (Tux)
   * @return Reference to Tux's CharacterProfile
   */
  const CharacterProfile& get_default_character() const;

  static CharacterType string_to_character(const std::string& id);
  static std::string character_to_string(CharacterType type);

private:
  void load_profiles(const std::string& filename);

private:
  std::map<std::string, CharacterProfile> m_profiles;
  std::string m_default_character_id;
};
