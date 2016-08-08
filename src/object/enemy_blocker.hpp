//  SuperTux
//  Copyright (C) 2007 Kovago Zoltan <DirtY.iCE.hu@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_ENEMY_BLOCKER_HPP
#define HEADER_SUPERTUX_OBJECT_ENEMY_BLOCKER_HPP

#include "supertux/moving_object.hpp"
#include "util/reader_fwd.hpp"

/** An object that only solid for badguys. */
class EnemyBlocker : public MovingObject
{
public:
  EnemyBlocker(const ReaderMapping& lisp);

  virtual bool collides(GameObject& other, const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  void draw(DrawingContext& context);

  std::string get_class() const {
    return "enemy_blocker";
  }
  std::string get_display_name() const {
    return _("Enemy blocker");
  }

  virtual ObjectSettings get_settings();
  virtual void after_editor_set();

private:
  void update(float elapsed_time);

  float width, height;
};

#endif

/* EOF */
