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

#ifndef HEADER_SUPERTUX_SCRIPTING_TILEMAP_HPP
#define HEADER_SUPERTUX_SCRIPTING_TILEMAP_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class TileMap;
#endif

namespace scripting {

/**
 * @summary A ""TileMap"" that was given a name can be controlled by scripts.
            The tilemap can be moved by specifying a path for it.
 * @instances A ""TileMap"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class TileMap final
#ifndef SCRIPTING_API
  : public GameObject<::TileMap>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;
private:
  TileMap(const TileMap&) = delete;
  TileMap& operator=(const TileMap&) = delete;
#endif

public:
  /**
   * Moves the tilemap along a path until at given node, then stops.
   * @param int $node_no
   */
  void goto_node(int node_no);

  /**
   * Jumps instantly to the given node.
   * @param int $node_no
   */
  void set_node(int node_no);

  /**
   * Starts moving the tilemap.
   */
  void start_moving();

  /**
   * Stops the tilemap at the next node.
   */
  void stop_moving();

  /**
   * Returns the ID of the tile at the given coordinates or 0 if out of bounds.
     The origin is at the top left.
   * @param int $x
   * @param int $y
   */
  int get_tile_id(int x, int y) const;

  /**
   * Returns the ID of the tile at the given position (in-world coordinates).
   * @param float $x
   * @param float $y
   */
  int get_tile_id_at(float x, float y) const;

  /**
   * Changes the tile at the given coordinates to ""newtile"".
     The origin is at the top left.
   * @param int $x
   * @param int $y
   * @param int $newtile
   */
  void change(int x, int y, int newtile);

  /**
   * Changes the tile at the given position (in-world coordinates) to ""newtile"".
   * @param float $x
   * @param float $y
   * @param int $newtile
   */
  void change_at(float x, float y, int newtile);

  /**
   * Starts fading the tilemap to the opacity given by ""alpha"".
   * Destination opacity will be reached after ""time"" seconds. Also influences solidity.
   * @param float $alpha
   * @param float $time
   */
  void fade(float alpha, float time);

  /**
   * Starts fading the tilemap to tint given by RGBA.
   * Destination opacity will be reached after ""time"" seconds. Doesn't influence solidity.
   * @param float $time
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void tint_fade(float time, float red, float green, float blue, float alpha);

  /**
   * Instantly switches the tilemap's opacity to ""alpha"". Also influences solidity.
   * @param float $alpha
   */
  void set_alpha(float alpha);

  /**
   * Returns the tilemap's opacity.${SRG_TABLENEWPARAGRAPH}
     Note that while the tilemap is fading in or out, this will return the current alpha value, not the target alpha.
   */
  float get_alpha() const;

  /**
   * Switches the tilemap's real solidity to ""solid"".${SRG_TABLENEWPARAGRAPH}
     Note that the effective solidity is also influenced by the alpha of the tilemap.
   * @param bool $solid
   */
  void set_solid(bool solid);
};

} // namespace scripting

#endif

/* EOF */
