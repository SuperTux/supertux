//  SuperTux - PushButton running a script
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_PUSHBUTTON_HPP
#define HEADER_SUPERTUX_OBJECT_PUSHBUTTON_HPP

#include "object/moving_sprite.hpp"

/** PushButton - jump on it to run a script */
class PushButton final : public MovingSprite
{
public:
  PushButton(const ReaderMapping& reader);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;
  virtual std::string get_class() const override { return "pushbutton"; }
  virtual std::string get_display_name() const override { return _("Button"); }

  virtual ObjectSettings get_settings() override;

  virtual void on_flip(float height) override;

private:
  enum PushButtonState {
    OFF,
    ON
  };

  std::string script;
  PushButtonState state;
  bool m_upside_down;

private:
  PushButton(const PushButton&) = delete;
  PushButton& operator=(const PushButton&) = delete;
};

#endif

/* EOF */
