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
#include "control/codecontroller.hpp"
#include "scripting/player.hpp"
#include "player_status.hpp"
#include "display_effect.hpp"
#include "script_interface.hpp"
#include "console.hpp"
#include "coin.hpp"

class BadGuy;
class Portable;
class Climbable;

/* Times: */
static const float TUX_SAFE_TIME = 1.8f;
static const float TUX_INVINCIBLE_TIME = 10.0f;
static const float TUX_INVINCIBLE_TIME_WARNING = 2.0f;
static const float GROWING_TIME = 0.35f;
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
  void draw(DrawingContext& context, const Vector& pos, int layer, Portable* grabbed_object);

  Sprite* head;
  Sprite* body;
  Sprite* arms;
  Sprite* feet;
};

extern TuxBodyParts* small_tux;
extern TuxBodyParts* big_tux;
extern TuxBodyParts* fire_tux;
extern TuxBodyParts* ice_tux;

class Player : public MovingObject, public UsesPhysic, public Scripting::Player, public ScriptInterface
{
public:
  enum FallMode { ON_GROUND, JUMPING, TRAMPOLINE_JUMP, FALLING };

  Controller* controller;
  CodeController* scripting_controller; /**< This controller is used when the Player is controlled via scripting */
  PlayerStatus* player_status;
  bool duck;
  bool dead;
  //Tux can only go this fast. If set to 0 no special limit is used, only the default limits.
  void set_speedlimit(float newlimit);
  float get_speedlimit();

private:
  bool dying;
  bool backflipping;
  int  backflip_direction;
  Direction peeking;
  bool swimming;
  float speedlimit;
  Controller* scripting_controller_old; /**< Saves the old controller while the scripting_controller is used */

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

public:
  Player(PlayerStatus* player_status, const std::string& name);
  virtual ~Player();

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  void set_controller(Controller* controller);
  Controller* get_controller()
  {
    return controller;
  }

  void use_scripting_controller(bool use_or_release);
  void do_scripting_controller(std::string control, bool pressed);

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual void collision_solid(const CollisionHit& hit);
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
  Direction peeking_direction() const
  {
    return peeking;
  }

  void kill(bool completely);
  void check_bounds(Camera* camera);
  void move(const Vector& vector);

  virtual bool add_bonus(const std::string& bonus);
  virtual void add_coins(int count);
  virtual int get_coins();

  /**
   * picks up a bonus, taking care not to pick up lesser bonus items than we already have
   *
   * @returns true if the bonus has been set (or was already good enough)
   *          false if the bonus could not be set (for example no space for big tux)
   */
  bool add_bonus(BonusType type, bool animate = false);
  /**
   * like add_bonus, but can also downgrade the bonus items carried
   */
  bool set_bonus(BonusType type, bool animate = false);

  PlayerStatus* get_status()
  {
    return player_status;
  }
  // set kick animation
  void kick();

  /**
   * play cheer animation.
   * This might need some space and behave in an unpredictable way. Best to use this at level end.
   */
  void do_cheer();

  /**
   * duck down if possible.
   * this won't last long as long as input is enabled.
   */
  void do_duck();

  /**
   * stand back up if possible.
   */
  void do_standup();

  /**
   * do a backflip if possible.
   */
  void do_backflip();

  /**
   * jump in the air if possible
   * sensible values for yspeed are negative - unless we want to jump into the ground of course
   */
  void do_jump(float yspeed);

  /**
   * Adds velocity to the player (be carefull when using this)
   */
  void add_velocity(const Vector& velocity);

  /**
   * Adds velocity to the player until given end speed is reached
   */
  void add_velocity(const Vector& velocity, const Vector& end_speed);

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

  /**
   * Switches ghost mode on/off.
   * Lets Tux float around and through solid objects.
   */
  void set_ghost_mode(bool enable);

  /**
   * Switches edit mode on/off.
   * In edit mode, Tux will enter ghost_mode instead of dying.
   */
  void set_edit_mode(bool enable);

  /**
   * Returns whether ghost mode is currently enabled
   */
  bool get_ghost_mode() { return ghost_mode; }

  /**
   * Changes height of bounding box.
   * Returns true if successful, false otherwise
   */
  bool adjust_height(float new_height);

  /**
   * Orders the current GameSession to start a sequence
   */
  void trigger_sequence(std::string sequence_name);
  
  /**
   * Requests that the player start climbing the given Climbable
   */
  void start_climbing(Climbable& climbable);

  /**
   * Requests that the player stop climbing the given Climbable
   */
  void stop_climbing(Climbable& climbable);

private:
  void handle_input();
  void handle_input_ghost(); /**< input handling while in ghost mode */
  void handle_input_climbing(); /**< input handling while climbing */
  bool deactivated;

  void init();

  void handle_horizontal_input();
  void handle_vertical_input();

  void activate();
  void deactivate();
  void walk(float speed);

  /**
   * slows Tux down a little, based on where he's standing
   */
  void apply_friction();

  bool visible;

  Portable* grabbed_object;

  Sprite* smalltux_gameover;
  Sprite* smalltux_star;
  Sprite* bigtux_star;

  std::auto_ptr<Surface> airarrow; /**< arrow indicating Tux' position when he's above the camera */

  Vector floor_normal;
  void try_grab();

  bool ghost_mode; /**< indicates if Tux should float around and through solid objects */
  bool edit_mode; /**< indicates if Tux should switch to ghost mode rather than dying */

  Timer unduck_hurt_timer; /**< if Tux wants to stand up again after ducking and cannot, this timer is started */

  Climbable* climbing; /**< Climbable object we are currently climbing, null if none */
};

#endif /*SUPERTUX_PLAYER_H*/
