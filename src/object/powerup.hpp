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

#ifndef HEADER_SUPERTUX_OBJECT_POWERUP_HPP
#define HEADER_SUPERTUX_OBJECT_POWERUP_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

class PowerUp : public MovingSprite
{
public:
  PowerUp(const ReaderMapping& lisp);
  PowerUp(const Vector& pos, const std::string& sprite_name);
  virtual void save(Writer& writer);

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual void collision_solid(const CollisionHit& hit);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  std::string get_class() const {
    return "powerup";
  }
  std::string get_display_name() const {
    return _("Power up");
  }

  virtual ObjectSettings get_settings();

private:
  Physic physic;
  std::string script;
  bool no_physics;
  Color light;
  SpritePtr lightsprite;
};

#endif

/* EOF */
