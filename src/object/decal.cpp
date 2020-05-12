//  SuperTux - Decal
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#include "object/decal.hpp"

#include "util/reader.hpp"
#include "util/reader_mapping.hpp"

Decal::Decal(const ReaderMapping& reader) :
  MovingSprite(reader, "images/decal/explanations/billboard-bigtux.png", LAYER_OBJECTS, COLGROUP_DISABLED),
  default_action("default"),
  solid()
{
  m_layer = reader_get_layer(reader, LAYER_OBJECTS);

  reader.get("solid", solid, false);
  if (solid)
    set_group(COLGROUP_STATIC);
  if (reader.get("action", default_action))
    set_action(default_action, -1);
}

ObjectSettings
Decal::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_int(_("Z-pos"), &m_layer, "z-pos", LAYER_OBJECTS);
  result.add_bool(_("Solid"), &solid, "solid", false);
  result.add_text(_("Action"), &default_action, "action", std::string("default"));

  result.reorder({"z-pos", "sprite", "x", "y"});

  return result;
}

Decal::~Decal()
{
}

/* EOF */
