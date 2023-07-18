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

#include "supertux/game_object.hpp"

#include "sprite/sprite_ptr.hpp"
#include "supertux/player_status.hpp"

class Controller;

namespace worldmap {

class SpecialTile;
class SpriteChange;
class WorldMap;

class Tux final : public GameObject
{
public:
  Tux(WorldMap* worldmap);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  virtual bool is_singleton() const override { return true; }

  void setup(); /**< called prior to first update */

  void set_direction(Direction dir);

  void set_ghost_mode(bool enabled);
  bool get_ghost_mode() const;

  bool is_moving() const { return m_moving; }
  Vector get_pos() const;
  Vector get_axis() const;
  Vector get_tile_pos() const { return m_tile_pos; }
  void set_pos(const Vector& pos) { m_tile_pos = pos / 32.f; }
  void set_tile_pos(const Vector& pos) { m_tile_pos = pos; }

  void process_special_tile(SpecialTile* special_tile);

private:
  void stop();
  std::string get_action_prefix_for_bonus(const BonusType& bonus) const;
  bool can_walk(int tile_data, Direction dir) const; /**< check if we can leave a tile (with given "tile_data") in direction "dir" */
  void update_input_direction(); /**< if controller was pressed, update input_direction */
  void try_start_walking(); /**< try starting to walk in input_direction */
  void try_continue_walking(float dt_sec); /**< try to continue walking in current direction */

  void change_sprite(SpriteChange* sc); /**< Uses the given sprite change */

public:
  Direction m_back_direction;

private:
  WorldMap* m_worldmap;
  SpritePtr m_sprite;
  Controller& m_controller;

  Direction m_input_direction;
  Direction m_direction;
  Vector m_tile_pos;
  /** Length by which tux is away from its current tile, length is in
      input_direction direction */
  float m_offset;
  bool m_moving;

  bool m_ghost_mode;

private:
  Tux(const Tux&) = delete;
  Tux& operator=(const Tux&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
