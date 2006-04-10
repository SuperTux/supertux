//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <stdexcept>
#include <sstream>
#include <cmath>

#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "lisp/list_iterator.hpp"
#include "scripting/camera.hpp"
#include "scripting/wrapper_util.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "tilemap.hpp"
#include "game_session.hpp"
#include "sector.hpp"
#include "main.hpp"
#include "object_factory.hpp"
#include "msg.hpp"
#include "path.hpp"
#include "path_walker.hpp"

Camera::Camera(Sector* newsector)
  : sector(newsector), do_backscrolling(true), scrollchange(NONE)
{
  mode = NORMAL;
}

Camera::~Camera()
{
}

void
Camera::expose(HSQUIRRELVM vm, int table_idx)
{
  Scripting::Camera* interface = new Scripting::Camera(this);
  expose_object(vm, table_idx, interface, "Camera", true);
}

void
Camera::unexpose(HSQUIRRELVM vm, int table_idx)
{
  Scripting::unexpose_object(vm, table_idx, "Camera");
}

const Vector&
Camera::get_translation() const
{
  return translation;
}

void
Camera::parse(const lisp::Lisp& reader)
{
  std::string modename;
  
  reader.get("mode", modename);
  if(modename == "normal") {
    mode = NORMAL;

    do_backscrolling = true;
    reader.get("backscrolling", do_backscrolling);
  } else if(modename == "autoscroll") {
    mode = AUTOSCROLL;

    const lisp::Lisp* pathLisp = reader.get_lisp("path");
    if(pathLisp == NULL)
      throw std::runtime_error("No path specified in autoscroll camera.");

    autoscroll_path.reset(new Path());
    autoscroll_path->read(*pathLisp);
    autoscroll_walker.reset(new PathWalker(autoscroll_path.get()));
  } else if(modename == "manual") {
    mode = MANUAL;
  } else {
    std::stringstream str;
    str << "invalid camera mode '" << modename << "'found in worldfile.";
    throw std::runtime_error(str.str());
  }
}

void
Camera::write(lisp::Writer& writer)
{
  writer.start_list("camera");
  
  if(mode == NORMAL) {
    writer.write_string("mode", "normal");
    writer.write_bool("backscrolling", do_backscrolling);
  } else if(mode == AUTOSCROLL) {
    writer.write_string("mode", "autoscroll");
    autoscroll_path->write(writer);
  } else if(mode == MANUAL) {
    writer.write_string("mode", "manual");
  }
                     
  writer.end_list("camera");
}

void
Camera::reset(const Vector& tuxpos)
{
  translation.x = tuxpos.x - SCREEN_WIDTH/3 * 2;
  translation.y = tuxpos.y - SCREEN_HEIGHT/2;
  shakespeed = 0;
  shaketimer.stop();
  keep_in_bounds(translation);
}

void
Camera::shake(float time, float x, float y)
{
  shaketimer.start(time);
  shakedepth_x = x;
  shakedepth_y = y;
  shakespeed = M_PI/2 / time;
}

void
Camera::scroll_to(const Vector& goal, float scrolltime)
{
  scroll_from = translation;
  scroll_goal = goal;
  keep_in_bounds(scroll_goal);

  scroll_to_pos = 0;
  scrollspeed = 1.0 / scrolltime;
  mode = SCROLLTO;
}

static const float EPSILON = .00001;
static const float max_speed_y = 140;

void
Camera::update(float elapsed_time)
{
  switch(mode) {
    case NORMAL:
      update_scroll_normal(elapsed_time);
      break;
    case AUTOSCROLL:
      update_scroll_autoscroll(elapsed_time);
      break;
    case SCROLLTO:
      update_scroll_to(elapsed_time);
      break;
    default:
      break;
  }
}

void
Camera::keep_in_bounds(Vector& translation)
{
  float width = sector->solids->get_width() * 32;
  float height = sector->solids->get_height() * 32;

  // don't scroll before the start or after the level's end
  if(translation.y > height - SCREEN_HEIGHT)
    translation.y = height - SCREEN_HEIGHT;
  if(translation.y < 0)                                      
    translation.y = 0; 
  if(translation.x > width - SCREEN_WIDTH)
    translation.x = width - SCREEN_WIDTH;
  if(translation.x < 0)
    translation.x = 0;                                         
}

