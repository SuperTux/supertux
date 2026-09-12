//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@gmail.com>
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

#pragma once

#include "supertux/sector.hpp"

namespace {

  /**
   * A function that creates a GameObject of type T and adds it to the current sector
   */
  template<class T>
  T* get_default_object_initializer()
  {
    if (!Sector::current())
    {
      auto type_name = std::string(typeid(T).name());
      auto msg = std::string("Tried to create " + type_name + " object without an active sector.");
      throw std::runtime_error(msg);
    }

    return &Sector::get().add<T>();
  }
}
