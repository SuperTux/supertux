//  SuperTux - Rusty Trampoline
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
//  Copyright (C) 2011 Jonas Kuemmerlin <rgcjonas@googlemail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_RUSTY_TRAMPOLINE_HPP
#define HEADER_SUPERTUX_OBJECT_RUSTY_TRAMPOLINE_HPP

#include "object/rock.hpp"

/**
 * Jumping on a trampoline makes tux jump higher.
 * After 3 jumps, it breaks (configurable)
 * It cannot be carried (breaks on ungrab)
 */
class RustyTrampoline : public Rock
{
public:
  RustyTrampoline(const ReaderMapping& reader);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void collision_solid(const CollisionHit& hit);
  void update(float elapsed_time);

  void grab(MovingObject&, const Vector& pos, Direction);
  void ungrab(MovingObject&, Direction);
  bool is_portable() const;
  virtual std::string get_class() const {
    return "rustytrampoline";
  }
  std::string get_display_name() const {
    return _("Rusty trampoline");
  }

private:
  bool portable;
  int counter;

};

#endif

/* EOF */
