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
#include "camera.h"

#include <math.h>
#include "lispwriter.h"
#include "player.h"
#include "level.h"
#include "globals.h"

Camera::Camera(Player* newplayer, Level* newlevel)
  : player(newplayer), level(newlevel), scrollchange(NONE)
{
  if(!player || !level)
    mode = MANUAL;
  else
    mode = NORMAL;
}

Camera::~Camera()
{
}

void
Camera::set_translation(const Vector& newtranslation)
{
  translation = newtranslation;
}

void
Camera::write(LispWriter& writer)
{
  writer.start_list("camera");
  
  if(mode == NORMAL) {
    writer.write_string("mode", "normal");
  } else if(mode == AUTOSCROLL) {
    writer.write_string("mode", "autoscroll");
  } else if(mode == MANUAL) {
    writer.write_string("mode", "manual");
  }
                     
  writer.end_list("camera");
}

static const float EPSILON = .00001;
static const float max_speed_y = 1.4;

void
Camera::action(float elapsed_time)
{
  if(mode == NORMAL)
    scroll_normal(elapsed_time);
  else if(mode == AUTOSCROLL)
    scroll_autoscroll(elapsed_time);
}

void
Camera::scroll_normal(float elapsed_time)
{
  assert(level != 0 && player != 0);
  
  // check that we don't have division by zero later
  if(elapsed_time < EPSILON)
    return;

  /****** Vertical Scrolling part ******/
  bool do_y_scrolling = true;

  if(player->dying || level->height == 19)
    do_y_scrolling = false;

  if(do_y_scrolling) {
    // target_y is the high we target our scrolling at. This is not always the
    // high of the player, but if he is jumping upwards we should use the
    // position where he last touched the ground.
    float target_y; 
    if(player->fall_mode == Player::JUMPING)
      target_y = player->last_ground_y + player->base.height;
    else
      target_y = player->base.y + player->base.height;

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

    // don't scroll before the start or after the level's end
    if(translation.y > level->height * 32 - screen->h)
      translation.y = level->height * 32 - screen->h;
    if(translation.y < 0)
      translation.y = 0; 
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
  if(player->base.x < translation.x + screen->w/3 && level->back_scrolling)
    scrollchange = LEFT;
  // scroll right when in right 1/3rd of screen
  else if(player->base.x > translation.x + screen->w/3*2)
    scrollchange = RIGHT;

  // calculate our scroll target depending on scroll mode
  float target_x;
  if(scrollchange == LEFT)
    target_x = player->base.x - screen->w/3*2;
  else if(scrollchange == RIGHT)
    target_x = player->base.x - screen->w/3;
  else
    target_x = translation.x;

  // that's the distance we would have to travel to reach target_x
  float delta_x = translation.x - target_x;
  // the speed we'd need to travel to reach target_x in this frame
  float speed_x = delta_x / elapsed_time;

  // limit our speed
  float maxv = 1 + fabsf(player->physic.get_velocity_x() * 1.3);
  if(speed_x > maxv)
    speed_x = maxv;
  else if(speed_x < -maxv)
    speed_x = -maxv;
 
  // apply scrolling
  translation.x -= speed_x * elapsed_time;

  // don't scroll before the start or after the level's end
  if(translation.x > level->width * 32 - screen->w)
    translation.x = level->width * 32 - screen->w;
  if(translation.x < 0)
    translation.x = 0;   
}

void
Camera::scroll_autoscroll(float elapsed_time)
{
  // TODO
}
