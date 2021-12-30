//  SuperTux - Ispy
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_ISPY_HPP
#define HEADER_SUPERTUX_OBJECT_ISPY_HPP

#include "object/moving_sprite.hpp"
#include "supertux/direction.hpp"
#include "video/flip.hpp"

/** An Ispy: When it spots Tux, a script will run. */
class Ispy final : public MovingSprite
{
public:
  Ispy(const ReaderMapping& mapping);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual std::string get_class() const override { return "ispy"; }
  virtual std::string get_display_name() const override { return _("Ispy"); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void on_flip(float height) override;

private:
  enum IspyState {
    ISPYSTATE_IDLE,
    ISPYSTATE_ALERT,
    ISPYSTATE_HIDING,
    ISPYSTATE_SHOWING
  };

private:
  IspyState state; /**< current state */

  std::string script; /**< script to execute when Tux is spotted */
  Direction dir;
  bool m_facing_down;

  Flip m_flip;

private:
  Ispy(const Ispy&) = delete;
  Ispy& operator=(const Ispy&) = delete;
};

#endif

/* EOF */
