//  SuperTux
//  Copyright (C) 2021 Daniel Ward <weluvgoatz@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_SHARD_HPP
#define HEADER_SUPERTUX_OBJECT_SHARD_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"

class Shard final : public MovingSprite
{
public:
  Shard(const ReaderMapping& reader);
  Shard(const Vector& pos, const Vector& velocity);

  virtual void update(float dt_sec) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual std::string get_class() const override { return "shard"; }
  virtual std::string get_display_name() const override { return _("Shard"); }

protected:
  Physic m_physic;

private:
  Timer m_stick_timer;

private:
  Shard(const Shard&) = delete;
  Shard& operator=(const Shard&) = delete;
};

#endif

/* EOF */
