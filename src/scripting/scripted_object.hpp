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

#ifndef HEADER_SUPERTUX_SCRIPTING_SCRIPTED_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_SCRIPTED_OBJECT_HPP

namespace scripting {

class ScriptedObject
{
public:
#ifndef SCRIPTING_API
  virtual ~ScriptedObject()
  {}
#endif

  virtual void set_action(const std::string& animation) = 0;
  virtual std::string get_action() = 0;

  virtual void move(float x, float y) = 0;
  virtual void set_pos(float x, float y) = 0;
  virtual float get_pos_x() = 0;
  virtual float get_pos_y() = 0;

  virtual void set_velocity(float x, float y) = 0;
  virtual float get_velocity_x() = 0;
  virtual float get_velocity_y() = 0;

  virtual void enable_gravity(bool gravity_enabled) = 0;
  virtual bool gravity_enabled() const = 0;

  virtual void set_visible(bool visible) = 0;
  virtual bool is_visible() = 0;

  virtual void set_solid(bool solid) = 0;
  virtual bool is_solid() = 0;

  virtual std::string get_name() = 0;
};

}

#endif

/* EOF */
