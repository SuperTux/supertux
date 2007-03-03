//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#include "lisp/parser.hpp"
#include "scripting/camera.hpp"
#include "scripting/squirrel_util.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "tilemap.hpp"
#include "game_session.hpp"
#include "sector.hpp"
#include "main.hpp"
#include "object_factory.hpp"
#include "log.hpp"
#include "path.hpp"
#include "path_walker.hpp"

namespace {
  enum CameraStyle { CameraStyleYI, CameraStyleKD, CameraStyleEXP };
  const CameraStyle cameraStyle = CameraStyleEXP;
}

class CameraConfig
{
public:
  // 0 = No, 1 = Fix, 2 = Mario/Yoshi, 3 = Kirby, 4 = inverse rubber
  int ymode;
  int xmode;
  float kirby_rectsize_x;
  float kirby_rectsize_y;
  // where to fix the player (used for Yoshi and Fix camera)
  float target_y;
  float target_x;
  // maximum scrolling speed in Y direction
  float max_speed_y;
  float max_speed_x;
  // factor to dynamically increase max_speed_x based on player speed
  float dynamic_max_speed_x;
  // edge_x
  float edge_x;
  float sensitive_x;

  float clamp_y;
  float clamp_x;

  CameraConfig() {
    xmode = 1;
    ymode = 1;
    target_x = .5f;
    target_y = 2.f/3.f;
    max_speed_y = 140;
    max_speed_x = 130;
    clamp_x = 1.f/6.f;
    clamp_y = 1.f/6.f;
    kirby_rectsize_x = 0.2f;
    kirby_rectsize_y = 0.34f;
    edge_x = 1.f/3.f;
    sensitive_x = 1.f/4.f;
	dynamic_max_speed_x = 1.0;
  }

  void load(const std::string& filename)
  {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(filename);
    const lisp::Lisp* camconfig = root->get_lisp("camera-config");
    if(camconfig == NULL)
      throw std::runtime_error("file is not a camera config file.");

    camconfig->get("xmode", xmode);
    camconfig->get("ymode", ymode);
    camconfig->get("target-x", target_x);
    camconfig->get("target-y", target_y);
    camconfig->get("max-speed-x", max_speed_x);
    camconfig->get("max-speed-y", max_speed_y);
	camconfig->get("dynamic-max-speed-x", dynamic_max_speed_x);
    camconfig->get("clamp-x", clamp_x);
    camconfig->get("clamp-y", clamp_y);
    camconfig->get("kirby-rectsize-x", kirby_rectsize_x);
    camconfig->get("kirby-rectsize-y", kirby_rectsize_y);
    camconfig->get("edge-x", edge_x);
    camconfig->get("sensitive-x", sensitive_x);
  }
};

Camera::Camera(Sector* newsector, std::string name)
  : mode(NORMAL), sector(newsector), scrollchange(NONE)
{
  this->name = name;
  config = new CameraConfig();
  reload_config();
}

Camera::~Camera()
{
  delete config;
}

void
Camera::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if(name.empty()) return;
  Scripting::Camera* interface = new Scripting::Camera(this);
  expose_object(vm, table_idx, interface, name, true);
}

void
Camera::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if(name.empty()) return;
  Scripting::unexpose_object(vm, table_idx, name);
}

void
Camera::draw(DrawingContext& )
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
  } else if(mode == AUTOSCROLL) {
    writer.write_string("mode", "autoscroll");
    autoscroll_path->write(writer);
  } else if(mode == MANUAL) {
    writer.write_string("mode", "manual");
  }

  writer.end_list("camera");
}

void
Camera::reset_kd(const Vector& tuxpos)
{
  translation.x = tuxpos.x - (SCREEN_WIDTH * 0.5);
  translation.y = tuxpos.y - (SCREEN_HEIGHT * 0.5);

  shakespeed = 0;
  shaketimer.stop();
  keep_in_bounds(translation);
}


