//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_GAME_OBJECT_MANAGER_HPP
#define HEADER_SUPERTUX_SCRIPTING_GAME_OBJECT_MANAGER_HPP

#ifndef SCRIPTING_API
#include <string>
class GameObjectManager;
#endif

namespace scripting {

/**
 * @summary This class provides basic controlling functions for a sector. Applies for both worldmap and in-level sectors.
 * @instances For in-level sectors, an instance under ""sector.settings"" is available from scripts and the console.${SRG_NEWPARAGRAPH}
              For worldmap sectors, such instance is available under ""worldmap.settings"".
 */
class GameObjectManager
{
#ifndef SCRIPTING_API
private:
  ::GameObjectManager* m_gom_parent;

public:
  GameObjectManager(::GameObjectManager* parent);

private:
  GameObjectManager(const GameObjectManager&) = delete;
  GameObjectManager& operator=(const GameObjectManager&) = delete;
#endif

public:
  /**
   * Sets the sector's ambient light to the specified color.
   * @param float $red
   * @param float $green
   * @param float $blue
   */
  void set_ambient_light(float red, float green, float blue);
  /**
   * Fades to a specified ambient light color in ""fadetime"" seconds.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $fadetime
   */
  void fade_to_ambient_light(float red, float green, float blue, float fadetime);
  /**
   * Returns the red channel of the ambient light color.
   */
  float get_ambient_red() const;
  /**
   * Returns the green channel of the ambient light color.
   */
  float get_ambient_green() const;
  /**
   * Returns the blue channel of the ambient light color.
   */
  float get_ambient_blue() const;

  /**
   * Sets the sector's music.
   * @param string $music Full filename, relative to the "music" folder.
   */
  void set_music(const std::string& music);

  /**
   * Adds a ""MovingObject"" to the manager.
     Note: If adding objects to a worldmap sector, ""posX"" and ""posY"" have to be tile positions (sector position / 32).
   * @param string $class_name GameObject's class.
   * @param string $name Name of the created object.
   * @param int $posX X position inside the current sector.
   * @param int $posY Y position inside the current sector.
   * @param string $direction Direction.
   * @param string $data Additional data in S-Expression format (check object definitions in level files).
   */
  void add_object(const std::string& class_name, const std::string& name,
                  int posX, int posY, const std::string& direction,
                  const std::string& data);
};

} // namespace scripting

#endif

/* EOF */
