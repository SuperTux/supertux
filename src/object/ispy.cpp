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

#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader.hpp"

#include <sstream>

Ispy::Ispy(const Reader& reader) :
  MovingSprite(reader, "images/objects/ispy/ispy.sprite", LAYER_TILES+5, COLGROUP_DISABLED),
  state(ISPYSTATE_IDLE),
  script(),
  dir(AUTO)
{
  // read script to execute
  reader.get("script", script);

  // read direction to face in
  std::string dir_str;
  bool facing_down = false;
  reader.get("direction", dir_str);
  if( dir_str == "left" ) dir = LEFT;
  if( dir_str == "right" ) dir = RIGHT;
  reader.get("facing-down", facing_down);
  if (facing_down) dir = DOWN;
  if (dir == AUTO) { log_warning << "Setting an Ispy's direction to AUTO is no good idea" << std::endl; }

  // set initial sprite action
  sprite->set_action((dir == DOWN) ? "idle-down" : ((dir == LEFT) ? "idle-left" : "idle-right"));
}

void
Ispy::save(lisp::Writer& writer) {
  MovingSprite::save(writer);
  switch (dir) {
    case LEFT:  writer.write("direction", "left" , false); break;
    case RIGHT: writer.write("direction", "right", false); break;
    case DOWN:  writer.write("facing_down", true); break;
    case AUTO: break;
    case UP: break;
  }
  writer.write("script", script, false);
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
    Vector eye = get_bbox().get_middle();
    if (dir == LEFT) eye = Vector(get_bbox().p1.x, get_bbox().get_middle().y);
    if (dir == RIGHT) eye = Vector(get_bbox().p2.x, get_bbox().get_middle().y);
    if (dir == UP) eye = Vector(get_bbox().get_middle().x, get_bbox().p1.y);
    if (dir == DOWN) eye = Vector(get_bbox().get_middle().x, get_bbox().p2.y);

    if (Sector::current()->can_see_player(eye)) {
      sprite->set_action((dir == DOWN) ? "alert-down" : ((dir == LEFT) ? "alert-left" : "alert-right"), 1);
      state = ISPYSTATE_ALERT;

      std::istringstream stream(script);
      Sector::current()->run_script(stream, "Ispy");
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
