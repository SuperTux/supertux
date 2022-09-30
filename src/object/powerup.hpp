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

class PowerUp final : public MovingSprite
{
public:
  PowerUp(const ReaderMapping& mapping);
  PowerUp(const Vector& pos, const std::string& sprite_name);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual void on_flip(float height) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "powerup"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Powerup"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

private:
  /** Initialize power up sprites and other defaults */
  virtual void initialize();

private:
  Physic physic;
  std::string script;
  bool no_physics;
  SpritePtr lightsprite;

private:
  PowerUp(const PowerUp&) = delete;
  PowerUp& operator=(const PowerUp&) = delete;
};

#endif

/* EOF */
