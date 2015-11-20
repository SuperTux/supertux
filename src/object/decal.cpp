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

Decal::Decal(const ReaderMapping& reader) :
  MovingSprite(reader, LAYER_OBJECTS, COLGROUP_DISABLED)
{
  layer = reader_get_layer (reader, /* default = */ LAYER_OBJECTS);

  bool solid = false;
  reader.get("solid", solid);
  if(solid)
    set_group(COLGROUP_STATIC);
  std::string action;
  if(reader.get("action", action))
    set_action(action, -1);
}

Decal::~Decal()
{
}

/* EOF */
