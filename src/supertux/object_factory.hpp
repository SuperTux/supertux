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

#include "supertux/direction.hpp"
#include "supertux/game_object_ptr.hpp"
#include "util/reader_fwd.hpp"

class Vector;
class GameObject;

class ObjectFactory
{
public:
  static ObjectFactory& instance();

private:
  typedef std::map<std::string, std::function<GameObjectPtr (const ReaderMapping&)> > Factories;
  Factories factories;

public:
  ObjectFactory();
  ~ObjectFactory();

  GameObjectPtr create(const std::string& name, const ReaderMapping& reader) const;
  GameObjectPtr create(const std::string& name, const Vector& pos, const Direction& dir = AUTO, const std::string& data = {}) const;

private:
  void add_factory(const char* name,
                   std::function<GameObjectPtr (const ReaderMapping&)> func)
  {
    assert(factories.find(name) == factories.end());
    factories[name] = func;
  }

  template<class C>
  void add_factory(const char* name)
  {
    add_factory(name, [](const ReaderMapping& reader) {
        return std::make_shared<C>(reader);
      });
  }
  void init_factories();
};

#endif

/* EOF */
