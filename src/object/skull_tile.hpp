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

#ifndef HEADER_SUPERTUX_OBJECT_SKULL_TILE_HPP
#define HEADER_SUPERTUX_OBJECT_SKULL_TILE_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"
#include "util/fade_helper.hpp"

/** A tile that starts falling down if tux stands to long on it */
class SkullTile final : public MovingSprite
{
public:
  SkullTile(const ReaderMapping& mapping);

  static std::string class_name() { return "skull_tile"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Skull Tile"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void on_flip(float height) override;

private:
  Physic physic;
  Timer timer;
  bool hit;
  bool falling;

  Timer m_revive_timer;
  std::unique_ptr<FadeHelper> m_respawn;
  float m_alpha;
  Vector m_original_pos;
};

#endif

/* EOF */
