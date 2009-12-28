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
  if (dir == AUTO) log_warning << "Setting an Ispy's direction to AUTO is no good idea" << std::endl;

  // set initial sprite action
  sprite->set_action((dir == DOWN) ? "idle-down" : ((dir == LEFT) ? "idle-left" : "idle-right"));
}

HitResponse
Ispy::collision(GameObject& , const CollisionHit& )
{
  return ABORT_MOVE;
}

bool
Ispy::line_intersects_line(Vector line1_start, Vector line1_end, Vector line2_start, Vector line2_end) {
  // Adapted from Striker, (C) 1999 Joris van der Hoeven, GPL

  float a1 = line1_start.x, b1 = line1_start.y, a2 = line1_end.x, b2 = line1_end.y;
  float c1 = line2_start.x, d1 = line2_start.y, c2 = line2_end.x, d2 = line2_end.y;

  float num = (b2-b1)*(c2-c1) - (a2-a1)*(d2-d1);
  float den1 = (d2-b2)*(c1-c2) + (a2-c2)*(d1-d2);
  float den2 = (d2-b2)*(a1-a2) + (a2-c2)*(b1-b2);

  // normalize to positive numerator
  if (num < 0) { 
    num =- num; 
    den1 =- den1; 
    den2 =- den2; 
  }

  // numerator is zero -> Check for parallel or coinciding lines
  if (num == 0) {
    if ((b1-b2)*(c1-a2) != (a1-a2)*(d1-b2)) return false;
    if (a1 == a2) { 
      std::swap(a1, b1); 
      std::swap(a2, b2); 
      std::swap(c1, d1); 
      std::swap(c2, d2); 
    }
    if (a1 > a2) std::swap(a1, a2);
    if (c1 > c2) std::swap(c1, c2);
    return ((a1 <= c2) && (a2 >= c1));
  }

  // Standard check
  return (den1>=0) && (den1<=num) && (den2>=0) && (den2<=num);

}

bool
Ispy::intersects_line(Rectf r, Vector line_start, Vector line_end)
{
  Vector p1 = r.p1;
  Vector p2 = Vector(r.p2.x, r.p1.y);
  Vector p3 = r.p2;
  Vector p4 = Vector(r.p1.x, r.p2.y);
  if (line_intersects_line(p1, p2, line_start, line_end)) return true;
  if (line_intersects_line(p2, p3, line_start, line_end)) return true;
  if (line_intersects_line(p3, p4, line_start, line_end)) return true;
  if (line_intersects_line(p4, p1, line_start, line_end)) return true;
  return false;
}

bool
Ispy::free_line_of_sight(Vector line_start, Vector line_end, const MovingObject* ignore_object)
{

  // check if no tile is in the way
  float lsx = std::min(line_start.x, line_end.x);
  float lex = std::max(line_start.x, line_end.x);
  float lsy = std::min(line_start.y, line_end.y);
  float ley = std::max(line_start.y, line_end.y);
  std::list<TileMap*> solid_tilemaps = Sector::current()->solid_tilemaps;
  for (float test_x = lsx; test_x <= lex; test_x += 16) {
    for (float test_y = lsy; test_y <= ley; test_y += 16) {
      for(std::list<TileMap*>::const_iterator i = solid_tilemaps.begin(); i != solid_tilemaps.end(); i++) {
        TileMap* solids = *i;
        const Tile* tile = solids->get_tile_at(Vector(test_x, test_y));
        if(!tile) continue;
        // FIXME: check collision with slope tiles
        if((tile->getAttributes() & Tile::SOLID)) return false;
      }
    }
  }

  // check if no object is in the way
  using namespace collision;
  Sector::MovingObjects& moving_objects = Sector::current()->moving_objects;
  for(Sector::MovingObjects::const_iterator i = moving_objects.begin();
      i != moving_objects.end(); ++i) {
    const MovingObject* moving_object = *i;
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if ((moving_object->get_group() == COLGROUP_MOVING)
        || (moving_object->get_group() == COLGROUP_MOVING_STATIC)
        || (moving_object->get_group() == COLGROUP_STATIC)) {
      if(intersects_line(moving_object->get_bbox(), line_start, line_end)) return false;
    }
  }

  return true;
}

void 
Ispy::update(float )
{

  if (state == ISPYSTATE_IDLE) {
    // check if a player has been spotted
    bool playerSpotted = false;
    std::vector<Player*> players = Sector::current()->get_players();
    for (std::vector<Player*>::iterator playerIter = players.begin(); playerIter != players.end(); ++playerIter) {
      Player* player = *playerIter;

      Vector eye = get_bbox().get_middle();
      if (dir == LEFT) eye = Vector(get_bbox().p1.x, get_bbox().get_middle().y);
      if (dir == RIGHT) eye = Vector(get_bbox().p2.x, get_bbox().get_middle().y);
      if (dir == UP) eye = Vector(get_bbox().get_middle().x, get_bbox().p1.y);
      if (dir == DOWN) eye = Vector(get_bbox().get_middle().x, get_bbox().p2.y);

      // test for free line of sight to any of all four corners and the middle of a player's bounding box
      if (free_line_of_sight(eye, player->get_bbox().p1, player)) playerSpotted = true;
      if (free_line_of_sight(eye, Vector(player->get_bbox().p2.x, player->get_bbox().p1.y), player)) playerSpotted = true;
      if (free_line_of_sight(eye, player->get_bbox().p2, player)) playerSpotted = true;
      if (free_line_of_sight(eye, Vector(player->get_bbox().p1.x, player->get_bbox().p2.y), player)) playerSpotted = true;
      if (free_line_of_sight(eye, player->get_bbox().get_middle(), player)) playerSpotted = true;
    }

    if (playerSpotted) {
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
