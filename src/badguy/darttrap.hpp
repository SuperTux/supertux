//  DartTrap - Shoots a Dart at regular intervals
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "object/sticky_object.hpp"

/** Shoots darts at regular intervals
 *
 * @scripting
 * @summary A ""DartTrap"" that was given a name can be controlled by scripts.
            It shoots darts at regular intervals.
 * @instances A ""DartTrap"" is instantiated by placing a definition inside a
 level. It can then be accessed by its name from a script or via ""sector.name""
 from the console.
*/
class DartTrap final : public StickyBadguy
{
public:
  static void register_class(ssq::VM& vm);

public:
  DartTrap(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual void activate() override;
  virtual void active_update(float dt_sec) override;

  /**
   * @scripting
   * @description Gets the delay between consecutive dart firings
   * @return Delay in seconds
   */
  float get_fire_delay() const { return m_fire_delay; };

  /**
   * @scripting
   * @description Sets the delay between consecutive dart firings
   * @param fire_delay Delay in seconds
   */
  void set_fire_delay(float fire_delay) { m_fire_delay = fire_delay; };

  /**
   * @scripting
   * @description Gets the amount of ammunition the darttrap has. 
   * @return Ammunition of the darttrap, -1 for infinite ammunition.
   */
  int get_ammo() const { return m_ammo; };

  /**
   * @scripting
   * @description Sets the amount of ammunition the darttrap has. 
   * @param ammo Ammunition the darttrap is supposed to have, -1 for infinite ammunition.
   */
  void set_ammo(int ammo) { m_ammo = ammo; }

  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  static std::string class_name() { return "darttrap"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "DartTrap"; }
  static std::string display_name() { return _("Dart Trap"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return StickyBadguy::get_class_types().add(typeid(DartTrap)); }

  virtual ObjectSettings get_settings() override;
  virtual GameObjectTypes get_types() const override;
  virtual bool get_type_from_sprite(const std::string& sprite_name, std::string& type) const override;
  virtual std::string get_default_sprite_name() const override;
  virtual void kill_fall() override;

  virtual void on_flip(float height) override;
  virtual void on_type_change(int old_type) override;

  /**
   * @scripting
   * @description Enables the DartTrap.
   */
  void enable();

  /**
   * @scripting
   * @description Disables the DartTrap.
   */
  void disable();

protected:
  virtual std::vector<Direction> get_allowed_directions() const override;

protected:
  enum State {
    IDLE, LOADING
  };

  enum Type {
    GRANITO, SKULL
  };

  void load();
  void fire();

private:
  bool m_enabled;
  float m_initial_delay;
  float m_fire_delay;
  int m_ammo; // ammo left (-1 means unlimited)

  std::string m_dart_sprite;
  // TODO: This should be configurable (possibly with linked sprites)
  std::string m_dart_lightsprite;

  State m_state;
  Timer m_fire_timer;

private:
  DartTrap(const DartTrap&) = delete;
  DartTrap& operator=(const DartTrap&) = delete;
};
