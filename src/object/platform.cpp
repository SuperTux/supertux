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
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Platform::Platform(const ReaderMapping& reader) :
  Platform(reader, "images/objects/flying_platform/flying_platform.sprite")
{
}

Platform::Platform(const ReaderMapping& reader, const std::string& default_sprite) :
  MovingSprite(reader, default_sprite, LAYER_OBJECTS, COLGROUP_STATIC),
  ExposedObject<Platform, scripting::Platform>(this),
  PathObject(),
  m_speed(Vector(0,0)),
  m_automatic(false),
  m_player_contact(false),
  m_last_player_contact(false)
{
  bool running = false;
  reader.get("running", running);
  if ((m_name.empty()) && (!running)) {
    m_automatic = true;
  }

  init_path(reader, false);
}

void
Platform::finish_construction()
{
  log_fatal << "finish_construction: " << get_path() << std::endl;
  if (!get_path())
  {
    // If no path is given, make a one-node dummy path
    init_path_pos(m_bbox.p1, false);
  }

  m_bbox.set_pos(get_path()->get_base());
}

void
Platform::save(Writer& writer)
{
  MovingSprite::save(writer);
  writer.write("running", get_walker()->is_moving());
  if (get_path()) {
    writer.write("path-ref", get_path_ref());
  }
}

ObjectSettings
Platform::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  result.options.push_back( Path::get_mode_option(&get_path()->m_mode) );
  result.options.push_back( PathWalker::get_running_option(&get_walker()->m_running) );
  return result;
}

HitResponse
Platform::collision(GameObject& other, const CollisionHit& )
{
  if (dynamic_cast<Player*>(&other)) m_player_contact = true;
  return FORCE_MOVE;
}

void
Platform::update(float elapsed_time)
{
  if (!get_path()->is_valid()) {
    remove_me();
    return;
  }

  // check if Platform should automatically pick a destination
  if (m_automatic) {

    if (!m_player_contact && !get_walker()->is_moving()) {
      // Player doesn't touch platform and Platform is not moving

      // Travel to node nearest to nearest player
      auto player = Sector::get().get_nearest_player(m_bbox);
      if (player) {
        int nearest_node_id = get_path()->get_nearest_node_no(player->get_bbox().p2);
        if (nearest_node_id != -1) {
          goto_node(nearest_node_id);
        }
      }
    }

    if (m_player_contact && !m_last_player_contact && !get_walker()->is_moving()) {
      // Player touched platform, didn't touch last frame and Platform is not moving

      // Travel to node farthest from current position
      int farthest_node_id = get_path()->get_farthest_node_no(get_pos());
      if (farthest_node_id != -1) {
        goto_node(farthest_node_id);
      }
    }

    // Clear player_contact flag set by collision() method
    m_last_player_contact = m_player_contact;
    m_player_contact = false;
  }

  Vector new_pos = get_walker()->advance(elapsed_time);
  if (Editor::is_active()) {
    set_pos(new_pos);
  } else {
    m_movement = new_pos - get_pos();
    m_speed = m_movement / elapsed_time;
  }

}

void
Platform::goto_node(int node_no)
{
  get_walker()->goto_node(node_no);
}

void
Platform::start_moving()
{
  get_walker()->start_moving();
}

void
Platform::stop_moving()
{
  get_walker()->stop_moving();
}

void
Platform::move_to(const Vector& pos)
{
  Vector shift = pos - m_bbox.p1;
  if (get_path()) {
    get_path()->move_by(shift);
  }
  set_pos(pos);
}

/* EOF */
