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

#ifndef HEADER_SUPERTUX_OBJECT_PLAYER_HPP
#define HEADER_SUPERTUX_OBJECT_PLAYER_HPP

#include "scripting/player.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/direction.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/physic.hpp"
#include "supertux/player_status.hpp"
#include "supertux/script_interface.hpp"
#include "supertux/timer.hpp"

class BadGuy;
class Portable;
class Climbable;
class Controller;
class CodeController;
class Surface;
class Timer;

/* Times: */
static const float TUX_SAFE_TIME = 1.8f;
static const float TUX_INVINCIBLE_TIME = 14.0f;
static const float TUX_INVINCIBLE_TIME_WARNING = 2.0f;
static const float GROWING_TIME = 0.35f;
static const int GROWING_FRAMES = 7;

class Camera;
class PlayerStatus;

class Player : public MovingObject, 
               public scripting::Player, 
               public ScriptInterface
{
public:
  enum FallMode { ON_GROUND, JUMPING, TRAMPOLINE_JUMP, FALLING };
  //Tux can only go this fast. If set to 0 no special limit is used, only the default limits.
  void set_speedlimit(float newlimit);
  float get_speedlimit();

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
  Direction peeking_direction_x() const
  {
    return peekingX;
  }

  Direction peeking_direction_y() const
  {
    return peekingY;
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
   * Adds velocity to the player (be careful when using this)
   */
  void add_velocity(const Vector& velocity);

  /**
   * Adds velocity to the player until given end speed is reached
   */
  void add_velocity(const Vector& velocity, const Vector& end_speed);
  
  /**
   * Returns the current velocity of the player
   */
  Vector get_velocity();

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
  void stop_grabbing()
  {
    grabbed_object = NULL;
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

  Physic& get_physic() { return physic; }

private:
  void handle_input();
  void handle_input_ghost(); /**< input handling while in ghost mode */
  void handle_input_climbing(); /**< input handling while climbing */

  void init();

  void handle_horizontal_input();
  void handle_vertical_input();

  void activate();
  void deactivate();
  void walk(float speed);

  void do_jump_apex();
  void early_jump_apex();

  /**
   * slows Tux down a little, based on where he's standing
   */
  void apply_friction();

private:
  bool deactivated;

  Controller* controller;
  std::auto_ptr<CodeController> scripting_controller; /**< This controller is used when the Player is controlled via scripting */
  PlayerStatus* player_status;
  bool duck;
  bool dead;

private:
  bool dying;
  bool backflipping;
  int  backflip_direction;
  Direction peekingX;
  Direction peekingY;
  bool swimming;
  float speedlimit;
  Controller* scripting_controller_old; /**< Saves the old controller while the scripting_controller is used */
  bool jump_early_apex;
  bool on_ice;
  bool ice_this_frame;

public:
  Direction dir;
  Direction old_dir;

  float last_ground_y;
  FallMode fall_mode;

  bool on_ground_flag;
  bool jumping;
  bool can_jump;
  Timer jump_button_timer; /**< started when player presses the jump button; runs until Tux jumps or JUMP_GRACE_TIME runs out */
  bool wants_buttjump;
  bool does_buttjump;

  Timer invincible_timer;
  Timer skidding_timer;
  Timer safe_timer;
  Timer kick_timer;
  Timer shooting_timer;   // used to show the arm when Tux is shooting
  Timer dying_timer;
  bool growing;
  Timer backflip_timer;

  Physic physic;

  bool visible;

  Portable* grabbed_object;

  SpritePtr sprite; /**< The main sprite representing Tux */

  SurfacePtr airarrow; /**< arrow indicating Tux' position when he's above the camera */

  Vector floor_normal;
  void position_grabbed_object();
  void try_grab();

  bool ghost_mode; /**< indicates if Tux should float around and through solid objects */
  bool edit_mode; /**< indicates if Tux should switch to ghost mode rather than dying */

  Timer unduck_hurt_timer; /**< if Tux wants to stand up again after ducking and cannot, this timer is started */

  Timer idle_timer;
  unsigned int idle_stage;

  Climbable* climbing; /**< Climbable object we are currently climbing, null if none */

private:
  Player(const Player&);
  Player& operator=(const Player&);
};

#endif /*SUPERTUX_PLAYER_H*/

/* EOF */
