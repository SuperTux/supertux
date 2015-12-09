//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
#include <config.h>

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"
#include "worldmap/special_tile.hpp"

namespace worldmap {

SpecialTile::SpecialTile(const Reader& lisp) :
  pos(),
  sprite(),
  map_message(),
  passive_message(false),
  script(),
  invisible(false),
  apply_action_north(true),
  apply_action_east(true),
  apply_action_south(true),
  apply_action_west(true)
{
  if(!lisp.get("x", pos.x)) {
    log_warning << "X coordinate of special tile not set, defaulting to 0" << std::endl;
  }
  if(!lisp.get("y", pos.y)) {
    log_warning << "Y coordinate of special tile not set, defaulting to 0" << std::endl;
  }
  if(!lisp.get("invisible-tile", invisible)) {
    // Ignore attribute if it's not specified. Tile is visible.
  }

  if(!invisible) {
    std::string spritefile = "";
    if(!lisp.get("sprite", spritefile)) {
      log_warning << "No sprite specified for visible special tile." << std::endl;
    }
    sprite = SpriteManager::current()->create(spritefile);
  }

  if(!lisp.get("map-message", map_message)) {
    // Ignore attribute if it's not specified. No map message set.
  }
  if(!lisp.get("passive-message", passive_message)) {
    // Ignore attribute if it's not specified. No passive message set.
  }
  if(!lisp.get("script", script)) {
    // Ignore attribute if it's not specified. No script set.
  }

  std::string apply_direction;
  if(!lisp.get("apply-to-direction", apply_direction)) {
    // Ignore attribute if it's not specified. Applies to all directions.
  }
  if(!apply_direction.empty()) {
    apply_action_north = false;
    apply_action_south = false;
    apply_action_east = false;
    apply_action_west = false;
    if(apply_direction.find("north") != std::string::npos)
      apply_action_north = true;
    if(apply_direction.find("south") != std::string::npos)
      apply_action_south = true;
    if(apply_direction.find("east") != std::string::npos)
      apply_action_east = true;
    if(apply_direction.find("west") != std::string::npos)
      apply_action_west = true;
  }
}

SpecialTile::~SpecialTile()
{
}

void
SpecialTile::draw(DrawingContext& context)
{
  if(invisible)
    return;

  sprite->draw(context, pos*32 + Vector(16, 16), LAYER_OBJECTS - 1);
}

void
SpecialTile::update(float )
{
}

}

/* EOF */
