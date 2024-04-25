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

class GranitoBig;

/** Interactable friendly NPC */
class Granito : public WalkingBadguy
{
public:
  static void register_class(ssq::VM& vm);

public:
  Granito(const ReaderMapping& reader,
          const std::string& sprite_name = "images/creatures/granito/granito.sprite",
          int layer = LAYER_OBJECTS);

  /** \addtogroup GameObject
      @{ */
  virtual void active_update(float dt_sec) override;

  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "granito"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "Granito"; }
  static std::string display_name() { return _("Granito"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual bool is_snipable() const override { return false; }
  virtual bool is_freezable() const override { return false; }
  virtual bool is_flammable() const override { return false; }

  virtual void kill_fall() override;

  virtual GameObjectTypes get_types() const override;
  virtual void after_editor_set() override;
  /** @} */

  // FIXME: Is this a good name?
  /** \addtogroup GranitoAPI
      @{ */

  /**
   * @scripting
   * @description Makes the Granito wave.
   */
  void wave();

  /**
   * @scripting
   * @description Makes the Granito sit.
   */
  void sit();

  /**
   * @scripting
   * @description Makes the Granito sit.
   * @param string $direction Direction to turn to. Can be "left or "right".
   */
  void turn(const std::string& direction);

  /**
   * @scripting
   * @description Sets the walking state for the Granito.
   * @param bool $walking
   */
  void set_walking(bool walking)
  {
    if (walking)
      walk();
    else
      stand();
  }

  /**
   * @scripting
   * @description Makes the Granito walk.
   */
  void walk();

  /**
   * @scripting
   * @description Makes the Granito stand, or stop if walking.
   */
  void stand();

  /**
   * @scripting
   * @description Makes the Granito jump.
   */
  void jump();

  //TODO: No way to expose enums?
  /**
   * @scripting
   * @description Gets the current granito state.
   *              0 - ""SIT""
   *              1 - ""STAND""
   *              2 - ""WALK""
   *              3 - ""WAVE""
   *              4 - ""LOOKUP""
   *              5 - ""JUMPING""
   */
  int get_state() { return static_cast<int>(m_state); }

  /** @} */

protected:
  virtual void initialize() override;
  virtual void update_hitbox() override;

  void activate() override;

protected:
  enum Type { DEFAULT, STAND, WALK, SCRIPTABLE, SIT };

  /**
   * NOTE: When changing this, make sure to also change
   * the description for get_state()
   */
  enum State
  {
    STATE_SIT,
    STATE_STAND,
    STATE_WALK,
    STATE_WAVE,
    STATE_LOOKUP,
    STATE_JUMPING
  };

protected:
  virtual bool try_wave();
  virtual bool try_jump();

  void restore_original_state();

private:
  Timer m_walk_interval;
  State m_state;
  State m_original_state;

  bool m_has_waved;
  bool m_stepped_on; /** True if tux was on top of granito last frame. */
  bool m_airborne; /** Unfortunately, on_ground() sucks. */

private:
  Granito(const Granito&) = delete;
  Granito& operator=(const Granito&) = delete;
};

#endif

/* EOF */