void
Camera::reset(const Vector& tuxpos)
{
  if (cameraStyle == CameraStyleKD) {
    reset_kd(tuxpos);
    return;
  }

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

static const float EPSILON = .00001f;
static const float MAX_SPEED_Y = 140;

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
Camera::reload_config()
{
  config->load("camera.cfg");
}

float clamp(float val, float min, float max)
{
  if(val < min)
    return min;
  if(val > max)
    return max;

  return val;
}

void
Camera::keep_in_bounds(Vector& translation)
{
  float width = sector->get_width();
  float height = sector->get_height();

  // don't scroll before the start or after the level's end
  translation.x = clamp(translation.x, 0, width - SCREEN_WIDTH);
  translation.y = clamp(translation.y, 0, height - SCREEN_HEIGHT);

  if (height < SCREEN_HEIGHT)
    translation.y = height/2.0 - SCREEN_HEIGHT/2.0;
  if (width < SCREEN_WIDTH)
    translation.x = width/2.0 - SCREEN_WIDTH/2.0;
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
Camera::update_scroll_normal_kd(float elapsed_time)
{
  // make sure some time has actually passed
  if(elapsed_time < EPSILON)
    return;

  // make sure we have an active player
  assert(sector != NULL);
  Player* player = sector->player;
  Vector playerCenter = player->get_bbox().get_middle();

  // If player is peeking, scroll in that direction
  if (player->peeking_direction() == ::LEFT) {
    translation.x -= elapsed_time * 128.0f;
  } else if (player->peeking_direction() == ::RIGHT) {
    translation.x += elapsed_time * 128.0f;
  }

  // keep player within a small box, centered on the screen (vertical part)
  bool do_y_scrolling = true;
  if (player->is_dying() || sector->get_height() == 19*32)
    do_y_scrolling = false;

  if (do_y_scrolling) {
    translation.y = clamp(translation.y,
        player->get_bbox().p1.y - SCREEN_HEIGHT * (0.5f - 0.17f),
        player->get_bbox().p2.y - SCREEN_HEIGHT * (0.5f + 0.17f));
  }

  // keep player within a small box, centered on the screen (horizontal part)
  translation.x = clamp(translation.x,
      player->get_bbox().p1.x - SCREEN_WIDTH * (0.5f - 0.1f),
      player->get_bbox().p2.x - SCREEN_WIDTH * (0.5f + 0.1f));

  // make sure camera doesn't point outside level borders
  keep_in_bounds(translation);

  // handle shaking of camera (if applicable)
  shake();
}

void
Camera::update_scroll_normal_exp(float elapsed_time)
{
  const CameraConfig& config = *(this->config);
  Player* player = sector->player;
  const Vector& player_pos = player->get_bbox().get_middle();
  static Vector last_player_pos = player_pos;
  Vector player_delta = player_pos - last_player_pos;

  // check that we don't have division by zero later
  if(elapsed_time < EPSILON)
    return;

  /****** Vertical Scrolling part ******/
  int ymode = config.ymode;

  if(player->is_dying() || sector->get_height() == 19*32) {
    ymode = 0;
  }

  if(ymode == 1) {
    translation.y = player_pos.y - SCREEN_HEIGHT * config.target_y;
  }
  if(ymode == 2) {
    // target_y is the high we target our scrolling at. This is not always the
    // high of the player, but if he is jumping upwards we should use the
    // position where he last touched the ground. (this probably needs
    // exceptions for trampolines and similar things in the future)
    float target_y;
    if(player->fall_mode == Player::JUMPING)
      target_y = player->last_ground_y + player->get_bbox().get_height();
    else
      target_y = player->get_bbox().p2.y;
    target_y -= SCREEN_HEIGHT * config.target_y;

    // delta_y is the distance we'd have to travel to directly reach target_y
    float delta_y = translation.y - target_y;
    // speed is the speed the camera would need to reach target_y in this frame
    float speed_y = delta_y / elapsed_time;

    // limit the camera speed when jumping upwards
    if(player->fall_mode != Player::FALLING
        && player->fall_mode != Player::TRAMPOLINE_JUMP) {
      speed_y = clamp(speed_y, -config.max_speed_y, config.max_speed_y);
    }

    // scroll with calculated speed
    translation.y -= speed_y * elapsed_time;
  }
  if(ymode == 3) {
    float halfsize = config.kirby_rectsize_y * 0.5f;
    translation.y = clamp(translation.y,
        player_pos.y - SCREEN_HEIGHT * (0.5f - halfsize),
        player_pos.y - SCREEN_HEIGHT * (0.5f + halfsize));
  }
  if(ymode == 4) {
    // TODO...
  }

  if(ymode != 0 && config.clamp_y > 0) {
    translation.y = clamp(translation.y,
        player_pos.y - SCREEN_HEIGHT * config.clamp_y,
        player_pos.y - SCREEN_HEIGHT * (1-config.clamp_y));
  }

  /****** Horizontal scrolling part *******/

  if(config.xmode == 1) {
    translation.x = player_pos.x - SCREEN_WIDTH * config.target_x;
  }
  if(config.xmode == 2) {
    // our camera is either in leftscrolling, rightscrolling or
    // nonscrollingmode.
    //
    // when suddenly changing directions while scrolling into the other
    // direction abort scrolling, since tux might be going left/right at a
    // relatively small part of the map (like when jumping upwards)

    // Find out direction in which the player walks
    LeftRightScrollChange walkDirection;
    if (player->physic.get_velocity_x() < -EPSILON) walkDirection = LEFT;
    else if (player->physic.get_velocity_x() > EPSILON) walkDirection = RIGHT;
    else if (player->dir == ::LEFT) walkDirection = LEFT;
    else walkDirection = RIGHT;

    float LEFTEND = SCREEN_WIDTH * config.sensitive_x;
    float RIGHTEND = SCREEN_WIDTH * (1-config.sensitive_x);

    if((walkDirection == LEFT && scrollchange == RIGHT)
        || (walkDirection == RIGHT && scrollchange == LEFT))
      scrollchange = NONE;
    // when in left 1/3rd of screen scroll left
    if(player_pos.x < translation.x + LEFTEND)
      scrollchange = LEFT;
    // scroll right when in right 1/3rd of screen
    else if(player_pos.x > translation.x + RIGHTEND)
      scrollchange = RIGHT;

    LEFTEND = SCREEN_WIDTH * config.edge_x;
    RIGHTEND = SCREEN_HEIGHT * (1- config.edge_x);

    // calculate our scroll target depending on scroll mode
    float target_x;
    if(scrollchange == LEFT)
      target_x = player->get_bbox().get_middle().x - RIGHTEND;
    else if(scrollchange == RIGHT)
      target_x = player->get_bbox().get_middle().x - LEFTEND;
    else
      target_x = translation.x;

    // that's the distance we would have to travel to reach target_x
    float delta_x = translation.x - target_x;
    // the speed we'd need to travel to reach target_x in this frame
    float speed_x = delta_x / elapsed_time;

    // limit our speed
    float maxv = config.max_speed_x + (fabsf(player->physic.get_velocity_x() * config.dynamic_max_speed_x));
    speed_x = clamp(speed_x, -maxv, maxv);

    // If player is peeking scroll in that direction. Fast.
    if(player->peeking_direction() == ::LEFT) {
      speed_x = config.max_speed_x;
    }
    if(player->peeking_direction() == ::RIGHT) {
      speed_x = -config.max_speed_x;
    }

    // apply scrolling
    translation.x -= speed_x * elapsed_time;
  }
  if(config.xmode == 3) {
    float halfsize = config.kirby_rectsize_x * 0.5f;
    translation.x = clamp(translation.x,
        player_pos.x - SCREEN_WIDTH * (0.5f - halfsize),
        player_pos.x - SCREEN_WIDTH * (0.5f + halfsize));
  }
  if(config.xmode == 4) {
    // TODO...
  }

  if(config.xmode != 0 && config.clamp_x > 0) {
    translation.x = clamp(translation.x,
        player_pos.x - SCREEN_WIDTH * config.clamp_x,
        player_pos.x - SCREEN_WIDTH * (1-config.clamp_x));
  }

  keep_in_bounds(translation);
  shake();

#if 0
  static const Vector camera_speed = Vector(300, 100);

  Player* player = sector->player;
  const Vector& player_pos = player->get_bbox().get_middle();
  static Vector last_player_pos = player_pos;
  static Vector camera_delta = Vector(0, 0);

  (void) elapsed_time;

  Vector player_delta_x = player_pos - last_player_pos;
  last_player_pos = player_pos;

  Vector camera_delta_antic = Vector(0, 0) + player_delta_x * 25;
  Vector myspeed = (camera_delta_antic - camera_delta) / elapsed_time;
  myspeed.x = clamp(-camera_speed.x, camera_speed.x, myspeed.x);
  myspeed.y = clamp(-camera_speed.y, camera_speed.y, myspeed.y);

  camera_delta += myspeed * elapsed_time;

  translation.x = camera_delta.x + player_pos.x - 0.5f * SCREEN_WIDTH;
  translation.y = camera_delta.y + player_pos.y - 0.5f * SCREEN_HEIGHT;

  keep_in_bounds(translation);
  shake();
#endif
}

void
Camera::update_scroll_normal(float elapsed_time)
{
  if (cameraStyle == CameraStyleEXP) {
    update_scroll_normal_exp(elapsed_time);
    return;
  }
  if (cameraStyle == CameraStyleKD) {
    update_scroll_normal_kd(elapsed_time);
    return;
  }

  assert(sector != 0);
  Player* player = sector->player;

  // check that we don't have division by zero later
  if(elapsed_time < EPSILON)
    return;

  /****** Vertical Scrolling part ******/
  bool do_y_scrolling = true;

  if(player->is_dying() || sector->get_height() == 19*32)
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
      speed_y = clamp(speed_y, -MAX_SPEED_Y, MAX_SPEED_Y);
    }

    // finally scroll with calculated speed
    translation.y -= speed_y * elapsed_time;

    // make sure to always keep the player inside the middle 1/6 of the screen
    translation.y = clamp(translation.y,
        player->get_bbox().p1.y - SCREEN_HEIGHT*1/6,
        player->get_bbox().p2.y - SCREEN_HEIGHT*5/6);
  }

  /****** Horizontal scrolling part *******/

  // our camera is either in leftscrolling, rightscrolling or nonscrollingmode.

  // when suddenly changing directions while scrolling into the other direction.
  // abort scrolling, since tux might be going left/right at a relatively small
  // part of the map (like when jumping upwards)


  // Find out direction in which the player walks: We want to try and show a
  // bit more of what's in front of the player and less of what's behind
  LeftRightScrollChange walkDirection;
  if (player->physic.get_velocity_x() < -EPSILON) walkDirection = LEFT;
  else if (player->physic.get_velocity_x() > EPSILON) walkDirection = RIGHT;
  else if (player->dir == ::LEFT) walkDirection = LEFT;
  else walkDirection = RIGHT;

  static const float LEFTEND = SCREEN_WIDTH*2/5;
  static const float RIGHTEND = SCREEN_WIDTH*4/5;

  if((walkDirection == LEFT && scrollchange == RIGHT)
      || (walkDirection == RIGHT && scrollchange == LEFT))
    scrollchange = NONE;
  // when in left 1/3rd of screen scroll left
  if(player->get_bbox().get_middle().x < translation.x + LEFTEND - 16
      && do_backscrolling)
    scrollchange = LEFT;
  // scroll right when in right 1/3rd of screen
  else if(player->get_bbox().get_middle().x > translation.x + RIGHTEND + 16)
    scrollchange = RIGHT;

  // calculate our scroll target depending on scroll mode
  float target_x;
  if(scrollchange == LEFT)
    target_x = player->get_bbox().get_middle().x - RIGHTEND;
  else if(scrollchange == RIGHT)
    target_x = player->get_bbox().get_middle().x - LEFTEND;
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

  // If player is peeking scroll in that direction. Fast.
  if( player->peeking_direction() == ::LEFT ){
        speed_x = maxv;
  }
  if( player->peeking_direction() == ::RIGHT ){
        speed_x = -maxv;
  }

  // apply scrolling
  translation.x -= speed_x * elapsed_time;

  // make sure to always keep the player inside the middle 4/6 of the screen
  translation.x = std::min(player->get_bbox().p1.x - SCREEN_WIDTH*1/6, translation.x);
  translation.x = std::max(player->get_bbox().p2.x - SCREEN_WIDTH*5/6, translation.x);

  keep_in_bounds(translation);
  shake();
}

void
Camera::update_scroll_autoscroll(float elapsed_time)
{
  Player* player = sector->player;
  if(player->is_dying())
    return;

  translation = autoscroll_walker->advance(elapsed_time);

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
