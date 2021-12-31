//  SuperTux - Ispy
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "object/ispy.hpp"

#include "editor/editor.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Ispy::Ispy(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/ispy/ispy.sprite", LAYER_TILES+5, COLGROUP_DISABLED),
  state(ISPYSTATE_IDLE),
  script(),
  dir(Direction::AUTO),
  m_facing_down(false)
{
  // read script to execute
  reader.get("script", script);

  // read direction to face in
  std::string dir_str;
  if (reader.get("direction", dir_str)) {
    dir = string_to_dir(dir_str);
  } else {
    if (!Editor::is_active()) {
      dir = Direction::LEFT;
    }
  }

  reader.get("facing-down", m_facing_down, false);
  if (!Editor::is_active()) {
    if (m_facing_down) {
      dir = Direction::DOWN;
    }
  }

  if (dir == Direction::AUTO) {
    log_warning << "Setting an Ispy's direction to AUTO is no good idea" << std::endl;
  }

  // set initial sprite action
  m_sprite->set_action((dir == Direction::DOWN) ? "idle-down" : ((dir == Direction::LEFT) ? "idle-left" : "idle-right"));
}

ObjectSettings
Ispy::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_bool(_("Facing Down"), &m_facing_down, "facing-down", false);
  result.add_script(_("Script"), &script, "script");
  result.add_direction(_("Direction"), &dir, Direction::AUTO, "direction");

  result.reorder({"script", "facing-down", "direction", "x", "y"});

  return result;
}

void
Ispy::after_editor_set()
{
  MovingSprite::after_editor_set();
  m_sprite->set_action((dir == Direction::DOWN) ? "idle-down" : ((dir == Direction::LEFT) ? "idle-left" : "idle-right"));
}

HitResponse
Ispy::collision(GameObject& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
Ispy::update(float dt_sec)
{

  if (state == ISPYSTATE_IDLE) {
    // check if a player has been spotted
    Vector eye = m_col.m_bbox.get_middle();
    if (dir == Direction::LEFT) eye = Vector(m_col.m_bbox.get_left(), m_col.m_bbox.get_middle().y);
    else if (dir == Direction::RIGHT) eye = Vector(m_col.m_bbox.get_right(), m_col.m_bbox.get_middle().y);
    else if (dir == Direction::UP || (dir == Direction::DOWN && m_flip != NO_FLIP)) eye = Vector(m_col.m_bbox.get_middle().x, m_col.m_bbox.get_top());
    else if (dir == Direction::DOWN) eye = Vector(m_col.m_bbox.get_middle().x, m_col.m_bbox.get_bottom());

    if (Sector::get().can_see_player(eye)) {
      m_sprite->set_action((dir == Direction::DOWN) ? "alert-down" : ((dir == Direction::LEFT) ? "alert-left" : "alert-right"), 1);
      state = ISPYSTATE_ALERT;
    }
  }
  if (state == ISPYSTATE_ALERT) {
    if (m_sprite->animation_done()) {
      m_sprite->set_action((dir == Direction::DOWN) ? "hiding-down" : ((dir == Direction::LEFT) ? "hiding-left" : "hiding-right"), 1);
      state = ISPYSTATE_HIDING;
	  
	  Sector::get().run_script(script, "Ispy");
    }
  }
  if (state == ISPYSTATE_HIDING) {
    if (m_sprite->animation_done()) {
      m_sprite->set_action((dir == Direction::DOWN) ? "showing-down" : ((dir == Direction::LEFT) ? "showing-left" : "showing-right"), 1);
      state = ISPYSTATE_SHOWING;
    }
  }
  if (state == ISPYSTATE_SHOWING) {
    if (m_sprite->animation_done()) {
      m_sprite->set_action((dir == Direction::DOWN) ? "idle-down" : ((dir == Direction::LEFT) ? "idle-left" : "idle-right"));
      state = ISPYSTATE_IDLE;
    }
  }
}

void
Ispy::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
