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

#include "scripting/exposed_object.hpp"
#include "scripting/player.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/direction.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/physic.hpp"
#include "supertux/player_status.hpp"
#include "supertux/sequence.hpp"
#include "supertux/timer.hpp"

class BadGuy;
class Portable;
class Climbable;
class Controller;
class CodeController;

/* Times: */
static const float TUX_SAFE_TIME = 1.8f;
static const float TUX_INVINCIBLE_TIME = 14.0f;
static const float TUX_INVINCIBLE_TIME_WARNING = 2.0f;
static const float GROWING_TIME = 0.35f;
static const int GROWING_FRAMES = 7;
static const float TUX_BACKFLIP_TIME = 2.1f; // minimum air time that backflip results in a loss of control

class Player : public MovingObject,
               public ExposedObject<Player, scripting::Player>
{
public:
  enum FallMode { ON_GROUND, JUMPING, TRAMPOLINE_JUMP, FALLING };
  //Tux can only go this fast. If set to 0 no special limit is used, only the default limits.
  void set_speedlimit(float newlimit);
  float get_speedlimit() const;
  virtual bool is_saveable() const {
    return false;
  }

public:
  Player(PlayerStatus& player_status, const std::string& name);
  virtual ~Player();

  void set_controller(Controller* controller);
  /*
   * Level solved. Don't kill Tux any more.
   */
  void set_winning();
  bool is_winning() const
  {
    return m_winning;
  }

  Controller* get_controller() const
  {
    return m_controller;
  }

  void use_scripting_controller(bool use_or_release);
  void do_scripting_controller(const std::string& control, bool pressed);

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual void collision_solid(const CollisionHit& hit);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void collision_tile(uint32_t tile_attributes);

  void make_invincible();
  bool is_invincible() const
  {
    return m_invincible_timer.started();
  }
  bool is_dying() const
  {
    return m_dying;
  }
  Direction peeking_direction_x() const
  {
    return m_peekingX;
  }

  Direction peeking_direction_y() const
  {
    return m_peekingY;
  }

  void kill(bool completely);
  void check_bounds();
  void move(const Vector& vector);

  virtual bool add_bonus(const std::string& bonus);
  virtual bool set_bonus(const std::string& bonus);
  virtual void add_coins(int count);
  virtual int get_coins() const;

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

  PlayerStatus& get_status() const
  {
    return m_player_status;
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
  Vector get_velocity() const;

  void bounce(BadGuy& badguy);

  bool is_dead() const
  { return m_dead; }
  bool is_big() const;
  bool is_stone() const
  { return m_stone; }

  void set_visible(bool visible);
  bool get_visible() const;

  bool on_ground() const;

  Portable* get_grabbed_object() const
  {
    return m_grabbed_object;
  }
  void stop_grabbing()
  {
    m_grabbed_object = NULL;
  }
  /**
   * Checks whether the player has grabbed a certain object
   * @param name Name of the object to check
   */
  bool has_grabbed(const std::string& object_name) const;

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
  bool get_ghost_mode() const { return m_ghost_mode; }

  /**
   * Changes height of bounding box.
   * Returns true if successful, false otherwise
   */
  bool adjust_height(float new_height);

  /**
   * Orders the current GameSession to start a sequence
   * @param sequence_name Name of the sequence to start
   * @param data Custom additional sequence data
   */
  void trigger_sequence(const std::string& sequence_name, const SequenceData* data = NULL);

  /**
   * Orders the current GameSession to start a sequence
   * @param sequence Sequence to start
   * @param data Custom additional sequence data
   */
  void trigger_sequence(Sequence seq, const SequenceData* data = NULL);

  /**
   * Requests that the player start climbing the given Climbable
   */
  void start_climbing(Climbable& climbable);

  /**
   * Requests that the player stop climbing the given Climbable
   */
  void stop_climbing(Climbable& climbable);

  Physic& get_physic() { return m_physic; }

  void activate();
  void deactivate();

  void walk(float speed);
  void set_dir(bool right);
  void stop_backflipping();

private:
  void handle_input();
  void handle_input_ghost(); /**< input handling while in ghost mode */
  void handle_input_climbing(); /**< input handling while climbing */

  void handle_horizontal_input();
  void handle_vertical_input();

  void do_jump_apex();
  void early_jump_apex();

  bool slightly_above_ground() const;

  BonusType string_to_bonus(const std::string& bonus) const;

  /**
   * slows Tux down a little, based on where he's standing
   */
  void apply_friction();

private:
  bool m_deactivated;

  Controller* m_controller;
  std::unique_ptr<CodeController> m_scripting_controller; /**< This controller is used when the Player is controlled via scripting */
  PlayerStatus& m_player_status;
  bool m_duck;
  bool m_dead;

private:
  bool m_dying;
  bool m_winning;
  bool m_backflipping;
  int  m_backflip_direction;
  Direction m_peekingX;
  Direction m_peekingY;
  float m_ability_time;
  bool m_stone;
  bool m_swimming;
  float m_speedlimit;
  Controller* m_scripting_controller_old; /**< Saves the old controller while the scripting_controller is used */
  bool m_jump_early_apex;
  bool m_on_ice;
  bool m_ice_this_frame;
  SpritePtr m_lightsprite;
  SpritePtr m_powersprite;

public:
  Direction m_dir;
  Direction m_old_dir;

  float m_last_ground_y;
  FallMode m_fall_mode;

  bool m_on_ground_flag;
  bool m_jumping;
  bool m_can_jump;
  Timer m_jump_button_timer; /**< started when player presses the jump button; runs until Tux jumps or JUMP_GRACE_TIME runs out */
  bool m_wants_buttjump;
  bool m_does_buttjump;

  Timer m_invincible_timer;
  Timer m_skidding_timer;
  Timer m_safe_timer;
  Timer m_kick_timer;
  Timer m_shooting_timer;   // used to show the arm when Tux is shooting
  Timer m_ability_timer;  // maximum lengh of time that special abilities can last
  Timer m_cooldown_timer; // minimum time period between successive uses of a special ability
  Timer m_dying_timer;
  Timer m_second_growup_sound_timer;
  bool m_growing;
  Timer m_backflip_timer;

  Physic m_physic;

  bool m_visible;

  Portable* m_grabbed_object;

  SpritePtr m_sprite; /**< The main sprite representing Tux */

  SurfacePtr m_airarrow; /**< arrow indicating Tux' position when he's above the camera */

  Vector m_floor_normal;
  void position_grabbed_object();
  void try_grab();

  bool m_ghost_mode; /**< indicates if Tux should float around and through solid objects */
  bool m_edit_mode; /**< indicates if Tux should switch to ghost mode rather than dying */

  Timer m_unduck_hurt_timer; /**< if Tux wants to stand up again after ducking and cannot, this timer is started */

  Timer m_idle_timer;
  unsigned int m_idle_stage;

  Climbable* m_climbing; /**< Climbable object we are currently climbing, null if none */

private:
  Player(const Player&);
  Player& operator=(const Player&);
};

#endif /*SUPERTUX_PLAYER_H*/

/* EOF */
