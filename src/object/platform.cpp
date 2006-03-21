//  $Id$
//
//  SuperTux
//  Copyright (C) 2005 Marek Moeckel <wansti@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "platform.hpp"

#include <iostream>
#include "video/drawing_context.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "sprite/sprite_manager.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"

Platform::Platform(const lisp::Lisp& reader)
{
  std::string use_path;
  std::string type;

  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  reader.get("type", type);
  reader.get("use_path", use_path);
  sprite = sprite_manager->create("images/objects/flying_platform/platform.sprite");
  sprite->set_action(type);
  bbox.set_size(sprite->get_width(), sprite->get_height());

  flags |= FLAG_SOLID;

  path = Path::GetByName(use_path);
  if (path == NULL) { 
     std::cerr << "Warning: Path for moving platform not found! Make sure that the name is spelled correctly,\nand that the path is initialized before the platform in the level file!\n";
  }

  path_offset = bbox.p1;
}

Platform::~Platform()
{
  delete sprite;
}

//TODO: Squish Tux when standing between platform and solid tile/object
//      Improve collision handling
//      Move all MovingObjects lying on the platform instead of only the player
HitResponse
Platform::collision(GameObject& other, const CollisionHit& hit)
{
  if (typeid(other) == typeid(Player)) {
    Player* player = (Player*) &other;
    if ((hit.normal.x == 0) && (hit.normal.y == 1)) {
      //Tux is standing on the platform
      player->movement += path->GetLastMovement();
    }
  }
  if(other.get_flags() & FLAG_SOLID) {
    //Collision with a solid tile
    //does nothing, because the movement vector isn't used at the moment
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

void
Platform::update(float )
{
  set_pos(path->GetPosition() + path_offset);
}

void
Platform::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

IMPLEMENT_FACTORY(Platform, "platform");
