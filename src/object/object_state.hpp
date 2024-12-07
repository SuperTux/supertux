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

#ifndef HEADER_SUPERTUX_OBJECT_OBJECT_STATE_HPP
#define HEADER_SUPERTUX_OBJECT_OBJECT_STATE_HPP
#include <simplesquirrel/vm.hpp>

#include <cstdint>
#include <vector>
#include "squirrel/exposable_class.hpp"

class ObjectState : public ExposableClass
{
public:
  static void expose(ssq::VM& vm);
  
public:
  ObjectState();
  ObjectState(const std::size_t size);
  virtual ~ObjectState() override = default;
  virtual std::string get_class_name() const { return "object-state"; }
  virtual std::string get_exposed_class_name() const { return "ObjectState"; }
  
  inline bool get(std::size_t idx) const {
    return m_state[idx];
  }
  inline void set(std::size_t idx, bool state) {
    m_state[idx] = state;
  }
  // Convenience utility for common scenario, also useful for grepping
  inline constexpr void set_if(bool cond, std::size_t idx, bool state) {
    if (cond)
      set(idx, state);
  }
  
  // Note: Operator[] is not overloaded because aesthetically it doesn't fit this case
private:
  std::vector<bool> m_state;
};

#endif // HEADER_SUPERTUX_OBJECT_STATE_HPP
