//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_WORLDMAP_TUX_HPP
#define HEADER_SUPERTUX_WORLDMAP_TUX_HPP

#include "worldmap/worldmap.hpp"

class Sprite;

namespace worldmap {

class SpecialTile;
class SpriteChange;
class WorldMap;

class Tux : public GameObject
{
public:
  Direction back_direction;
private:
  WorldMap* worldmap;
  SpritePtr sprite;
  Controller* controller;

  Direction input_direction;
  Direction direction;
  Vector tile_pos;
  /** Length by which tux is away from its current tile, length is in
      input_direction direction */
  float offset;
  bool  moving;

  bool ghost_mode;

private:
  void stop();
  std::string get_action_prefix_for_bonus(const BonusType& bonus) const;
  bool canWalk(int tile_data, Direction dir) const; /**< check if we can leave a tile (with given "tile_data") in direction "dir" */
  void updateInputDirection(); /**< if controller was pressed, update input_direction */
  void tryStartWalking(); /**< try starting to walk in input_direction */
  void tryContinueWalking(float elapsed_time); /**< try to continue walking in current direction */

  void ChangeSprite(SpriteChange* sc); /**< Uses the given sprite change */

public:
  Tux(WorldMap* worldmap_);
  ~Tux();

  void setup(); /**< called prior to first update */
  void draw(DrawingContext& context);
  void update(float elapsed_time);

  void set_direction(Direction dir);

  void set_ghost_mode(bool enabled);
  bool get_ghost_mode() const;

  bool is_moving() const { return moving; }
  Vector get_pos() const;
  Vector get_tile_pos() const { return tile_pos; }
  void  set_tile_pos(Vector p) { tile_pos = p; }

  void process_special_tile(SpecialTile* special_tile);

private:
  Tux(const Tux&);
  Tux& operator=(const Tux&);
};

} // namespace worldmap

#endif

/* EOF */
