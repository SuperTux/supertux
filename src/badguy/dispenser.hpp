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

#ifndef __DISPENSER_H__
#define __DISPENSER_H__

#include "badguy.hpp"
#include "supertux/timer.hpp"

class Dispenser : public BadGuy
{
public:
  Dispenser(const lisp::Lisp& reader);

  void activate();
  void deactivate();
  void write(lisp::Writer& writer);
  void active_update(float elapsed_time);

  void freeze();
  void unfreeze();
  bool is_freezable() const;

  virtual Dispenser* clone() const { return new Dispenser(*this); }

protected:
  bool collision_squished(GameObject& object);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void launch_badguy();
  float cycle;
  std::vector<std::string> badguys;
  unsigned int next_badguy;
  Timer dispense_timer;
  bool autotarget;
  bool swivel;
  bool broken;
  bool random;
  std::string type;
};

#endif
