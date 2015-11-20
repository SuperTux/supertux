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

#include "object/camera.hpp"

#include <math.h>
#include <physfs.h>

#include "util/reader.hpp"
#include "util/writer.hpp"
#include "lisp/parser.hpp"
#include "object/path_walker.hpp"
#include "object/player.hpp"
#include "scripting/camera.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"

/* this is the fractional distance toward the peek
   position to move each frame; lower is slower,
   0 is never get there, 1 is instant */
static const float PEEK_ARRIVE_RATIO = 0.1;

class CameraConfig
{
public:
  // 0 = No, 1 = Fix, 2 = Mario/Yoshi, 3 = Kirby, 4 = Super Metroid-like
  int xmode;
  // as above
  int ymode;
  float kirby_rectsize_x;
  float kirby_rectsize_y;
  // where to fix the player (used for Yoshi and Fix camera)
  float target_x;
  float target_y;
  // maximum scrolling speed in Y direction
  float max_speed_x;
  float max_speed_y;
  // factor to dynamically increase max_speed_x based on player speed
  float dynamic_max_speed_x;

  // time the player has to face into the other direction before we assume a
  // changed direction
  float dirchange_time;
  // edge_x
  float edge_x;
  // when too change from noscroll mode back to lookahead left/right mode
  // set to <= 0 to disable noscroll mode
  float sensitive_x;

  float clamp_x;
  float clamp_y;

  float dynamic_speed_sm;

  CameraConfig() :
    xmode(4),
    ymode(3),
    kirby_rectsize_x(0.2f),
    kirby_rectsize_y(0.34f),
    target_x(.5f),
    target_y(.5f),
    max_speed_x(100),
    max_speed_y(100),
    dynamic_max_speed_x(1.0),
    dirchange_time(0.2f),
    edge_x(0.4f),
    sensitive_x(-1),
    clamp_x(0.1666f),
    clamp_y(0.3f),
    dynamic_speed_sm(0.8f)
  {
  }

  void load(const std::string& filename)
  {
    auto doc = ReaderDocument::parse(filename);
    auto root = doc.get_root();
    if(root.get_name() == "camera-config")
    {
      throw std::runtime_error("file is not a camera config file.");
    }
    else
    {
      auto camconfig = root.get_mapping();

      camconfig.get("xmode", xmode);
      camconfig.get("ymode", ymode);
      camconfig.get("target-x", target_x);
      camconfig.get("target-y", target_y);
      camconfig.get("max-speed-x", max_speed_x);
      camconfig.get("max-speed-y", max_speed_y);
      camconfig.get("dynamic-max-speed-x", dynamic_max_speed_x);
      camconfig.get("dirchange-time", dirchange_time);
      camconfig.get("clamp-x", clamp_x);
      camconfig.get("clamp-y", clamp_y);
      camconfig.get("kirby-rectsize-x", kirby_rectsize_x);
      camconfig.get("kirby-rectsize-y", kirby_rectsize_y);
      camconfig.get("edge-x", edge_x);
      camconfig.get("sensitive-x", sensitive_x);
      camconfig.get("dynamic-speed-sm", dynamic_speed_sm);
    }
  }
};

Camera::Camera(Sector* newsector, std::string name_) :
  mode(NORMAL),
  translation(),
  sector(newsector),
  lookahead_mode(LOOKAHEAD_NONE),
  changetime(),
  lookahead_pos(),
  peek_pos(),
  cached_translation(),
  autoscroll_path(),
  autoscroll_walker(),
  shaketimer(),
  shakespeed(),
  shakedepth_x(),
  shakedepth_y(),
  scroll_from(),
  scroll_goal(),
  scroll_to_pos(),
  scrollspeed(),
  config()
{
  this->name = name_;
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
  scripting::Camera* _this = new scripting::Camera(this);
  expose_object(vm, table_idx, _this, name, true);
}

void
Camera::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if(name.empty()) return;
  scripting::unexpose_object(vm, table_idx, name);
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
Camera::parse(const ReaderMapping& reader)
{
  std::string modename;

  reader.get("mode", modename);
  if(modename == "normal") {
    mode = NORMAL;
  } else if(modename == "autoscroll") {
    mode = AUTOSCROLL;

    ReaderMapping path_mapping;
    if(!reader.get("path", path_mapping)) {
      log_warning << "No path specified in autoscroll camera." << std::endl;
      mode = NORMAL;
    } else {
      autoscroll_path.reset(new Path());
      autoscroll_path->read(path_mapping);
      autoscroll_walker.reset(new PathWalker(autoscroll_path.get()));
    }
  } else if(modename == "manual") {
    mode = MANUAL;
  } else {
    mode = NORMAL;
    log_warning << "invalid camera mode '" << modename << "'found in worldfile." << std::endl;
  }
}

