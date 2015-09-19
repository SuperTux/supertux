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

#include "supertux/direction.hpp"
#include "supertux/game_object_ptr.hpp"
#include "util/reader_fwd.hpp"

class Vector;
class GameObject;

class AbstractObjectFactory
{
public:
  virtual ~AbstractObjectFactory()
  { }

  /** Creates a new gameobject from a lisp node.
   * Remember to delete the objects later
   */
  virtual GameObjectPtr create(const Reader& reader) = 0;
};

template<class C>
class ConcreteObjectFactory : public AbstractObjectFactory
{
public:
  ConcreteObjectFactory() {}
  ~ConcreteObjectFactory() {}

  GameObjectPtr create(const Reader& reader)
  {
    return std::make_shared<C>(reader);
  }
};

class ObjectFactory
{
public:
  static ObjectFactory& instance();

private:
  typedef std::map<std::string, std::unique_ptr<AbstractObjectFactory> > Factories;
  Factories factories;

public:
  ObjectFactory();
  ~ObjectFactory();

  GameObjectPtr create(const std::string& name, const Reader& reader) const;
  GameObjectPtr create(const std::string& name, const Vector& pos, const Direction dir = AUTO) const;

private:
  template<class C>
  void add_factory(const char* name)
  {
    assert(factories.find(name) == factories.end());
    factories[name] = std::unique_ptr<AbstractObjectFactory>(new ConcreteObjectFactory<C>());
  }
  void init_factories();
};

#endif

/* EOF */
