//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_BADGUY_BADGUYNOSCRIPT_HPP
#define HEADER_SUPERTUX_BADGUY_BADGUYNOSCRIPT_HPP

#include "badguy/badguy.hpp"
#include "scripting/badguy.hpp"

/** Same as BadGuy, but Badguys that don't implement custom scripting should inherit from this instead.
 *  Does your badguy need to inherit a special ExposedObject<>? If yes, inherit BadGuy. Else, inherit BadGuyNoScript.
 */
class BadGuyNoScript : public BadGuy,
                       public ExposedObject<BadGuy, scripting::BadGuy>
{
public:
  BadGuyNoScript(const Vector& pos, const std::string& sprite_name, int layer = LAYER_OBJECTS,
                 const std::string& light_sprite_name = "images/objects/lightmap_light/lightmap_light-medium.sprite");
  BadGuyNoScript(const Vector& pos, Direction direction, const std::string& sprite_name, int layer = LAYER_OBJECTS,
                 const std::string& light_sprite_name = "images/objects/lightmap_light/lightmap_light-medium.sprite");
  BadGuyNoScript(const ReaderMapping& reader, const std::string& sprite_name, int layer = LAYER_OBJECTS,
                 const std::string& light_sprite_name = "images/objects/lightmap_light/lightmap_light-medium.sprite");
};

#endif

/* EOF */
