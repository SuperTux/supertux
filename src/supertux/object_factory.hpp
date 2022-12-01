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
#include <vector>

#include "math/fwd.hpp"
#include "supertux/direction.hpp"

class ReaderMapping;
class GameObject;

class ObjectFactory
{
private:
  typedef std::function<std::unique_ptr<GameObject> (const ReaderMapping&)> FactoryFunction;
  typedef std::function<std::string ()> NameFactoryFunction;
  typedef std::map<std::string, FactoryFunction> Factories;
  typedef std::map<std::string, NameFactoryFunction> NameFactories;
  Factories factories;
  NameFactories name_factories;
  std::vector<std::string> m_badguys_names;
  std::vector<uint8_t> m_badguys_params;
  std::vector<std::string> m_objects_names;
  std::vector<uint8_t> m_objects_params;

protected:
  bool m_adding_badguys;

public:
  enum RegisteredObjectParam
  {
    OBJ_PARAM_NONE=0, OBJ_PARAM_PORTABLE=0b10000000
  };

public:
  /** Will throw in case of creation failure, will never return nullptr */
  std::unique_ptr<GameObject> create(const std::string& name, const ReaderMapping& reader) const;
  std::string get_factory_display_name(const std::string& name) const;

  std::vector<std::string>& get_registered_badguys() { return m_badguys_names; }
  std::vector<std::string> get_registered_badguys(uint8_t params);
  std::vector<std::string>& get_registered_objects() { return m_objects_names; }
  std::vector<std::string> get_registered_objects(uint8_t params);

protected:
  ObjectFactory();

  void add_name_factory(const char* class_name, const NameFactoryFunction& func)
  {
    assert(name_factories.find(class_name) == name_factories.end());
    name_factories[class_name] = func;
  }
  
  void add_custom_name_factory(const char* class_name, const std::string display_name)
  {
    add_name_factory(class_name, [display_name]() {
        return display_name;
      });
  }

  void add_factory(const char* name, const FactoryFunction& func, uint8_t obj_params = 0)
  {
    assert(factories.find(name) == factories.end());
    if (m_adding_badguys)
    {
      m_badguys_names.push_back(name);
      m_badguys_params.push_back(obj_params);
    }
    m_objects_names.push_back(name);
    m_objects_params.push_back(obj_params);
    factories[name] = func;
  }

  template<class C>
  void add_factory(const char* class_name, uint8_t obj_params = 0)
  {
    add_factory(class_name, [](const ReaderMapping& reader) {
        return std::make_unique<C>(reader);
      }, obj_params);
    add_name_factory(class_name, []() {
        return C::display_name();
      });
  }
};

#endif

/* EOF */
