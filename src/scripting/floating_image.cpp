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
  m_parent_uid()
{
  using namespace worldmap;

  if(::Sector::current() != nullptr) {
    m_parent_uid = ::Sector::get().add<::FloatingImage>(spritefile)->get_uid();
  } else if(WorldMap::current() != nullptr) {
    m_parent_uid = WorldMap::current()->add<::FloatingImage>(spritefile)->get_uid();
  } else {
    throw std::runtime_error("Neither sector nor worldmap active");
  }
}

::FloatingImage*
FloatingImage::get_floating_image() const
{
  using namespace worldmap;

  if(::Sector::current() != nullptr) {
    return ::Sector::get().get_object_by_uid<::FloatingImage>(m_parent_uid);
  } else if(WorldMap::current() != nullptr) {
    return WorldMap::current()->get_object_by_uid<::FloatingImage>(m_parent_uid);
  } else {
    return nullptr;
  }
}

void
FloatingImage::set_layer(int layer)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->set_layer(layer);
}

int
FloatingImage::get_layer() const
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    return floating_image->get_layer();
  else
    return 0;
}

void
FloatingImage::set_pos(float x, float y)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->set_pos(Vector(x, y));
}

float
FloatingImage::get_pos_x() const
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    return floating_image->get_pos().x;
  else
    return 0.0f;
}

float
FloatingImage::get_pos_y() const
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    return floating_image->get_pos().y;
  else
    return 0.0f;
}

void
FloatingImage::set_anchor_point(int anchor)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->set_anchor_point(static_cast<AnchorPoint>(anchor));
}

int
FloatingImage::get_anchor_point() const
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    return static_cast<int>(floating_image->get_anchor_point());
  else
    return 0;
}

bool
FloatingImage::get_visible() const
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    return floating_image->get_visible();
  else
    return false;
}

void
FloatingImage::set_visible(bool visible)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->set_visible(visible);
}

void
FloatingImage::set_action(const std::string& action)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->set_action(action);
}

std::string
FloatingImage::get_action() const
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    return floating_image->get_action();
  else
    return {};
}

void
FloatingImage::fade_in(float fadetime)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->fade_in(fadetime);
}

void
FloatingImage::fade_out(float fadetime)
{
  ::FloatingImage* floating_image = get_floating_image();
  if (floating_image)
    floating_image->fade_out(fadetime);
}

} // scripting

/* EOF */
