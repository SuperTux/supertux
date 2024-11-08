//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_UTIL_TYPED_UID_HPP
#define HEADER_SUPERTUX_UTIL_TYPED_UID_HPP

#include "util/uid.hpp"

#include "supertux/game_object_manager.hpp"

/**
 * Typed UIDs allows storing GameObjects as UID while offering to option to use
 * the object either as if it was a pointer or as if it was an UID. This helps
 * staying resilient against the objects' variable lifetime.
 */
template<class T>
class TypedUID : public UID
{
public:
  TypedUID() : UID() {}
  TypedUID(const T* object) : UID()
  {
    if (object)
    {
      UID::operator=(object->get_uid());
      m_object_manager = object->get_parent();
    }
  }
  TypedUID(const TypedUID& other) = default;
  TypedUID& operator=(const TypedUID& other) = default;

  inline T* get() const
  {
    if (m_value == 0 || !m_object_manager)
      return nullptr;

    auto* object = m_object_manager->get_object_by_uid<T>(*this);

    // Would break const correctness
#if 0
    if (!object)
    {
      *this = UID(); 
    }
#endif

    return object;
  }

  inline TypedUID& operator=(const T* object)
  {
    UID::operator=(object ? object->get_uid() : UID());
    m_object_manager = object ? object->get_parent() : nullptr;
    return *this;
  }

  template<class C>
  inline TypedUID& operator=(const TypedUID<C>& other)
  {
    static_assert(std::is_base_of<T, C>::value, "TypedUID object type must inherit target TypedUID object type!");

    UID::operator=(other);
    m_object_manager = other.get_object_manager();
    return *this;
  }

  inline T* operator->() const
  {
    return get();
  }

  inline T& operator*() const
  {
    T* t = get();
    if (!t)
      throw std::runtime_error("Attempt to dereference invalid TypedUID");

    return *t;
  }

  inline bool operator==(const T* object) const
  {
    return (!object && m_value == 0) || object->get_uid() == *this;
  }

  inline bool operator!=(const T* object) const
  {
    return object ? object->get_uid() != *this : m_value == 0;
  }

  inline operator bool() const
  {
    return m_value != 0 && m_object_manager && m_object_manager->get_object_by_uid<T>(*this);
  }

  inline GameObjectManager* get_object_manager() const { return m_object_manager; }

private:
  GameObjectManager* m_object_manager;
};

template<class T>
inline bool operator==(const T* object, const TypedUID<T>& typed_uid) {
  return typed_uid == object;
}

template<class T>
inline bool operator!=(const T* object, const TypedUID<T>& typed_uid) {
  return typed_uid != object;
}

#endif

/* EOF */
