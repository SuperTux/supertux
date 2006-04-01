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

#include <stdexcept>
#include "msg.hpp"
#include "video/drawing_context.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "path.hpp"
#include "path_walker.hpp"
#include "sprite/sprite_manager.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"

Platform::Platform(const lisp::Lisp& reader)
{
  std::string sprite_name;
  reader.get("sprite", sprite_name);
  if(sprite_name == "")
    throw std::runtime_error("No sprite specified in platform object"); 
  sprite.reset(sprite_manager->create(sprite_name));

  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if(pathLisp == NULL)
    throw std::runtime_error("No path specified for platform");
  path.reset(new Path());
  path->read(*pathLisp);
  walker.reset(new PathWalker(path.get()));

  bbox.p1 = path->get_base();
  bbox.set_size(sprite->get_width(), sprite->get_height());
  
  set_group(COLGROUP_STATIC);
  flags |= FLAG_SOLID;
}

Platform::~Platform()
{
}

//TODO: Squish Tux when standing between platform and solid tile/object
//      Improve collision handling
//      Move all MovingObjects lying on the platform instead of only the player
HitResponse
Platform::collision(GameObject& other, const CollisionHit& hit)
{
  if (typeid(other) == typeid(Player)) {
    if (hit.normal.y >= 0.9) {
      //Tux is standing on the platform
      //Player* player = (Player*) &other;
      //player->add_velocity(speed * 1.5);
      return PASS_MOVEMENT;
    }
  }
  if(other.get_flags() & FLAG_SOLID) {
    //Collision with a solid tile
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

void
Platform::update(float elapsed_time)
{
  movement = walker->advance(elapsed_time);
  speed = movement / elapsed_time;
}

void
Platform::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

IMPLEMENT_FACTORY(Platform, "platform");