void
Camera::reset(const Vector& tuxpos)
{
  translation.x = tuxpos.x - SCREEN_WIDTH/2;
  translation.y = tuxpos.y - SCREEN_HEIGHT/2;

  shakespeed = 0;
  shaketimer.stop();
  keep_in_bounds(translation);

  cached_translation = translation;
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

static const float CAMERA_EPSILON = .00001f;

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
  shake();
}

void
Camera::reload_config()
{
  if(PHYSFS_exists("camera.cfg")) {
    try {
      config->load("camera.cfg");
      log_info << "Loaded camera.cfg." << std::endl;
    } catch(std::exception &e) {
      log_debug << "Couldn't load camera.cfg, using defaults ("
                << e.what() << ")" << std::endl;
    }
  }
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
Camera::keep_in_bounds(Vector& translation_)
{
  float width = sector->get_width();
  float height = sector->get_height();

  // don't scroll before the start or after the level's end
  translation_.x = clamp(translation_.x, 0, width - SCREEN_WIDTH);
  translation_.y = clamp(translation_.y, 0, height - SCREEN_HEIGHT);

  if (height < SCREEN_HEIGHT)
    translation_.y = height/2.0 - SCREEN_HEIGHT/2.0;
  if (width < SCREEN_WIDTH)
    translation_.x = width/2.0 - SCREEN_WIDTH/2.0;
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
  const CameraConfig& config_ = *(this->config);
  Player* player = sector->player;
  // TODO: co-op mode needs a good camera
  Vector player_pos(player->get_bbox().get_middle().x,
                                    player->get_bbox().get_bottom());
  static Vector last_player_pos = player_pos;
  Vector player_delta = player_pos - last_player_pos;
  last_player_pos = player_pos;

  // check that we don't have division by zero later
  if(elapsed_time < CAMERA_EPSILON)
    return;

  /****** Vertical Scrolling part ******/
  int ymode = config_.ymode;

  if(player->is_dying() || sector->get_height() == 19*32) {
    ymode = 0;
  }
  if(ymode == 1) {
    cached_translation.y = player_pos.y - SCREEN_HEIGHT * config_.target_y;
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
    target_y -= SCREEN_HEIGHT * config_.target_y;

    // delta_y is the distance we'd have to travel to directly reach target_y
    float delta_y = cached_translation.y - target_y;
    // speed is the speed the camera would need to reach target_y in this frame
    float speed_y = delta_y / elapsed_time;

    // limit the camera speed when jumping upwards
    if(player->fall_mode != Player::FALLING
       && player->fall_mode != Player::TRAMPOLINE_JUMP) {
      speed_y = clamp(speed_y, -config_.max_speed_y, config_.max_speed_y);
    }

    // scroll with calculated speed
    cached_translation.y -= speed_y * elapsed_time;
  }
  if(ymode == 3) {
    float halfsize = config_.kirby_rectsize_y * 0.5f;
    cached_translation.y = clamp(cached_translation.y,
                                 player_pos.y - SCREEN_HEIGHT * (0.5f + halfsize),
                                 player_pos.y - SCREEN_HEIGHT * (0.5f - halfsize));
  }
  if(ymode == 4) {
    float upperend = SCREEN_HEIGHT * config_.edge_x;
    float lowerend = SCREEN_HEIGHT * (1 - config_.edge_x);

    if (player_delta.y < -CAMERA_EPSILON) {
      // walking left
      lookahead_pos.y -= player_delta.y * config_.dynamic_speed_sm;

      if(lookahead_pos.y > lowerend) {
        lookahead_pos.y = lowerend;
      }
    } else if (player_delta.y > CAMERA_EPSILON) {
      // walking right
      lookahead_pos.y -= player_delta.y * config_.dynamic_speed_sm;
      if(lookahead_pos.y < upperend) {
        lookahead_pos.y = upperend;
      }
    }

    // adjust for level ends
    if (player_pos.y < upperend) {
      lookahead_pos.y = upperend;
    }
    if (player_pos.y > sector->get_width() - upperend) {
      lookahead_pos.y = lowerend;
    }

    cached_translation.y = player_pos.y - lookahead_pos.y;
  }

  translation.y = cached_translation.y;

  if(ymode != 0) {
    float top_edge, bottom_edge;
    if(config_.clamp_y <= 0) {
      top_edge = 0;
      bottom_edge = SCREEN_HEIGHT;
    } else {
      top_edge = SCREEN_HEIGHT*config_.clamp_y;
      bottom_edge = SCREEN_HEIGHT*(1-config_.clamp_y);
    }

    float peek_to = 0;
    float translation_compensation = player_pos.y - translation.y;

    if(player->peeking_direction_y() == ::UP) {
      peek_to = bottom_edge - translation_compensation;
    } else if(player->peeking_direction_y() == ::DOWN) {
      peek_to = top_edge - translation_compensation;
    }

    float peek_move = (peek_to - peek_pos.y) * PEEK_ARRIVE_RATIO;
    if(fabs(peek_move) < 1.0) {
      peek_move = 0.0;
    }

    peek_pos.y += peek_move;

    translation.y -= peek_pos.y;

    if(config_.clamp_y > 0) {
      translation.y = clamp(translation.y,
                            player_pos.y - SCREEN_HEIGHT * (1-config_.clamp_y),
                            player_pos.y - SCREEN_HEIGHT * config_.clamp_y);
      cached_translation.y = clamp(cached_translation.y,
                                   player_pos.y - SCREEN_HEIGHT * (1-config_.clamp_y),
                                   player_pos.y - SCREEN_HEIGHT * config_.clamp_y);
    }
  }

  /****** Horizontal scrolling part *******/
  int xmode = config_.xmode;

  if(player->is_dying())
    xmode = 0;

  if(xmode == 1) {
    cached_translation.x = player_pos.x - SCREEN_WIDTH * config_.target_x;
  }
  if(xmode == 2) {
    // our camera is either in leftscrolling, rightscrolling or
    // nonscrollingmode.
    //
    // when suddenly changing directions while scrolling into the other
    // direction abort scrolling, since tux might be going left/right at a
    // relatively small part of the map (like when jumping upwards)

    // Find out direction in which the player moves
    LookaheadMode walkDirection;
    if (player_delta.x < -CAMERA_EPSILON) walkDirection = LOOKAHEAD_LEFT;
    else if (player_delta.x > CAMERA_EPSILON) walkDirection = LOOKAHEAD_RIGHT;
    else if (player->dir == ::LEFT) walkDirection = LOOKAHEAD_LEFT;
    else walkDirection = LOOKAHEAD_RIGHT;

    float LEFTEND, RIGHTEND;
    if(config_.sensitive_x > 0) {
      LEFTEND = SCREEN_WIDTH * config_.sensitive_x;
      RIGHTEND = SCREEN_WIDTH * (1-config_.sensitive_x);
    } else {
      LEFTEND = SCREEN_WIDTH;
      RIGHTEND = 0;
    }

    if(lookahead_mode == LOOKAHEAD_NONE) {
      /* if we're undecided then look if we crossed the left or right
       * "sensitive" area */
      if(player_pos.x < cached_translation.x + LEFTEND) {
        lookahead_mode = LOOKAHEAD_LEFT;
      } else if(player_pos.x > cached_translation.x + RIGHTEND) {
        lookahead_mode = LOOKAHEAD_RIGHT;
      }
      /* at the ends of a level it's obvious which way we will go */
      if(player_pos.x < SCREEN_WIDTH*0.5) {
        lookahead_mode = LOOKAHEAD_RIGHT;
      } else if(player_pos.x >= sector->get_width() - SCREEN_WIDTH*0.5) {
        lookahead_mode = LOOKAHEAD_LEFT;
      }

      changetime = -1;
    } else if(lookahead_mode != walkDirection) {
      /* player changed direction while camera was scrolling...
       * he has to do this for a certain time to add robustness against
       * sudden changes */
      if(changetime < 0) {
        changetime = game_time;
      } else if(game_time - changetime > config_.dirchange_time) {
        if(lookahead_mode == LOOKAHEAD_LEFT &&
           player_pos.x > cached_translation.x + RIGHTEND) {
          lookahead_mode = LOOKAHEAD_RIGHT;
        } else if(lookahead_mode == LOOKAHEAD_RIGHT &&
                  player_pos.x < cached_translation.x + LEFTEND) {
          lookahead_mode = LOOKAHEAD_LEFT;
        } else {
          lookahead_mode = LOOKAHEAD_NONE;
        }
      }
    } else {
      changetime = -1;
    }

    LEFTEND = SCREEN_WIDTH * config_.edge_x;
    RIGHTEND = SCREEN_WIDTH * (1-config_.edge_x);

    // calculate our scroll target depending on scroll mode
    float target_x;
    if(lookahead_mode == LOOKAHEAD_LEFT)
      target_x = player_pos.x - RIGHTEND;
    else if(lookahead_mode == LOOKAHEAD_RIGHT)
      target_x = player_pos.x - LEFTEND;
    else
      target_x = cached_translation.x;

    // that's the distance we would have to travel to reach target_x
    float delta_x = cached_translation.x - target_x;
    // the speed we'd need to travel to reach target_x in this frame
    float speed_x = delta_x / elapsed_time;

    // limit our speed
    float player_speed_x = player_delta.x / elapsed_time;
    float maxv = config_.max_speed_x + (fabsf(player_speed_x * config_.dynamic_max_speed_x));
    speed_x = clamp(speed_x, -maxv, maxv);

    // apply scrolling
    cached_translation.x -= speed_x * elapsed_time;
  }
  if(xmode == 3) {
    float halfsize = config_.kirby_rectsize_x * 0.5f;
    cached_translation.x = clamp(cached_translation.x,
                                 player_pos.x - SCREEN_WIDTH * (0.5f + halfsize),
                                 player_pos.x - SCREEN_WIDTH * (0.5f - halfsize));
  }
  if(xmode == 4) {
    float LEFTEND = SCREEN_WIDTH * config_.edge_x;
    float RIGHTEND = SCREEN_WIDTH * (1 - config_.edge_x);

    if (player_delta.x < -CAMERA_EPSILON) {
      // walking left
      lookahead_pos.x -= player_delta.x * config_.dynamic_speed_sm;
      if(lookahead_pos.x > RIGHTEND) {
        lookahead_pos.x = RIGHTEND;
      }

    } else if (player_delta.x > CAMERA_EPSILON) {
      // walking right
      lookahead_pos.x -= player_delta.x * config_.dynamic_speed_sm;
      if(lookahead_pos.x < LEFTEND) {
        lookahead_pos.x = LEFTEND;
      }
    }

    // adjust for level ends
    if (player_pos.x < LEFTEND) {
      lookahead_pos.x = LEFTEND;
    }
    if (player_pos.x > sector->get_width() - LEFTEND) {
      lookahead_pos.x = RIGHTEND;
    }

    cached_translation.x = player_pos.x - lookahead_pos.x;
  }

  translation.x = cached_translation.x;

  if(xmode != 0) {
    float left_edge, right_edge;
    if(config_.clamp_x <= 0) {
      left_edge = 0;
      right_edge = SCREEN_WIDTH;
    } else {
      left_edge = SCREEN_WIDTH*config_.clamp_x;
      right_edge = SCREEN_WIDTH*(1-config_.clamp_x);
    }

    float peek_to = 0;
    float translation_compensation = player_pos.x - translation.x;

    if(player->peeking_direction_x() == ::LEFT) {
      peek_to = right_edge - translation_compensation;
    } else if(player->peeking_direction_x() == ::RIGHT) {
      peek_to = left_edge - translation_compensation;
    }

    float peek_move = (peek_to - peek_pos.x) * PEEK_ARRIVE_RATIO;
    if(fabs(peek_move) < 1.0) {
      peek_move = 0.0;
    }

    peek_pos.x += peek_move;

    translation.x -= peek_pos.x;

    if(config_.clamp_x > 0) {
      translation.x = clamp(translation.x,
                            player_pos.x - SCREEN_WIDTH * (1-config_.clamp_x),
                            player_pos.x - SCREEN_WIDTH * config_.clamp_x);

      cached_translation.x = clamp(cached_translation.x,
                                   player_pos.x - SCREEN_WIDTH * (1-config_.clamp_x),
                                   player_pos.x - SCREEN_WIDTH * config_.clamp_x);
    }
  }

  keep_in_bounds(translation);
  keep_in_bounds(cached_translation);
}

void
Camera::update_scroll_autoscroll(float elapsed_time)
{
  Player* player = sector->player;
  if(player->is_dying())
    return;

  translation = autoscroll_walker->advance(elapsed_time);

  keep_in_bounds(translation);
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

Vector
Camera::get_center() const {
  return translation + Vector(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

/* EOF */
