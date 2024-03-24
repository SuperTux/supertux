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
#include "squirrel/exposed_object.hpp"
#include "supertux/direction.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/object_remove_listener.hpp"
#include "supertux/physic.hpp"
#include "supertux/player_status.hpp"
#include "supertux/sequence.hpp"
#include "supertux/timer.hpp"
#include "video/layer.hpp"
#include "video/surface_ptr.hpp"

class BadGuy;
class Climbable;
class Controller;
class CodeController;
class Key;
class Portable;

extern const float TUX_INVINCIBLE_TIME_WARNING;

class Player final : public MovingObject,
                     public ExposedObject<Player, scripting::Player>
{
public:
  enum FallMode { ON_GROUND, JUMPING, TRAMPOLINE_JUMP, FALLING };

private:
  class GrabListener final : public ObjectRemoveListener
  {
  public:
    GrabListener(Player& player) : m_player(player)
    {}

    virtual void object_removed(GameObject* object) override {
      m_player.ungrab_object(object);
    }

  private:
    Player& m_player;

  private:
    GrabListener(const GrabListener&) = delete;
    GrabListener& operator=(const GrabListener&) = delete;
  };

public:
  static Color get_player_color(int id);

public:
  Player(PlayerStatus& player_status, const std::string& name, int player_id);
  ~Player() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void collision_tile(uint32_t tile_attributes) override;
  virtual void on_flip(float height) override;
  virtual bool is_saveable() const override { return false; }
  virtual bool is_singleton() const override { return false; }
  virtual bool has_object_manager_priority() const override { return true; }
  virtual void remove_me() override;

  int get_id() const { return m_id; }
  void set_id(int id);

  virtual int get_layer() const override { return LAYER_OBJECTS + 1; }

  void set_controller(const Controller* controller);
  /** Level solved. Don't kill Tux any more. */
  void set_winning();
  bool is_winning() const { return m_winning; }

  // Tux can only go this fast. If set to 0 no special limit is used, only the default limits.
  void set_speedlimit(float newlimit);
  float get_speedlimit() const;

  const Controller& get_controller() const { return *m_controller; }

  void use_scripting_controller(bool use_or_release);
  void do_scripting_controller(const std::string& control, bool pressed);

  /** Move the player to a different sector, including any objects that it points to, or references. */
  void move_to_sector(Sector& other);

  void make_invincible();
  void make_temporarily_safe();

  bool is_invincible() const { return m_invincible_timer.started(); }
  bool is_dying() const { return m_dying; }

  Direction peeking_direction_x() const { return m_peekingX; }
  Direction peeking_direction_y() const { return m_peekingY; }

  void kill(bool completely);
  void move(const Vector& vector);

  bool add_bonus(const std::string& bonus);
  bool set_bonus(const std::string& bonus);
  void add_coins(int count);
  int get_coins() const;

  /** picks up a bonus, taking care not to pick up lesser bonus items than we already have

      @returns true if the bonus has been set (or was already good enough)
               false if the bonus could not be set (for example no space for big tux) */
  bool add_bonus(BonusType type, bool animate = false);

  /** like add_bonus, but can also downgrade the bonus items carried */
  bool set_bonus(BonusType type, bool animate = false, bool increment_powerup_counter = true);
  BonusType get_bonus() const;

  std::string bonus_to_string() const;

  PlayerStatus& get_status() const { return m_player_status; }

  /** set kick animation */
  void kick();

  /** gets the players action */
  std::string get_action() const;

  /** play cheer animation.
      This might need some space and behave in an unpredictable way.
      Best to use this at level end. */
  void do_cheer();

  /** duck down if possible.
      this won't last long as long as input is enabled. */
  void do_duck();

  /** stand back up if possible. */
  void do_standup(bool force_standup);

  /** do a backflip if possible. */
  void do_backflip();

  /** jump in the air if possible
      sensible values for yspeed are negative - unless we want to jump
      into the ground of course */
  void do_jump(float yspeed);

  /** Adds velocity to the player (be careful when using this) */
  void add_velocity(const Vector& velocity);

  /** Adds velocity to the player until given end speed is reached */
  void add_velocity(const Vector& velocity, const Vector& end_speed);

  /** Returns the current velocity of the player */
  Vector get_velocity() const;

  void bounce(BadGuy& badguy);
  void override_velocity() { m_velocity_override = true; }

  bool is_dead() const { return m_dead; }
  bool is_big() const;
  bool is_stone() const { return m_stone; }
  bool is_sliding() const { return m_sliding; }
  bool is_swimming() const { return m_swimming; }
  bool is_swimboosting() const { return m_swimboosting; }
  bool is_water_jumping() const { return m_water_jump; }
  bool is_skidding() const { return m_skidding_timer.started(); }
  float get_swimming_angle() const { return m_swimming_angle; }

  void set_visible(bool visible);
  bool get_visible() const;

  bool on_ground() const;
  void set_on_ground(bool flag);

  Portable* get_grabbed_object() const { return m_grabbed_object; }
  void stop_grabbing() { ungrab_object(); }

  /** Checks whether the player has grabbed a certain object
      @param name Name of the object to check */
  bool has_grabbed(const std::string& object_name) const;

  /** Switches ghost mode on/off.
      Lets Tux float around and through solid objects. */
  void set_ghost_mode(bool enable);

  /** Returns whether ghost mode is currently enabled */
  bool get_ghost_mode() const { return m_ghost_mode; }

  /** Changes height of bounding box.
      Returns true if successful, false otherwise */
  bool adjust_height(float new_height, float bottom_offset = 0);

  /** Orders the current GameSession to start a sequence
      @param sequence_name Name of the sequence to start
      @param data Custom additional sequence data */
  void trigger_sequence(const std::string& sequence_name, const SequenceData* data = nullptr);

  /** Orders the current GameSession to start a sequence
      @param sequence Sequence to start
      @param data Custom additional sequence data */
  void trigger_sequence(Sequence seq, const SequenceData* data = nullptr);

  /** Requests that the player start climbing the given Climbable */
  void start_climbing(Climbable& climbable);

  /** Requests that the player stop climbing the given Climbable */
  void stop_climbing(Climbable& climbable);

  Physic& get_physic() { return m_physic; }

  void activate();
  void deactivate();

  void walk(float speed);
  void set_dir(bool right);
  void stop_backflipping();

  void position_grabbed_object(bool teleport = false);
  bool try_grab();

  /** Boosts Tux in a certain direction, sideways. Useful for bumpers/walljumping. */
  void sideways_push(float delta);

  void multiplayer_prepare_spawn();

  void set_ending_direction(int direction) { m_ending_direction = direction; }
  int get_ending_direction() const { return m_ending_direction; }

  const std::vector<Key*>& get_collected_keys() const { return m_collected_keys; }
  void add_collected_key(Key* key);
  void remove_collected_key(Key* key);

  bool track_state() const override { return false; }

private:
  void handle_input();
  void handle_input_ghost(); /**< input handling while in ghost mode */
  void handle_input_climbing(); /**< input handling while climbing */
  void handle_input_rolling();

  void handle_input_swimming();

  void handle_horizontal_input();
  void handle_vertical_input();

  void do_jump_apex();
  void early_jump_apex();

  void slide();
  void swim(float pointx, float pointy, bool boost);

  BonusType string_to_bonus(const std::string& bonus) const;

  /** slows Tux down a little, based on where he's standing */
  void apply_friction();

  void check_bounds();

  /**
   * Ungrabs the currently grabbed object, if any. Only call with its argument
   * from an ObjectRemoveListener.
   */
  void ungrab_object(GameObject* gameobject = nullptr);

  void next_target();
  void prev_target();

  void multiplayer_respawn();

  void stop_rolling(bool violent = true);

private:
  int m_id;
  std::unique_ptr<UID> m_target; /**< (Multiplayer) If not null, then the player does not exist in game and is offering the player to spawn at that player's position */
  bool m_deactivated;

  const Controller* m_controller;
  std::unique_ptr<CodeController> m_scripting_controller; /**< This controller is used when the Player is controlled via scripting */
  PlayerStatus& m_player_status;
  bool m_duck;
  bool m_crawl;
  bool m_dead;
  bool m_dying;
  bool m_winning;
  bool m_backflipping;
  int  m_backflip_direction;
  Direction m_peekingX;
  Direction m_peekingY;
  bool m_stone;
  bool m_sliding;
  bool m_slidejumping;
  bool m_swimming;
  bool m_swimboosting;
  bool m_no_water;
  bool m_on_left_wall;
  bool m_on_right_wall;
  bool m_in_walljump_tile;
  bool m_can_walljump;
  float m_boost;
  float m_speedlimit;
  bool m_velocity_override;
  const Controller* m_scripting_controller_old; /**< Saves the old controller while the scripting_controller is used */
  bool m_jump_early_apex;
  bool m_on_ice;
  bool m_ice_this_frame;
  //SpritePtr m_santahatsprite;
  SpritePtr m_multiplayer_arrow;

  // Multiplayer tag stuff (number displayed over the players)
  Timer m_tag_timer;
  std::unique_ptr<FadeHelper> m_tag_fade;
  float m_tag_alpha;
  bool m_has_moved; // If the player sent input to move the player

public:
  Direction m_dir;

private:
  Direction m_old_dir;

public:
  float m_last_ground_y;
  FallMode m_fall_mode;

private:
  bool m_on_ground_flag;
  bool m_jumping;
  bool m_can_jump;
  Timer m_jump_button_timer; /**< started when player presses the jump button; runs until Tux jumps or JUMP_GRACE_TIME runs out */
  Timer m_coyote_timer; /**< started when Tux falls off a ledge; runs until Tux jumps or COYOTE_TIME runs out */
  bool m_wants_buttjump;
  bool m_buttjump_stomp;

public:
  bool m_does_buttjump;
  Timer m_invincible_timer;

private:
  Timer m_skidding_timer;
  Timer m_safe_timer;
  bool m_safe_due_to_hurt;
  Timer m_kick_timer;
  Timer m_buttjump_timer;

public:
  Timer m_dying_timer;

private:
  Timer m_second_growup_sound_timer;
  bool m_growing;
  Timer m_backflip_timer;

  Physic m_physic;

  bool m_visible;

  Portable* m_grabbed_object;
  std::unique_ptr<ObjectRemoveListener> m_grabbed_object_remove_listener;
  bool m_released_object;

  SpritePtr m_sprite; /**< The main sprite representing Tux */

  float m_swimming_angle;
  float m_swimming_accel_modifier;
  bool m_water_jump;

  SurfacePtr m_airarrow; /**< arrow indicating Tux' position when he's above the camera */

  Vector m_floor_normal;

  bool m_ghost_mode; /**< indicates if Tux should float around and through solid objects */

  Timer m_unduck_hurt_timer; /**< if Tux wants to stand up again after ducking and cannot, this timer is started */

  Timer m_idle_timer;
  unsigned int m_idle_stage;

  Climbable* m_climbing; /**< Climbable object we are currently climbing, null if none */

  int m_ending_direction;
  std::vector<Key*> m_collected_keys;

  float m_last_sliding_angle;
  float m_current_sliding_angle;
  float m_target_sliding_angle;
  Timer m_sliding_rotation_timer;
  bool m_is_slidejump_falling;
  bool m_was_crawling_before_slide;

private:
  Player(const Player&) = delete;
  Player& operator=(const Player&) = delete;
};

#endif

/* EOF */
