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

/** Jumping on a trampoline makes tux jump higher.
    After 3 jumps, it breaks (configurable)
    It cannot be carried (breaks on ungrab) */
class RustyTrampoline final : public Rock
{
public:
  RustyTrampoline(const ReaderMapping& reader);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;

  virtual void grab(MovingObject&, const Vector& pos, Direction) override;
  virtual void ungrab(MovingObject&, Direction) override;
  virtual bool is_portable() const override;
  virtual std::string get_class() const override { return "rustytrampoline"; }
  virtual std::string get_display_name() const override { return _("Rusty Trampoline"); }
  virtual ObjectSettings get_settings() override;

private:
  bool portable;
  int counter;

private:
  RustyTrampoline(const RustyTrampoline&) = delete;
  RustyTrampoline& operator=(const RustyTrampoline&) = delete;
};

#endif

/* EOF */
