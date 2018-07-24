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

#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Ispy::Ispy(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/ispy/ispy.sprite", LAYER_TILES+5, COLGROUP_DISABLED),
  state(ISPYSTATE_IDLE),
  script(),
  dir(AUTO)
{
  // read script to execute
  reader.get("script", script);

  // read direction to face in
  std::string dir_str;
  bool facing_down;
  if (reader.get("direction", dir_str)) {
    dir = string_to_dir(dir_str);
  } else {
    dir = LEFT;
  }
  reader.get("facing-down", facing_down, false);
  if (facing_down) dir = DOWN;
  if (dir == AUTO) { log_warning << "Setting an Ispy's direction to AUTO is no good idea" << std::endl; }

  // set initial sprite action
  sprite->set_action((dir == DOWN) ? "idle-down" : ((dir == LEFT) ? "idle-left" : "idle-right"));
}

void
Ispy::save(Writer& writer) {
  MovingSprite::save(writer);
  if(dir != AUTO) {
    writer.write("direction", dir_to_string(dir), false);
  }
}

ObjectSettings
Ispy::get_settings() {
  ObjectSettings result = MovingSprite::get_settings();
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Script"), &script, "script"));
  result.options.push_back( dir_option(&dir) );

  return result;
}

void
Ispy::after_editor_set()
{
  MovingSprite::after_editor_set();
  sprite->set_action((dir == DOWN) ? "idle-down" : ((dir == LEFT) ? "idle-left" : "idle-right"));
}

HitResponse
Ispy::collision(GameObject& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
Ispy::update(float )
{

  if (state == ISPYSTATE_IDLE) {
    // check if a player has been spotted
    Vector eye = bbox.get_middle();
    if (dir == LEFT) eye = Vector(bbox.p1.x, bbox.get_middle().y);
    if (dir == RIGHT) eye = Vector(bbox.p2.x, bbox.get_middle().y);
    if (dir == UP) eye = Vector(bbox.get_middle().x, bbox.p1.y);
    if (dir == DOWN) eye = Vector(bbox.get_middle().x, bbox.p2.y);

    if (Sector::current()->can_see_player(eye)) {
      sprite->set_action((dir == DOWN) ? "alert-down" : ((dir == LEFT) ? "alert-left" : "alert-right"), 1);
      state = ISPYSTATE_ALERT;

      Sector::current()->run_script(script, "Ispy");
    }
  }
  if (state == ISPYSTATE_ALERT) {
    if (sprite->animation_done()) {
      sprite->set_action((dir == DOWN) ? "hiding-down" : ((dir == LEFT) ? "hiding-left" : "hiding-right"), 1);
      state = ISPYSTATE_HIDING;
    }
  }
  if (state == ISPYSTATE_HIDING) {
    if (sprite->animation_done()) {
      sprite->set_action((dir == DOWN) ? "showing-down" : ((dir == LEFT) ? "showing-left" : "showing-right"), 1);
      state = ISPYSTATE_SHOWING;
    }
  }
  if (state == ISPYSTATE_SHOWING) {
    if (sprite->animation_done()) {
      sprite->set_action((dir == DOWN) ? "idle-down" : ((dir == LEFT) ? "idle-left" : "idle-right"));
      state = ISPYSTATE_IDLE;
    }
  }
}

/* EOF */
