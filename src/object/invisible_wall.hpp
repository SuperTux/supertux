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

#ifndef HEADER_SUPERTUX_OBJECT_INVISIBLE_WALL_HPP
#define HEADER_SUPERTUX_OBJECT_INVISIBLE_WALL_HPP

#include "supertux/moving_object.hpp"
#include "util/reader_fwd.hpp"

/** A tile that starts falling down if tux stands to long on it */
class InvisibleWall : public MovingObject
{
public:
  InvisibleWall(const ReaderMapping& lisp);
  virtual void save(Writer& writer);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void draw(DrawingContext& context);

  virtual std::string get_class() const {
    return "invisible_wall";
  }
  std::string get_display_name() const {
    return _("Invisible wall");
  }

  virtual ObjectSettings get_settings();
  virtual void after_editor_set();

private:
  Physic physic;

  void draw(DrawingContext& context);
  void update(float elapsed_time);

  float width, height;
};

#endif

/* EOF */
