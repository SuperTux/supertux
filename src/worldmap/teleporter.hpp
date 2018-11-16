//  SuperTux - Teleporter Worldmap Tile
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

#ifndef HEADER_SUPERTUX_WORLDMAP_TELEPORTER_HPP
#define HEADER_SUPERTUX_WORLDMAP_TELEPORTER_HPP

#include <string>

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"

class ReaderMapping;

namespace worldmap {

class Teleporter final : public GameObject
{
public:
  Teleporter(const ReaderMapping& mapping);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  Vector get_pos() const { return m_pos; }

private:
  /** Position (in tiles, not pixels) */
  Vector m_pos;

public:
  /** Sprite to render, or 0 for no sprite */
  SpritePtr m_sprite;

  /** Worldmap filename (relative to data root) to teleport to. Leave empty to use current word */
  std::string m_worldmap;

  /** Spawnpoint to teleport to. Leave empty to use "main" or last one */
  std::string m_spawnpoint;

  /** true if this teleporter does not need to be activated, but teleports Tux as soon as it's touched */
  bool m_automatic;

  /** optional map message to display */
  std::string m_message;

private:
  Teleporter(const Teleporter&) = delete;
  Teleporter& operator=(const Teleporter&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
