//  SuperTux
//  Copyright (C) 2026 e
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

#include "badguy/badguy.hpp"

class GranitoSentinel final : public BadGuy
{

public:
  GranitoSentinel(const ReaderMapping& reader);
  GranitoSentinel(const ReaderMapping& reader, int type);

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual bool collision_squished(MovingObject& object) override;
  virtual void active_update(float dt_sec) override;

  virtual void unfreeze(bool melt = true) override;
  virtual bool is_freezable() const override;
  static std::string class_name() { return "granito_sentinel"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Granito Sentinel"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return BadGuy::get_class_types().add(typeid(GranitoSentinel)); }

  virtual bool is_snipable() const override { return true; }
  virtual bool is_flammable() const override { return false; }

private:

  enum GranitoSentinelState {
    STANDING,
    CHARGING,
    JUMPING
  };

private:
  void set_state(GranitoSentinelState newState);
  bool should_jump();
  float find_parabola_point(float x_value);

private:
  Timer recover_timer;
  GranitoSentinelState state;

private:
  GranitoSentinel(const GranitoSentinel&) = delete;
  GranitoSentinel& operator=(const GranitoSentinel&) = delete;
};
