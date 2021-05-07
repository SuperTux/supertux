//  SuperTux
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

#ifndef HEADER_SUPERTUX_WORLDMAP_SPECIAL_TILE_HPP
#define HEADER_SUPERTUX_WORLDMAP_SPECIAL_TILE_HPP

#include <string>

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"

class ReaderMapping;

namespace worldmap {

class SpecialTile final : public GameObject
{
public:
  SpecialTile(const ReaderMapping& mapping);
  ~SpecialTile() override;

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  Vector get_pos() const { return m_pos; }
  std::string get_map_message() const { return m_map_message; }
  bool is_passive_message() const { return m_passive_message; }
  std::string get_script() const { return m_script; }

  bool get_apply_action_north() const { return m_apply_action_north; }
  bool get_apply_action_east() const  { return m_apply_action_east; }
  bool get_apply_action_south() const { return m_apply_action_south; }
  bool get_apply_action_west() const { return m_apply_action_west; }

private:
  Vector m_pos;

  /** Sprite to render instead of guessing what image to draw */
  SpritePtr m_sprite;

  /** Message to show in the Map */
  std::string m_map_message;
  bool m_passive_message;

  /** Script to execute when tile is touched */
  std::string m_script;

  /** Hide special tile */
  bool m_invisible;

  /** Only applies actions (ie. passive messages) when going to that direction */
  bool m_apply_action_north;
  bool m_apply_action_east;
  bool m_apply_action_south;
  bool m_apply_action_west;

private:
  SpecialTile(const SpecialTile&) = delete;
  SpecialTile& operator=(const SpecialTile&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
