//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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
#include "SDL.h"

#include "timer.h"
#include "direction.h"
#include "video/surface.h"
#include "moving_object.h"
#include "sprite/sprite.h"
#include "math/physic.h"
#include "control/controller.h"
#include "player_status.h"

using namespace SuperTux;

class BadGuy;
class Portable;

/* Times: */
static const float TUX_SAFE_TIME = 1.250;
static const float TUX_INVINCIBLE_TIME = 10.0;
static const float TUX_INVINCIBLE_TIME_WARNING = 2.0;
static const float TUX_FLAPPING_TIME = 1; /* How long Tux can flap his wings to gain additional jump height */
static const float TIME_WARNING = 20;     /* When to alert player they're low on time! */
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
  void draw(DrawingContext& context, const Vector& pos, int layer,
                Uint32 drawing_effect = NONE_EFFECT);

  Sprite* head;
  Sprite* body;
  Sprite* arms;
  Sprite* feet;
};

extern TuxBodyParts* small_tux;
extern TuxBodyParts* big_tux;
extern TuxBodyParts* fire_tux;
extern TuxBodyParts* ice_tux;

class Player : public MovingObject
{
public:
  enum HurtMode { KILL, SHRINK };
  enum FallMode { ON_GROUND, JUMPING, TRAMPOLINE_JUMP, FALLING };

  Controller* controller;
  PlayerStatus* player_status;
  bool duck;
  bool dead;

private:
  bool dying;
public:

  Direction dir;
  Direction old_dir;

  float last_ground_y;
  FallMode fall_mode;

  bool on_ground_flag;
  bool jumping;
  bool flapping;
  bool can_jump;
  bool can_flap;
  bool falling_from_flap;
  bool enable_hover;
  bool butt_jump;
  
  float flapping_velocity;

  // Ricardo's flapping
  int flaps_nb;

  // temporary to help player's choosing a flapping
  enum { MAREK_FLAP, RICARDO_FLAP, RYAN_FLAP, NO_FLAP };
  int flapping_mode;

  Timer2 invincible_timer;
  Timer2 skidding_timer;
  Timer2 safe_timer;
  Timer2 kick_timer;
  Timer2 shooting_timer;   // used to show the arm when Tux is shooting
  Timer2 dying_timer;
  Timer2 growing_timer;
  Timer2 idle_timer;
  Timer2 flapping_timer;
  Physic physic;
  
public:
  Player(PlayerStatus* player_status);
  virtual ~Player();

  void set_controller(Controller* controller);  

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

  void make_invincible();
  bool is_invincible() const
  {
    return invincible_timer.started();
  }
  bool is_dying() const
  {
    return dying;
  }
  
  void kill(HurtMode mode);
  void check_bounds(Camera* camera);
  void move(const Vector& vector);
  void set_bonus(BonusType type, bool animate = false);
  PlayerStatus* get_status()
  {
    return player_status;
  }

  void bounce(BadGuy& badguy);

  bool is_dead() const
  { return dead; }
  bool is_big();
  
private:
  void handle_input();
  bool on_ground();
  
  void init();
  
  void handle_horizontal_input();
  void handle_vertical_input();

  Portable* grabbed_object;

  Sprite* smalltux_gameover;
  Sprite* smalltux_star;
  Sprite* bigtux_star;
};

#endif /*SUPERTUX_PLAYER_H*/
