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

#ifndef HEADER_SUPERTUX_BADGUY_GHOUL_HPP
#define HEADER_SUPERTUX_BADGUY_GHOUL_HPP

#include "badguy/badguy.hpp"

class Ghoul final : public BadGuy
{
public:
  Ghoul(const ReaderMapping& reader);
  static std::string class_name() { return "ghoul"; }
  static std::string display_name() { return _("Ghoul"); }
  std::string get_class_name() const override { return class_name(); }
  std::string get_display_name() const override { return display_name(); }
  virtual bool is_snipable() const override { return true; }
  virtual bool is_freezable() const override { return true; }
  virtual std::string get_overlay_size() const override { return "3x3"; }

  void active_update(float dt_sec) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual ObjectSettings get_settings() override;

  virtual void freeze() override;
  virtual void unfreeze(bool melt = true) override;

protected:
  bool collision_squished(GameObject& object) override;
  
private:
  float m_speed;
  float m_track_range;
  float m_speed_modifier;
  Vector m_chase_dir;

private:
  enum SpriteState {
    NORMAL,
    FAST
  };

  SpriteState m_sprite_state;
  
private:
  Ghoul(const Ghoul&) = delete;
  Ghoul& operator=(const Ghoul&) = delete;
};

#endif

/* EOF */
