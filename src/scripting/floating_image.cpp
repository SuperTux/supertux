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

namespace scripting {

FloatingImage::FloatingImage(const std::string& spritefile) :
  GameObject(get_sector().add<::FloatingImage>(spritefile))
{
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
FloatingImage::fade_in(float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_in(time);
}

void
FloatingImage::fade_out(float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_out(time);
}

} // namespace scripting

/* EOF */
