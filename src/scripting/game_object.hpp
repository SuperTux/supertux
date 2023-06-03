//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_GAME_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_GAME_OBJECT_HPP

#include "supertux/game_object_manager.hpp"
#include "util/log.hpp"
#include "util/uid.hpp"

#ifndef SCRIPTING_API

#define SCRIPT_GUARD_VOID                                               \
  auto object_ptr = get_object_ptr();                                   \
  if (object_ptr == nullptr) {                                          \
    log_fatal << "error: script is accessing a dead object: "           \
              << m_uid << std::endl;                                    \
    return;                                                             \
  }                                                                     \
  auto& object = *object_ptr

#define SCRIPT_GUARD_DEFAULT                                            \
  auto object_ptr = get_object_ptr();                                   \
  if (object_ptr == nullptr) {                                          \
    log_fatal << "error: script is accessing a dead object: "           \
              << m_uid << std::endl;                                    \
    return {};                                                          \
  }                                                                     \
  auto& object = *object_ptr

#define SCRIPT_GUARD_RETURN(x)                                          \
  auto object_ptr = get_object_ptr();                                   \
  if (object_ptr == nullptr) {                                          \
    log_fatal << "error: script is accessing a dead object: "           \
              << m_uid << std::endl;                                    \
    return x;                                                           \
  }                                                                     \
  auto& object = *object_ptr

#define SCRIPT_GUARD_VOID_T(OBJECT)                                     \
  auto object_ptr = GameObject<::OBJECT>::get_object_ptr();             \
  if (object_ptr == nullptr) {                                          \
    log_fatal << "error: script is accessing a dead object: "           \
              << GameObject<::OBJECT>::m_uid << std::endl;              \
    return;                                                             \
  }                                                                     \
  auto& object = *object_ptr

#define SCRIPT_GUARD_DEFAULT_T(OBJECT)                                  \
  auto object_ptr = GameObject<::OBJECT>::get_object_ptr();             \
  if (object_ptr == nullptr) {                                          \
    log_fatal << "error: script is accessing a dead object: "           \
              << GameObject<::OBJECT>::m_uid << std::endl;              \
    return {};                                                          \
  }                                                                     \
  auto& object = *object_ptr

#define SCRIPT_GUARD_RETURN_T(OBJECT, x)                                \
  auto object_ptr = GameObject<::OBJECT>::get_object_ptr();             \
  if (object_ptr == nullptr) {                                          \
    log_fatal << "error: script is accessing a dead object: "           \
              << GameObject<::OBJECT>::m_uid << std::endl;              \
    return x;                                                           \
  }                                                                     \
  auto& object = *object_ptr

class GameObjectManager;

namespace scripting {

::GameObjectManager& get_game_object_manager();

template<class T>
class GameObject
{
public:
  GameObject(UID uid) :
    m_uid(uid)
  {}

  T* get_object_ptr() const
  {
    return get_game_object_manager().get_object_by_uid<T>(m_uid);
  }

protected:
  UID m_uid;
};

} // namespace scripting

#endif

#endif

/* EOF */
