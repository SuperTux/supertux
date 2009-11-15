//  SuperTux - Switch Trigger
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

#ifndef HEADER_SUPERTUX_TRIGGER_SWITCH_HPP
#define HEADER_SUPERTUX_TRIGGER_SWITCH_HPP

#include <string>

#include "trigger/trigger_base.hpp"
#include "util/serializable.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "video/drawing_context.hpp"
#include "sprite/sprite.hpp"

class Switch : public TriggerBase, public Serializable
{
public:
  Switch(const lisp::Lisp& reader);
  virtual ~Switch();

  virtual void write(lisp::Writer& writer);

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual void event(Player& player, EventType type);

private:
  enum SwitchState {
    OFF,
    TURN_ON,
    ON,
    TURN_OFF
  };

  std::string sprite_name;
  Sprite* sprite;
  std::string script;

  SwitchState state;

};

#endif
