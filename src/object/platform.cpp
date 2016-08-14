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

#include "object/platform.hpp"

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "scripting/platform.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Platform::Platform(const ReaderMapping& reader) :
  Platform(reader, "images/objects/flying_platform/flying_platform.sprite")
{
}

Platform::Platform(const ReaderMapping& reader, const std::string& default_sprite) :
  MovingSprite(reader, default_sprite, LAYER_OBJECTS, COLGROUP_STATIC),
  path(),
  walker(),
  speed(Vector(0,0)),
  automatic(false),
  player_contact(false),
  last_player_contact(false)
{
  bool running = true;
  reader.get("name", name);
  reader.get("running", running);
  if ((name.empty()) && (!running)) {
    automatic = true;
  }

  ReaderMapping path_mapping;
  if (!reader.get("path", path_mapping))
  {
    path.reset(new Path(bbox.p1));
    walker.reset(new PathWalker(path.get(), running));
  }
  else
  {
    path.reset(new Path());
    path->read(path_mapping);
    walker.reset(new PathWalker(path.get(), running));
    bbox.set_pos(path->get_base());
  }
}

void
Platform::save(Writer& writer) {
  MovingSprite::save(writer);
  writer.write("running", walker->is_moving());
  path->save(writer);
}

ObjectSettings
Platform::get_settings() {
  ObjectSettings result = MovingSprite::get_settings();
  result.options.push_back( Path::get_mode_option(&path->mode) );
  result.options.push_back( PathWalker::get_running_option(&walker->running) );
  return result;
}

/*
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
*/

HitResponse
Platform::collision(GameObject& other, const CollisionHit& )
{
  if (dynamic_cast<Player*>(&other)) player_contact = true;
  return FORCE_MOVE;
}

void
Platform::update(float elapsed_time)
{
  if (!path->is_valid()) {
    remove_me();
    return;
  }

  // check if Platform should automatically pick a destination
  if (automatic) {

    if (!player_contact && !walker->is_moving()) {
      // Player doesn't touch platform and Platform is not moving

      // Travel to node nearest to nearest player
      auto player = Sector::current()->get_nearest_player(bbox);
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

  Vector new_pos = walker->advance(elapsed_time);
  if (Editor::is_active()) {
    set_pos(new_pos);
  } else {
    movement = new_pos - get_pos();
    speed = movement / elapsed_time;
  }

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
  auto _this = new scripting::Platform(this);
  expose_object(vm, table_idx, _this, name, true);
}

void
Platform::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  scripting::unexpose_object(vm, table_idx, name);
}

void
Platform::move_to(const Vector& pos)
{
  Vector shift = pos - bbox.p1;
  if (path) {
    path->move_by(shift);
  }
  set_pos(pos);
}

/* EOF */
