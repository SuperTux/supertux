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

class Dispenser : public BadGuy
{
public:
  Dispenser(const ReaderMapping& reader);

  void draw(DrawingContext& context);
  void activate();
  void deactivate();
  void active_update(float elapsed_time);

  void freeze();
  void unfreeze();
  bool is_freezable() const;
  bool is_flammable() const;
  std::string get_class() const {
    return "dispenser";
  }
  std::string get_display_name() const {
    return _("Dispenser");
  }
  std::string get_type_string() const {
    switch(type) {
    case DT_DROPPER:
      return "dropper";
    case DT_ROCKETLAUNCHER:
      return "rocketlauncher";
    case DT_CANNON:
      return "cannon";
    case DT_POINT:
      return "point";
    default:
      return "unknown";
    }
  }

  ObjectSettings get_settings();
  void after_editor_set();

protected:
  bool collision_squished(GameObject& object);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void launch_badguy();

private:

  void set_correct_action();

  /** CollisionGroup the badguy should be in while active */
  CollisionGroup colgroup_active;

  float cycle;
  std::vector<std::string> badguys;
  unsigned int next_badguy;
  Timer dispense_timer;
  bool autotarget;
  bool swivel;
  bool broken;
  bool random;

  typedef enum {
    DT_DROPPER, DT_ROCKETLAUNCHER, DT_CANNON, DT_POINT
  } DispenserType;

  DispenserType type;
  std::string type_str;
};

#endif

/* EOF */
