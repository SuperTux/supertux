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

#ifndef HEADER_SUPERTUX_BADGUY_MRBOMB_HPP
#define HEADER_SUPERTUX_BADGUY_MRBOMB_HPP

#include "badguy/walking_badguy.hpp"

class MrBomb final : public WalkingBadguy
{
public:
  MrBomb(const ReaderMapping& reader);

  virtual void kill_fall() override;
  virtual void ignite() override;
  virtual HitResponse collision(GameObject& object, const CollisionHit& hit) override;
  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;

  virtual void active_update(float dt_sec) override;

  virtual void grab(MovingObject& object, const Vector& pos, Direction dir) override;
  virtual bool is_portable() const override;

  virtual bool is_freezable() const override;
  static std::string class_name() { return "mrbomb"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Bomb"); }
  virtual std::string get_display_name() const override { return display_name(); }

protected:
  virtual bool collision_squished(GameObject& object) override;

private:
  MrBomb(const MrBomb&) = delete;
  MrBomb& operator=(const MrBomb&) = delete;
};

#endif

/* EOF */
