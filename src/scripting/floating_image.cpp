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
  floating_image(std::make_shared<::FloatingImage>(spritefile))
{
  using namespace worldmap;

  if(Sector::current() != NULL) {
    Sector::current()->add_object(floating_image);
  } else if(WorldMap::current() != NULL) {
    WorldMap::current()->add_object(floating_image);
  } else {
    throw new std::runtime_error("Neither sector nor worldmap active");
  }
}

FloatingImage::~FloatingImage()
{
  floating_image->remove_me();
}

void
FloatingImage::set_layer(int layer)
{
  floating_image->set_layer(layer);
}

int
FloatingImage::get_layer()
{
  return floating_image->get_layer();
}

void
FloatingImage::set_pos(float x, float y)
{
  floating_image->set_pos(Vector(x, y));
}

float
FloatingImage::get_pos_x()
{
  return floating_image->get_pos().x;
}

float
FloatingImage::get_pos_y()
{
  return floating_image->get_pos().y;
}

void
FloatingImage::set_anchor_point(int anchor)
{
  floating_image->set_anchor_point((AnchorPoint) anchor);
}

int
FloatingImage::get_anchor_point()
{
  return (int) floating_image->get_anchor_point();
}

bool
FloatingImage::get_visible()
{
  return floating_image->get_visible();
}

void
FloatingImage::set_visible(bool visible)
{
  floating_image->set_visible(visible);
}

void
FloatingImage::set_action(const std::string& action)
{
  floating_image->set_action(action);
}

std::string
FloatingImage::get_action()
{
  return floating_image->get_action();
}

void
FloatingImage::fade_in(float fadetime)
{
  floating_image->fade_in(fadetime);
}

void
FloatingImage::fade_out(float fadetime)
{
  floating_image->fade_out(fadetime);
}

}

/* EOF */
