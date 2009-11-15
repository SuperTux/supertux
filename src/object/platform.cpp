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

#include <config.h>

#include "object/platform.hpp"

#include <stdexcept>

#include "lisp/lisp.hpp"
#include "object/path.hpp"
#include "object/path_walker.hpp"
#include "object/player.hpp"
#include "scripting/platform.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"

Platform::Platform(const lisp::Lisp& reader)
        : MovingSprite(reader, Vector(0,0), LAYER_OBJECTS, COLGROUP_STATIC), 
        speed(Vector(0,0)), 
        automatic(false), player_contact(false), last_player_contact(false)
{
  bool running = true;
  reader.get("name", name);
  reader.get("running", running);
  if ((name == "") && (!running)) automatic=true;
  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if(pathLisp == NULL)
    throw std::runtime_error("No path specified for platform");
  path.reset(new Path());
  path->read(*pathLisp);
  walker.reset(new PathWalker(path.get(), running));
  bbox.set_pos(path->get_base());
}

Platform::Platform(const Platform& other) :
  MovingSprite(other), 
  ScriptInterface(other), 
  speed(other.speed), 
  automatic(other.automatic), 
  player_contact(false), 
  last_player_contact(false)
{
  name = other.name;
  path.reset(new Path(*other.path));
  walker.reset(new PathWalker(*other.walker));
  walker->path = &*path;
}

HitResponse
Platform::collision(GameObject& other, const CollisionHit& )
{
  if (dynamic_cast<Player*>(&other)) player_contact = true;
  return FORCE_MOVE;
}

void
Platform::update(float elapsed_time)
{
  // check if Platform should automatically pick a destination
  if (automatic) {

    if (!player_contact && !walker->is_moving()) {
      // Player doesn't touch platform and Platform is not moving

      // Travel to node nearest to nearest player
      // FIXME: does not really use nearest player
      Player* player = 0;      
      std::vector<Player*> players = Sector::current()->get_players();
      for (std::vector<Player*>::iterator playerIter = players.begin(); playerIter != players.end(); ++playerIter) {
        player = *playerIter;
      }
      if (player) {
        int nearest_node_id = path->get_nearest_node_no(player->get_bbox().p2);
        if (nearest_node_id != -1) {
          goto_node(nearest_node_id);
        }
      }
    } 

    if (player_contact && !last_player_contact && !walker->is_moving()) {
      // Player touched platform, didn't touch last frame and Platform is not moving

      // Travel to node farthest from current position
      int farthest_node_id = path->get_farthest_node_no(get_pos());
      if (farthest_node_id != -1) {
        goto_node(farthest_node_id);
      } 
    }

    // Clear player_contact flag set by collision() method
    last_player_contact = player_contact;
    player_contact = false;
  }

  movement = walker->advance(elapsed_time) - get_pos();
  speed = movement / elapsed_time;
}

void
Platform::goto_node(int node_no)
{
  walker->goto_node(node_no);
}

void
Platform::start_moving()
{
  walker->start_moving();
}

void
Platform::stop_moving()
{
  walker->stop_moving();
}

void
Platform::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  Scripting::Platform* interface = new Scripting::Platform(this);
  expose_object(vm, table_idx, interface, name, true);
}

void
Platform::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  Scripting::unexpose_object(vm, table_idx, name);
}

IMPLEMENT_FACTORY(Platform, "platform");
