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

Player::Player(::Player* parent) :
  m_parent(parent)
{
}

Player::~Player()
{
}

bool
Player::add_bonus(const std::string& bonus)
{
  return m_parent->add_bonus(bonus);
}

bool
Player::set_bonus(const std::string& bonus)
{
  return m_parent->set_bonus(bonus);
}

void
Player::add_coins(int count)
{
  m_parent->add_coins(count);
}

int
Player::get_coins() const
{
  return m_parent->get_coins();
}

void
Player::make_invincible()
{
  m_parent->make_invincible();
}

void
Player::deactivate()
{
  m_parent->deactivate();
}

void
Player::activate()
{
  m_parent->activate();
}

void
Player::walk(float speed)
{
  m_parent->walk(speed);
}

void
Player::set_dir(bool right)
{
  m_parent->set_dir(right);
}

void
Player::set_visible(bool visible)
{
  m_parent->set_visible(visible);
}

bool
Player::get_visible() const
{
  return m_parent->get_visible();
}

void
Player::kill(bool completely)
{
  m_parent->kill(completely);
}

void
Player::set_ghost_mode(bool enable)
{
  m_parent->set_ghost_mode(enable);
}

bool
Player::get_ghost_mode() const
{
  return m_parent->get_ghost_mode();
}

void
Player::kick()
{
  m_parent->kick();
}

void
Player::do_cheer()
{
  m_parent->do_cheer();
}

void
Player::do_duck()
{
  m_parent->do_duck();
}

void
Player::do_standup()
{
  m_parent->do_standup();
}

void
Player::do_backflip()
{
  m_parent->do_backflip();
}

void
Player::do_jump(float yspeed)
{
  m_parent->do_jump(yspeed);
}

void
Player::trigger_sequence(const std::string& sequence_name)
{
  m_parent->trigger_sequence(sequence_name);
}

void
Player::use_scripting_controller(bool use_or_release)
{
  m_parent->use_scripting_controller(use_or_release);
}

void
Player::do_scripting_controller(const std::string& control, bool pressed)
{
  m_parent->do_scripting_controller(control, pressed);
}

float
Player::get_velocity_x() const
{
  return m_parent->physic.get_velocity_x();
}

float
Player::get_velocity_y() const
{
  return m_parent->physic.get_velocity_y();
}

bool
Player::has_grabbed(const std::string& name) const
{
  return m_parent->has_grabbed(name);
}

} // namespace scripting

/* EOF */
