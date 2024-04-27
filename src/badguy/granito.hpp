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

/**
 * Interactable friendly NPC
 *
 * @scripting
 * @summary A ""Granito"" that was given a name can be controlled by scripts.
 * @instances A ""Granito"" is instantiated by placing a definition inside a level.
 *            It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Granito : public WalkingBadguy
{
public:
  static void register_class(ssq::VM& vm);

public:
  Granito(const ReaderMapping& reader,
          const std::string& sprite_name = "images/creatures/granito/granito.sprite",
          int layer = LAYER_OBJECTS);

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

  virtual ObjectSettings get_settings() override;
  virtual GameObjectTypes get_types() const override;
  virtual void after_editor_set() override;

  virtual GranitoBig* get_carrier();
  void turn(const Direction& direction);

  /**
   * @scripting
   * @description Makes the Granito wave.
   */
  virtual void wave();

  /**
   * @scripting
   * @description Makes the Granito sit.
   */
  virtual void sit();

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
   * @description Makes the Granito walk for a specified amount of seconds.
   * @param float $seconds
   */
  void walk_for(float seconds);

  /**
   * @scripting
   * @description Makes the Granito stand, or stop if walking.
   */
  void stand();

  /**
   * @scripting
   * @description Makes the Granito jump.
   */
  virtual void jump();

  /**
   * @scripting
   * @description Eject itself from the Big Granito.
   */
  virtual void eject();

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

  /**
   * @scripting
   * @description Gets the name of the Big Granito that is carrying the Granito.
   */
  std::string get_carrier_name();

  /**
   * @scripting
   * @description Resets the detection checker,
   *              allowing the Detect Script to be ran again.
   */
  void reset_detection() { m_has_waved = false; }


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

protected:
  Timer m_walk_interval;
  State m_state;
  State m_original_state;

  bool m_has_waved;
  bool m_stepped_on; /** True if tux was on top of granito last frame. */
  bool m_airborne; /** Unfortunately, on_ground() sucks. */

  std::string m_detect_script;
  std::string m_carried_script; /** This is ran when the Granito is carried by a Big Granito */

private:
  Granito(const Granito&) = delete;
  Granito& operator=(const Granito&) = delete;
};

#endif

/* EOF */
