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

#include "supertux/object_factory.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"

Decal::Decal(const ReaderMapping& reader) :
  MovingSprite(reader, "images/decal/explanations/billboard-fireflower.png", LAYER_OBJECTS, COLGROUP_DISABLED),
  default_action(),
  solid()
{
  layer = reader_get_layer (reader, /* default = */ LAYER_OBJECTS);

  if (!reader.get("solid", solid)) solid = false;
  if(solid)
    set_group(COLGROUP_STATIC);
  if(reader.get("action", default_action))
    set_action(default_action, -1);
}

ObjectSettings
Decal::get_settings() {
  ObjectSettings result = MovingObject::get_settings();
  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name, "sprite");
  spr.select.push_back(".png");
  spr.select.push_back(".sprite");
  result.options.push_back(spr);
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Action"), &default_action, "action"));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Solid"), &solid, "solid"));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &layer, "z-pos"));

  return result;
}

Decal::~Decal()
{
}

/* EOF */
