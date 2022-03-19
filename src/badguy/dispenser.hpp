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

#ifndef HEADER_SUPERTUX_BADGUY_DISPENSER_HPP
#define HEADER_SUPERTUX_BADGUY_DISPENSER_HPP

#include "badguy/badguy.hpp"
#include "scripting/dispenser.hpp"
#include "squirrel/exposed_object.hpp"

class Dispenser final : public BadGuy,
                        public ExposedObject<Dispenser, scripting::Dispenser>
{
private:
  enum class DispenserType {
    DROPPER, ROCKETLAUNCHER, CANNON, POINT
  };

  static DispenserType DispenserType_from_string(const std::string& type_string);
  static std::string DispenserType_to_string(DispenserType type);

public:
  Dispenser(const ReaderMapping& reader);

  virtual void draw(DrawingContext& context) override;
  virtual void activate() override;
  virtual void deactivate() override;
  virtual void active_update(float dt_sec) override;

  virtual void freeze() override;
  virtual void unfreeze() override;
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;
  virtual std::string get_class() const override { return "dispenser"; }
  virtual std::string get_display_name() const override { return _("Dispenser"); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void on_flip(float height) override;

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) override
  {
    ExposedObject<Dispenser, scripting::Dispenser>::expose(vm, table_idx);
  }

  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) override
  {
    ExposedObject<Dispenser, scripting::Dispenser>::unexpose(vm, table_idx);
  }

  void notify_dead() {
    if (m_limit_dispensed_badguys) {
      m_current_badguys--;
    }
  }

protected:
  virtual bool collision_squished(GameObject& object) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  void launch_badguy();

private:
  void set_correct_action();

private:
  float m_cycle;
  std::vector<std::string> m_badguys;
  unsigned int m_next_badguy;
  Timer m_dispense_timer;
  bool m_autotarget;
  bool m_swivel;
  bool m_broken;
  bool m_random;
  bool m_gravity;

  DispenserType m_type;
  std::string m_type_str;

  /** Do we need to limit the number of dispensed badguys? */
  bool m_limit_dispensed_badguys;

  /** Maximum concurrent number of badguys to be dispensed */
  int m_max_concurrent_badguys;

  /** Current amount of spawned badguys */
  int m_current_badguys;

private:
  Dispenser(const Dispenser&) = delete;
  Dispenser& operator=(const Dispenser&) = delete;
};

#endif

/* EOF */
