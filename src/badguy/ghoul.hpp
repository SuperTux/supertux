//  SuperTux
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

class Ghoul final : public BadGuy
{
public:
  Ghoul(const ReaderMapping& reader);
  static std::string class_name() { return "ghoul"; }
  static std::string display_name() { return _("Ghoul"); }
  std::string get_class_name() const override { return class_name(); }
  std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return BadGuy::get_class_types().add(typeid(Ghoul)); }
  virtual bool is_snipable() const override { return true; }
  virtual bool is_flammable() const override { return false; }

  virtual void active_update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual void collision_tile(uint32_t tile_attributes) override;
  virtual ObjectSettings get_settings() override;

protected:
  bool collision_squished(MovingObject& object) override;
  virtual void kill_fall() override;

private:
  float m_track_range;
  Vector m_home_pos;
  Timer m_respawn_timer;

private:
  enum GhoulState {
    ROAMING_DOWN,
    ROAMING_ACCEL1,
    ROAMING_ACCEL2,
    ROAMING_UP,
    CHASING_DOWN,
    CHASING_ACCEL1,
    CHASING_ACCEL2,
    CHASING_UP,
    STUNNED,
    INVISIBLE,
    RECOVERING,
  };

  GhoulState m_state;
  void set_state(GhoulState new_state);
  void update_speed(const Vector& dist);
  void horizontal_thrust();
  void start_roaming_decel();
  void roaming_decel_check();
  Vector to_target() const;

private:
  Ghoul(const Ghoul&) = delete;
  Ghoul& operator=(const Ghoul&) = delete;
};