void
Camera::shake()
{
  if(shaketimer.started()) {
    translation.x -= sin(shaketimer.get_timegone() * shakespeed) * shakedepth_x;
    translation.y -= sin(shaketimer.get_timegone() * shakespeed) * shakedepth_y;
  }
}

void
Camera::update_scroll_normal(float elapsed_time)
{
  assert(sector != 0);
  Player* player = sector->player;
  
  // check that we don't have division by zero later
  if(elapsed_time < EPSILON)
    return;

  /****** Vertical Scrolling part ******/
  bool do_y_scrolling = true;

  if(player->is_dying() || sector->solids->get_height() == 19)
    do_y_scrolling = false;

  if(do_y_scrolling) {
    // target_y is the high we target our scrolling at. This is not always the
    // high of the player, but if he is jumping upwards we should use the
    // position where he last touched the ground. (this probably needs
    // exceptions for trampolines and similar things in the future)
    float target_y;
    if(player->fall_mode == Player::JUMPING)
      target_y = player->last_ground_y + player->get_bbox().get_height();
    else
      target_y = player->get_bbox().p2.y;

    // delta_y is the distance we'd have to travel to directly reach target_y
    float delta_y = translation.y - (target_y - SCREEN_HEIGHT*2/3);
    // speed is the speed the camera would need to reach target_y in this frame
    float speed_y = delta_y / elapsed_time;

    // limit the camera speed when jumping upwards
    if(player->fall_mode != Player::FALLING 
        && player->fall_mode != Player::TRAMPOLINE_JUMP) {
      if(speed_y > max_speed_y)
        speed_y = max_speed_y;
      else if(speed_y < -max_speed_y)
        speed_y = -max_speed_y;
    }

    // finally scroll with calculated speed
    translation.y -= speed_y * elapsed_time;
  }

  /****** Horizontal scrolling part *******/

  // our camera is either in leftscrolling, rightscrolling or nonscrollingmode.
  
  // when suddenly changing directions while scrolling into the other direction.
  // abort scrolling, since tux might be going left/right at a relatively small
  // part of the map (like when jumping upwards)
  if((player->dir == ::LEFT && scrollchange == RIGHT)
      || (player->dir == ::RIGHT && scrollchange == LEFT))
    scrollchange = NONE;
  // when in left 1/3rd of screen scroll left
  if(player->get_bbox().get_middle().x < translation.x + SCREEN_WIDTH/3 - 16
      && do_backscrolling)
    scrollchange = LEFT;
  // scroll right when in right 1/3rd of screen
  else if(player->get_bbox().get_middle().x > translation.x + SCREEN_WIDTH/3*2+16)
    scrollchange = RIGHT;

  // calculate our scroll target depending on scroll mode
  float target_x;
  if(scrollchange == LEFT)
    target_x = player->get_bbox().get_middle().x - SCREEN_WIDTH/3*2;
  else if(scrollchange == RIGHT)
    target_x = player->get_bbox().get_middle().x - SCREEN_WIDTH/3;
  else
    target_x = translation.x;

  // that's the distance we would have to travel to reach target_x
  float delta_x = translation.x - target_x;
  // the speed we'd need to travel to reach target_x in this frame
  float speed_x = delta_x / elapsed_time;

  // limit our speed
  float maxv = 130 + (fabsf(player->physic.get_velocity_x() * 1.3));
  if(speed_x > maxv)
    speed_x = maxv;
  else if(speed_x < -maxv)
    speed_x = -maxv;
 
  // apply scrolling
  translation.x -= speed_x * elapsed_time;

  keep_in_bounds(translation);
  shake();
}

void
Camera::update_scroll_autoscroll(float elapsed_time)
{
  Player* player = sector->player; 
  if(player->is_dying())
    return;

  translation += autoscroll_walker->advance(elapsed_time);

  keep_in_bounds(translation);
  shake();
}

void
Camera::update_scroll_to(float elapsed_time)
{
  scroll_to_pos += elapsed_time * scrollspeed;
  if(scroll_to_pos >= 1.0) {
    mode = MANUAL;
    translation = scroll_goal;
    return;
  }

  translation = scroll_from + (scroll_goal - scroll_from) * scroll_to_pos;
}

