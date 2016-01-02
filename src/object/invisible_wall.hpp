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

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

class Player;

/** A tile that starts falling down if tux stands to long on it */
class InvisibleWall : public MovingSprite
{
public:
  InvisibleWall(const ReaderMapping& lisp);
  virtual void save(Writer& writer);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual std::string get_class() const {
    return "invisible_wall";
  }

  virtual ObjectSettings get_settings();

private:
  Physic physic;
  float width, height;
};

#endif

/* EOF */
