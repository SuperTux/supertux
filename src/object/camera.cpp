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

#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "lisp/list_iterator.h"
#include "camera.h"
#include "player.h"
#include "tilemap.h"
#include "gameloop.h"
#include "app/globals.h"
#include "sector.h"
#include "object_factory.h"

using namespace SuperTux;

Camera::Camera(Sector* newsector)
  : sector(newsector), do_backscrolling(true), scrollchange(NONE),
    auto_idx(0), auto_t(0)
{
  mode = NORMAL;
}

Camera::~Camera()
{
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
    printf("autoscroll.\n");
    mode = AUTOSCROLL;
    
    const lisp::Lisp* path_lisp = reader.get_lisp("path");
    if(!path_lisp)
      throw std::runtime_error("No path specified in autoscroll camera.");

    lisp::ListIterator iter(path_lisp);
    float speed = .5;
    while(iter.next()) {
      if(iter.item() != "point") {
        std::cerr << "Warning: unknown token '" << iter.item() 
          << "' in camera path.\n";
        continue;
      }
      const lisp::Lisp* point_lisp = iter.lisp();

      ScrollPoint point;
      if(!point_lisp->get("x", point.position.x) ||
         !point_lisp->get("y", point.position.y)) {
        throw std::runtime_error("x and y missing in point of camerapath");
      }
      point_lisp->get("speed", speed);
      point.speed = speed;
      scrollpoints.push_back(point);
    }
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
    writer.start_list("path");
    for(std::vector<ScrollPoint>::iterator i = scrollpoints.begin();
        i != scrollpoints.end(); ++i) {
      writer.start_list("point");
      writer.write_float("x", i->position.x);
      writer.write_float("y", i->position.y);
      writer.write_float("speed", i->speed);
      writer.end_list("point");
    }

    writer.end_list("path");
  } else if(mode == MANUAL) {
    writer.write_string("mode", "manual");
  }
                     
  writer.end_list("camera");
}

void
Camera::reset(const Vector& tuxpos)
{
  translation.x = tuxpos.x - screen->w/3 * 2;
  translation.y = tuxpos.y - screen->h/2;
  keep_in_bounds();
}

static const float EPSILON = .00001;
static const float max_speed_y = 140;

void
Camera::action(float elapsed_time)
{
  if(mode == NORMAL)
    scroll_normal(elapsed_time);
  else if(mode == AUTOSCROLL)
    scroll_autoscroll(elapsed_time);
}

void
Camera::keep_in_bounds()
{
  float width = sector->solids->get_width() * 32;
  float height = sector->solids->get_height() * 32;

  // don't scroll before the start or after the level's end
  if(translation.y > height - screen->h)
    translation.y = height - screen->h;
  if(translation.y < 0)                                      
    translation.y = 0; 
  if(translation.x > width - screen->w)
    translation.x = width - screen->w;
  if(translation.x < 0)
    translation.x = 0;                                         
}

void
Camera::scroll_normal(float elapsed_time)
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
    float delta_y = translation.y - (target_y - screen->h/2);
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
  if(player->get_bbox().get_middle().x < translation.x + screen->w/3 - 16
      && do_backscrolling)
    scrollchange = LEFT;
  // scroll right when in right 1/3rd of screen
  else if(player->get_bbox().get_middle().x > translation.x + screen->w/3*2+16)
    scrollchange = RIGHT;

  // calculate our scroll target depending on scroll mode
  float target_x;
  if(scrollchange == LEFT)
    target_x = player->get_bbox().get_middle().x - screen->w/3*2;
  else if(scrollchange == RIGHT)
    target_x = player->get_bbox().get_middle().x - screen->w/3;
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

  keep_in_bounds();
}

void
Camera::scroll_autoscroll(float elapsed_time)
{
  Player* player = sector->player;
  
  if(player->is_dying())
    return;

  if(auto_t - elapsed_time >= 0) {
    translation += current_dir * elapsed_time;
    auto_t -= elapsed_time;
  } else {
    // do the rest of the old movement
    translation += current_dir * auto_t;
    elapsed_time -= auto_t;
    auto_t = 0;

    // construct path for next point
    if(auto_idx+1 >= scrollpoints.size()) {
      keep_in_bounds();
      return;
    }
    Vector distance = scrollpoints[auto_idx+1].position 
                      - scrollpoints[auto_idx].position;
    current_dir = distance.unit() * scrollpoints[auto_idx].speed;
    auto_t = distance.norm() / scrollpoints[auto_idx].speed;

    // do movement for the remaining time
    translation += current_dir * elapsed_time;
    auto_t -= elapsed_time;
    auto_idx++;
  }

  keep_in_bounds();
}

