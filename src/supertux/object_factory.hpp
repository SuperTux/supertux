//  SuperTux
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_OBJECT_FACTORY_HPP
#define HEADER_SUPERTUX_SUPERTUX_OBJECT_FACTORY_HPP

#include <assert.h>
#include <map>
#include <memory>
#include <functional>

#include "math/fwd.hpp"
#include "supertux/direction.hpp"

class ReaderMapping;
class GameObject;

class ObjectFactory
{
private:
  typedef std::function<std::unique_ptr<GameObject> (const ReaderMapping&)> FactoryFunction;
  typedef std::map<std::string, FactoryFunction> Factories;
  Factories factories;

public:
  /** Will throw in case of creation failure, will never return nullptr */
  std::unique_ptr<GameObject> create(const std::string& name, const ReaderMapping& reader) const;

protected:
  ObjectFactory();

  void add_factory(const char* name, const FactoryFunction& func)
  {
    assert(factories.find(name) == factories.end());
    factories[name] = func;
  }

  template<class C>
  void add_factory(const char* name)
  {
    add_factory(name, [](const ReaderMapping& reader) {
        return std::make_unique<C>(reader);
      });
  }
};

#endif

/* EOF */
