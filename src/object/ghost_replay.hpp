//  SuperTux
//  Copyright (C) 2026
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

#include "supertux/game_object.hpp"

#include <string>
#include <vector>

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "worldmap/level_tile.hpp"

class DrawingContext;

/** Cosmetic Tux ghost that replays the player's best run for the current
    level. Does not interact with physics, enemies, or any world objects. */
class GhostReplay final : public GameObject
{
public:
  explicit GhostReplay(std::vector<worldmap::LevelTile::GhostRunPoint> path);

  virtual GameObjectClasses get_class_types() const override
  {
    return GameObject::get_class_types().add(typeid(GhostReplay));
  }

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual bool is_saveable() const override { return false; }

private:
  Vector sample_position(float time) const;
  const std::string& sample_action(float time) const;
  void apply_action(const std::string& action);

  std::vector<worldmap::LevelTile::GhostRunPoint> m_path;
  float m_time;
  Vector m_position;
  SpritePtr m_sprite;
  std::string m_current_action;
};
