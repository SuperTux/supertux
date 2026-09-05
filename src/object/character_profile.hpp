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

#include <string>

enum class CharacterType {
  TUX   = 0,
  PENNY = 1,
  LARRY = 2
};

/**
 * Defines all gameplay and visual attributes for a character.
 * Adding new characters or changing gameplay only requires
 * adding an entry to CharacterRegistry
 */
struct CharacterProfile
{
  std::string id;
  std::string name;

  std::string sprite_path;

  float max_speed;
  float max_speed_multiplier;
  float acceleration;
  float deceleration;
  float jump_force;
  float fall_acceleration;

  float hitbox_width;
  float hitbox_height;

  // Default constructor for Tux (fallback)
  CharacterProfile() :
    id("tux"),
    name("Tux"),
    sprite_path("tux.sprite"),
    max_speed(230.0f),
    max_speed_multiplier(1.0f),
    acceleration(200.0f),
    deceleration(200.0f),
    jump_force(8.4f),
    fall_acceleration(20.0f),
    hitbox_width(31.8f),
    hitbox_height(63.8f)
  {}
};
