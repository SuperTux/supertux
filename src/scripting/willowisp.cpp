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

#include "scripting/willowisp.hpp"

#include "badguy/willowisp.hpp"

namespace scripting {

WillOWisp::WillOWisp(::WillOWisp* parent) :
  m_parent(parent)
{
}

WillOWisp::~WillOWisp()
{
}

void
WillOWisp::goto_node(int node_no)
{
  m_parent->goto_node(node_no);
}

void
WillOWisp::set_state(const std::string& state)
{
  m_parent->set_state(state);
}

void
WillOWisp::start_moving()
{
  m_parent->start_moving();
}

void
WillOWisp::stop_moving()
{
  m_parent->stop_moving();
}

} // namespace scripting

/* EOF */
