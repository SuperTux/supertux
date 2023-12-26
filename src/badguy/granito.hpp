//  SuperTux
//  Copyright (C) 2023 MatusGuy
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

#ifndef HEADER_SUPERTUX_BADGUY_GRANITO_HPP
#define HEADER_SUPERTUX_BADGUY_GRANITO_HPP

#include "badguy/walking_badguy.hpp"

/** Interactable friendly NPC */
class Granito : public WalkingBadguy
{
public:
  Granito(const ReaderMapping& reader,
          const std::string& sprite_name = "images/creatures/granito/granito.sprite",
          int layer = LAYER_OBJECTS);

  virtual void active_update(float dt_sec) override;

  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "granito"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Granito"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual bool is_snipable() const override { return false; }
  virtual bool is_freezable() const override { return false; }
  virtual bool is_flammable() const override { return false; }

  virtual GameObjectTypes get_types() const override;
  virtual void after_editor_set() override;

protected:
  virtual void initialize() override;
  virtual void update_hitbox() override;

  void activate() override;

protected:
  enum Type { DEFAULT, STAND, WALK, SIT };
  enum State
  {
    STATE_STAND,
    STATE_WALK,
    STATE_WAVE,
    STATE_LOOKUP,
    STATE_JUMPING
  };

protected:
  virtual bool try_wave();
  void wave();

  virtual bool try_jump();
  void jump();

  void restore_original_state();

private:
  Timer m_walk_interval;
  State m_state;
  State m_original_state;

  bool m_has_waved;
  bool m_stepped_on; /** true if tux was on top of granito last frame */
  bool m_airborne; /** unfortunately on_ground() sucks */

private:
  Granito(const Granito&) = delete;
  Granito& operator=(const Granito&) = delete;
};

#endif

/* EOF */
