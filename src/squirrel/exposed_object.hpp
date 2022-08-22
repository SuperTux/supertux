//  SuperTux -- ExposedObject class
//  Copyright (C) 2016 Tobias Markus <tobbi.bugs@googlemail.com>
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

#ifndef HEADER_SUPERTUX_SQUIRREL_EXPOSED_OBJECT_HPP
#define HEADER_SUPERTUX_SQUIRREL_EXPOSED_OBJECT_HPP

#include <memory>

#include "squirrel/squirrel_virtual_machine.hpp"
#include "squirrel/squirrel_util.hpp"
#include "squirrel/script_interface.hpp"
#include "util/log.hpp"

/**
 * @class ExposedObject
 * This class binds a certain GameObject class to a scripting class.
 * To bind a game object class to a scripting class, extend the GameObject
 * class as shown in the following example:
 * \code{.cpp}
 * public ExposedObject<Gradient, scripting::Gradient>
 * \endcode
 *
 * and instantiate it in each constructor with the <i>this</i> pointer, like this:
 * \code{.cpp}
 * Gradient::Gradient(const ReaderMapping& reader) :
 *  ExposedObject<Gradient, scripting::Gradient>(this)
 * \endcode
 * @param class S: GameObject class (e.g. Gradient)
 * @param class T: Scripting class (e.g. scripting::Gradient)
 */
template<class S, class T>
class ExposedObject : virtual public ScriptInterface
{
private:
  /**
 * The parent object that is exposed to the script interface
 */
  S* m_parent;

public:
  /**
   * Constructor
   * @param parent GameObject
   */
  ExposedObject(S* parent) :
    m_parent(parent)
  {
  }

  /**
   * Exposes the parent GameObject to the script Interface
   * @param vm The squirrel virtual machine to expose the object on
   * @param table_idx Index of the table to expose the object on
   */
  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) override
  {
    auto name = m_parent->get_name();
    if (name.empty())
    {
      return;
    }

    log_debug << "Exposing " << m_parent->get_class_name() << " object " << name << std::endl;

    auto object = std::make_unique<T>(m_parent->get_uid());
    expose_object(vm, table_idx, std::move(object), name);
  }

  /**
   * Un-exposes the parent GameObject to the script Interface
   * @param vm The squirrel virtual machine to un-expose the object on
   * @param table_idx Index of the table to un-expose the object on
   */
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) override
  {
    auto name = m_parent->get_name();
    if (name.empty())
    {
      return;
    }

    log_debug << "Unexposing object " << name << std::endl;

    unexpose_object(vm, table_idx, name);
  }

private:
  ExposedObject(const ExposedObject&) = delete;
  ExposedObject& operator=(const ExposedObject&) = delete;
};

#endif

/* EOF */
