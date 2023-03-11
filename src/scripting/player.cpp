//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/player.hpp"

#include "object/player.hpp"

namespace scripting {

bool
Player::add_bonus(const std::string& bonus)
{
  SCRIPT_GUARD_DEFAULT;
  return object.add_bonus(bonus);
}

bool
Player::set_bonus(const std::string& bonus)
{
  SCRIPT_GUARD_DEFAULT;
  return object.set_bonus(bonus);
}

std::string
Player::get_bonus() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.bonus_to_string();
}

void
Player::add_coins(int count)
{
  SCRIPT_GUARD_VOID;
  object.add_coins(count);
}

int
Player::get_coins() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_coins();
}

void
Player::make_invincible()
{
  SCRIPT_GUARD_VOID;
  object.make_invincible();
}

void
Player::deactivate()
{
  SCRIPT_GUARD_VOID;
  object.deactivate();
}

void
Player::activate()
{
  SCRIPT_GUARD_VOID;
  object.activate();
}

void
Player::walk(float speed)
{
  SCRIPT_GUARD_VOID;
  object.walk(speed);
}

void
Player::set_dir(bool right)
{
  SCRIPT_GUARD_VOID;
  object.set_dir(right);
}

void
Player::set_visible(bool visible)
{
  SCRIPT_GUARD_VOID;
  object.set_visible(visible);
}

bool
Player::get_visible() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_visible();
}

void
Player::kill(bool completely)
{
  SCRIPT_GUARD_VOID;
  object.kill(completely);
}

void
Player::set_ghost_mode(bool enable)
{
  SCRIPT_GUARD_VOID;
  object.set_ghost_mode(enable);
}

bool
Player::get_ghost_mode() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_ghost_mode();
}

void
Player::kick()
{
  SCRIPT_GUARD_VOID;
  object.kick();
}

void
Player::do_cheer()
{
  SCRIPT_GUARD_VOID;
  object.do_cheer();
}

void
Player::do_duck()
{
  SCRIPT_GUARD_VOID;
  object.do_duck();
}

void
Player::do_standup()
{
  SCRIPT_GUARD_VOID;
  // Force standup for backwards compatibility
  object.do_standup(true);
}

void
Player::do_backflip()
{
  SCRIPT_GUARD_VOID;
  object.do_backflip();
}

void
Player::do_jump(float yspeed)
{
  SCRIPT_GUARD_VOID;
  object.do_jump(yspeed);
}

void
Player::trigger_sequence(const std::string& sequence_name)
{
  SCRIPT_GUARD_VOID;
  object.trigger_sequence(sequence_name);
}

void
Player::use_scripting_controller(bool use_or_release)
{
  SCRIPT_GUARD_VOID;
  object.use_scripting_controller(use_or_release);
}

void
Player::do_scripting_controller(const std::string& control, bool pressed)
{
  SCRIPT_GUARD_VOID;
  object.do_scripting_controller(control, pressed);
}

float
Player::get_velocity_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_physic().get_velocity_x();
}

float
Player::get_velocity_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_physic().get_velocity_y();
}

bool
Player::has_grabbed(const std::string& name) const
{
  SCRIPT_GUARD_DEFAULT;
  return object.has_grabbed(name);
}

float 
Player::get_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_bbox().get_left();
}

float 
Player::get_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_bbox().get_top();
}

void 
Player::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_pos(Vector(x, y));
}

std::string
Player::get_action() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_action();
}

} // namespace scripting

/* EOF */
