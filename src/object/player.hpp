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

#ifndef SUPERTUX_PLAYER_H
#define SUPERTUX_PLAYER_H

#include <vector>
#include <SDL.h>

#include "timer.hpp"
#include "direction.hpp"
#include "video/surface.hpp"
#include "moving_object.hpp"
#include "sprite/sprite.hpp"
#include "physic.hpp"
#include "control/controller.hpp"
#include "scripting/player.hpp"
#include "player_status.hpp"
#include "display_effect.hpp"
#include "script_interface.hpp"
#include "console.hpp"
#include "coin.hpp"

class BadGuy;
class Portable;

/* Times: */
static const float TUX_SAFE_TIME = 1.8;
static const float TUX_INVINCIBLE_TIME = 10.0;
static const float TUX_INVINCIBLE_TIME_WARNING = 2.0;
static const float GROWING_TIME = 1.0;
static const int GROWING_FRAMES = 7;

class Camera;
class PlayerStatus;

extern Surface* growingtux_left[GROWING_FRAMES];
extern Surface* growingtux_right[GROWING_FRAMES];

class TuxBodyParts
{
public:
  TuxBodyParts()
    : head(0), body(0), arms(0), feet(0)
  { }
  ~TuxBodyParts() {
    delete head;
    delete body;
    delete arms;
    delete feet;
  }

  void set_action(std::string action, int loops = -1);
  void one_time_animation();
  void draw(DrawingContext& context, const Vector& pos, int layer);

  Sprite* head;
  Sprite* body;
  Sprite* arms;
  Sprite* feet;
};

extern TuxBodyParts* small_tux;
extern TuxBodyParts* big_tux;
extern TuxBodyParts* fire_tux;
extern TuxBodyParts* ice_tux;

class Player : public MovingObject, public Scripting::Player, public ScriptInterface
{
public:
  enum FallMode { ON_GROUND, JUMPING, TRAMPOLINE_JUMP, FALLING };

  Controller* controller;
  PlayerStatus* player_status;
  bool duck;
  bool dead;

private:
  bool dying;
  bool backflipping;
  int  backflip_direction;
  
public:
  Direction dir;
  Direction old_dir;

  float last_ground_y;
  FallMode fall_mode;

  bool on_ground_flag;
  bool jumping;
  bool can_jump;
  bool butt_jump;

  Timer invincible_timer;
  Timer skidding_timer;
  Timer safe_timer;
  Timer kick_timer;
  Timer shooting_timer;   // used to show the arm when Tux is shooting
  Timer dying_timer;
  Timer growing_timer;
  Timer idle_timer;
  Timer backflip_timer;
  Physic physic;
  
public:
  Player(PlayerStatus* player_status);
  virtual ~Player();

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  void set_controller(Controller* controller);
  Controller* get_controller()
  {
    return controller;
  }

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void collision_tile(uint32_t tile_attributes);

  void make_invincible();
  bool is_invincible() const
  {
    return invincible_timer.started();
  }
  bool is_dying() const
  {
    return dying;
  }
  
  void kill(bool completely);
  void check_bounds(Camera* camera);
  void move(const Vector& vector);

  virtual void add_bonus(const std::string& bonus);
  virtual void add_coins(int count);
  void add_bonus(BonusType type, bool animate = false); /**< picks up a bonus, taking care not to pick up lesser bonus items than we already have */
  void set_bonus(BonusType type, bool animate = false); /**< like add_bonus, but can also downgrade the bonus items carried */
  PlayerStatus* get_status()
  {
    return player_status;
  }
  // set kick animation
  void kick();

  /**
   * Adds velocity to the player (be carefull when using this)
   */
  void add_velocity(const Vector& velocity);

  void bounce(BadGuy& badguy);

  bool is_dead() const
  { return dead; }
  bool is_big();

  void set_visible(bool visible);
  bool get_visible();

  bool on_ground();

  Portable* get_grabbed_object() const
  {
      return grabbed_object;
  }

private:
  void handle_input();
  bool deactivated;
  
  void init();
  
  void handle_horizontal_input();
  void handle_vertical_input();

  void activate();
  void deactivate();
  void walk(float speed);

  bool visible;

  float adjust_height;

  Portable* grabbed_object;

  Sprite* smalltux_gameover;
  Sprite* smalltux_star;
  Sprite* bigtux_star;
  Vector floor_normal;
};

#endif /*SUPERTUX_PLAYER_H*/
