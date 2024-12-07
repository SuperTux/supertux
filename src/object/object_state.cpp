//  SuperTux
//  Copyright (C) 2024 Hyland B. <me@ow.swag.toys>
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

#include "object_state.hpp"
#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

ObjectState::ObjectState() :
  m_state()
{ }

ObjectState::ObjectState(const std::size_t size) :
  m_state(size+1)
{
}

void
ObjectState::expose(ssq::VM& vm)
{
  ssq::Class cls = vm.addClass<ObjectState>("ObjectState");
  
  cls.addFunc("set_state", &ObjectState::set);
  cls.addFunc("get_state", &ObjectState::get);
}
