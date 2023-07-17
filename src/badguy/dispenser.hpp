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

class GameObject;

class Dispenser final : public BadGuy,
                        public ExposedObject<Dispenser, scripting::Dispenser>
{
private:
  enum DispenserType {
    DROPPER, CANNON, POINT
  };

public:
  Dispenser(const ReaderMapping& reader);

  void add_object(std::unique_ptr<GameObject> object);

  virtual void draw(DrawingContext& context) override;
  virtual void initialize() override;
  virtual void activate() override;
  virtual void deactivate() override;
  virtual void active_update(float dt_sec) override;

  virtual void freeze() override;
  virtual void unfreeze(bool melt = true) override;
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;
  virtual bool is_portable() const override;

  static std::string class_name() { return "dispenser"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Dispenser"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  virtual GameObjectTypes get_types() const override;
  std::string get_default_sprite_name() const override;

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
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  void launch_object();

  void on_type_change(int old_type) override;

private:
  void set_correct_action();

private:
  float m_cycle;
  std::vector<std::unique_ptr<GameObject>> m_objects;
  unsigned int m_next_object;
  Timer m_dispense_timer;
  bool m_autotarget;
  bool m_random;
  bool m_gravity;

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
