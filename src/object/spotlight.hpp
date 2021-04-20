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

#ifndef HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP
#define HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP

#include "sprite/sprite_ptr.hpp"
#include "supertux/moving_object.hpp"
#include "video/color.hpp"

class ReaderMapping;

class Spotlight final : public MovingObject
{
public:
  Spotlight(const ReaderMapping& reader);
  ~Spotlight() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit_) override;

  virtual std::string get_class() const override { return "spotlight"; }
  virtual std::string get_display_name() const override { return _("Spotlight"); }

  virtual ObjectSettings get_settings() override;

private:
  float   angle;
  SpritePtr center;
  SpritePtr base;
  SpritePtr lights;
  SpritePtr light;
  SpritePtr lightcone;

  Color   color;

  /** Speed that the spotlight is rotating with */
  float speed;

  /** If true, the spotlight will rotate counter-clockwise */
  bool counter_clockwise;

  /** The layer (z-pos) of the spotlight. */
  int m_layer;

private:
  Spotlight(const Spotlight&) = delete;
  Spotlight& operator=(const Spotlight&) = delete;
};

#endif

/* EOF */
