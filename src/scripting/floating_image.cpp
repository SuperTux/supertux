//  SuperTux
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

#include <stdexcept>

#include "object/floating_image.hpp"
#include "scripting/floating_image.hpp"
#include "supertux/sector.hpp"
#include "worldmap/worldmap.hpp"

#define SCRIPT_GUARD_VOID \
  auto object_ptr = get_game_object_manager().get_object_by_uid<::FloatingImage>(m_uid); \
  if (object_ptr == nullptr)                    \
    return;                                     \
  auto& object = *object_ptr

#define SCRIPT_GUARD_DEFAULT \
  auto object_ptr = get_game_object_manager().get_object_by_uid<::FloatingImage>(m_uid); \
  if (object_ptr == nullptr)                    \
    return {};                                  \
  auto& object = *object_ptr

#define SCRIPT_GUARD_RETURN(x) \
  auto object_ptr = get_game_object_manager().get_object_by_uid<::FloatingImage>(m_uid); \
  if (object_ptr == nullptr)                    \
    return x;                                   \
  auto& object __attribute__((unused)) = *object_ptr

namespace {

GameObjectManager& get_game_object_manager()
{
  using namespace worldmap;

  if (::Sector::current() != nullptr) {
    return ::Sector::get();
  } else if(WorldMap::current() != nullptr) {
    return *WorldMap::current();
  } else {
    throw std::runtime_error("Neither sector nor worldmap active");
  }
}

} // namespace

namespace scripting {

FloatingImage::FloatingImage(const std::string& spritefile) :
  m_uid()
{
  m_uid = get_game_object_manager().add<::FloatingImage>(spritefile)->get_uid();
}

void
FloatingImage::set_layer(int layer)
{
  SCRIPT_GUARD_VOID;
  object.set_layer(layer);
}

int
FloatingImage::get_layer() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_layer();
}

void
FloatingImage::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_pos(Vector(x, y));
}

float
FloatingImage::get_pos_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos().x;
}

float
FloatingImage::get_pos_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos().y;
}

void
FloatingImage::set_anchor_point(int anchor)
{
  SCRIPT_GUARD_VOID;
  object.set_anchor_point(static_cast<AnchorPoint>(anchor));
}

int
FloatingImage::get_anchor_point() const
{
  SCRIPT_GUARD_DEFAULT;
  return static_cast<int>(object.get_anchor_point());
}

bool
FloatingImage::get_visible() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_visible();
}

void
FloatingImage::set_visible(bool visible)
{
  SCRIPT_GUARD_VOID;
  object.set_visible(visible);
}

void
FloatingImage::set_action(const std::string& action)
{
  SCRIPT_GUARD_VOID;
  object.set_action(action);
}

std::string
FloatingImage::get_action() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_action();
}

void
FloatingImage::fade_in(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.fade_in(fadetime);
}

void
FloatingImage::fade_out(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.fade_out(fadetime);
}

bool
FloatingImage::is_valid() const
{
  SCRIPT_GUARD_RETURN(false);
  return true;
}

} // scripting

/* EOF */
