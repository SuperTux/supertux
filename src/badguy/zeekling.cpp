//  Zeekling - flyer that swoops down when she spots the player
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/zeekling.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include <algorithm>

Zeekling::Zeekling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"),
  speed(gameRandom.rand(130, 171)),
  diveRecoverTimer(),
  state(FLYING),
  last_player(0),
  last_player_pos(),
  last_self_pos(),
  pre_dive_pos(),
  pre_dive_player_pos(),
  estimated_player_pos(),
  m_solution(3)
{
  physic.enable_gravity(false);
}

Zeekling::Zeekling(const Vector& pos, Direction d) :
  BadGuy(pos, d, "images/creatures/zeekling/zeekling.sprite"),
  speed(gameRandom.rand(130, 171)),
  diveRecoverTimer(),
  state(FLYING),
  last_player(0),
  last_player_pos(),
  last_self_pos(),
  pre_dive_pos(),
  pre_dive_player_pos(),
  estimated_player_pos(),
  m_solution(3)
{
  physic.enable_gravity(false);
}

void
Zeekling::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -speed : speed);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
Zeekling::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

void
Zeekling::onBumpHorizontal() {
  if (frozen)
  {
    physic.set_velocity_x(0);
    return;
  }
  if (state == FLYING) {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(dir == LEFT ? -speed : speed);
  } else
    if (state == DIVING) {
      dir = (dir == LEFT ? RIGHT : LEFT);
      state = FLYING;
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(dir == LEFT ? -speed : speed);
      physic.set_velocity_y(0);
    } else
      if (state == CLIMBING) {
        dir = (dir == LEFT ? RIGHT : LEFT);
        sprite->set_action(dir == LEFT ? "left" : "right");
        physic.set_velocity_x(dir == LEFT ? -speed : speed);
      } else {
        assert(false);
      }
}

void
Zeekling::onBumpVertical() {
  if (frozen || BadGuy::get_state() == STATE_BURNING)
  {
    physic.set_velocity_y(0);
    physic.set_velocity_x(0);
    return;
  }
  if (state == FLYING) {
    physic.set_velocity_y(0);
  } else
    if (state == DIVING) {
      state = CLIMBING;
      physic.set_velocity_y(-speed);
      sprite->set_action(dir == LEFT ? "left" : "right");
    } else
      if (state == CLIMBING) {
        state = FLYING;
        physic.set_velocity_y(0);
      }
}

void
Zeekling::collision_solid(const CollisionHit& hit)
{
  if(sprite->get_action() == "squished-left" ||
     sprite->get_action() == "squished-right")
  {
    return;
  }

  if(hit.top || hit.bottom) {
    onBumpVertical();
  }
  if(hit.left || hit.right) {
    onBumpHorizontal();
  }
}

/**
 * linear prediction of player and badguy positions to decide if we should enter the DIVING state
 */
bool
Zeekling::should_we_dive() {
  if (frozen)
    return false;

  const auto player = get_nearest_player();
  if (player && last_player && (player == last_player)) {

    // get positions, calculate movement
    const Vector& player_pos = player->get_pos();
    const Vector player_mov = (player_pos - last_player_pos);
    const Vector self_pos = bbox.p1;
    const Vector self_mov = (self_pos - last_self_pos);

    // new vertical speed to test with
    float vy = 2*fabsf(self_mov.x);

    // do not dive if we are not above the player
    float height = player_pos.y - self_pos.y;
    if (height <= 0) return false;

    // do not dive if we are too far above the player
    if (height > 512) return false;
    // do not dive if we would not descend faster than the player
    float relSpeed = vy - player_mov.y;
    if (relSpeed <= 0) return false;

    // guess number of frames to descend to same height as player
    float estFrames = height / relSpeed;
    // guess where the player would be at this time
    float estPx = (player_pos.x + (estFrames * player_mov.x));
    estimated_player_pos = Vector(estPx,player_pos.y+ (estFrames * player_mov.y));
      return (dir==LEFT) ? (get_pos().x >= pre_dive_player_pos.x ) : (get_pos().x <= pre_dive_player_pos.x);

  }

  // update last player tracked, as well as our positions
  last_player = player;
  if (player) {
    last_player_pos = player->get_pos();
    last_self_pos = bbox.p1;
  }

  return false;
}
/**
 *  Fills an equation vector given a Point (x|y)
 *  with the coefficients for f(x) = ax^2+bx+c
 */
void filleq(std::vector<double>& eq,double x,double y)
{
  eq[0] = pow(x,2);
  eq[1] = x;
  eq[2] = 1;
  eq[3] = y;
}
/**
 * Solves a system of linear equations using gaussian elimanation.
 * Uses O(n^3) Operations (because for zeeklings n = 3, this isn't as bad as it sounds)
 */
std::vector<double> gauss(std::vector< std::vector<double> > A) {
    int n = A.size();

    for (int i=0; i<n; i++) {
        // Search for maximum in this column
        double maxEl = abs(A[i][i]);
        int maxRow = i;
        for (int k=i+1; k<n; k++) {
            if (abs(A[k][i]) > maxEl) {
                maxEl = abs(A[k][i]);
                maxRow = k;
            }
        }
        for (int k=i; k<n+1;k++) {
            double tmp = A[maxRow][k];
            A[maxRow][k] = A[i][k];
            A[i][k] = tmp;
        }
        for (int k=i+1; k<n; k++) {
            double c = -A[k][i]/A[i][i];
            for (int j=i; j<n+1; j++) {
                if (i==j) {
                    A[k][j] = 0;
                } else {
                    A[k][j] += c * A[i][j];
                }
            }
        }
    }
    std::vector<double> x(n);
    for (int i=n-1; i>=0; i--) {
        x[i] = A[i][n]/A[i][i];
        for (int k=i-1;k>=0; k--) {
            A[k][n] -= A[k][i] * x[i];
        }
    }
    return x;
}
/**
 *  Evaluates f(x) = ax^2+bx+c , where the coefficients are given as an array
 *  at the point x.
 */
double feval(double x,std::vector<double> sol)
{
  return sol[0]*pow(x,2)+sol[1]*x+sol[2];
}

void
Zeekling::active_update(float elapsed_time) {
  if (state == FLYING) {
    if (should_we_dive()) {
      // Our position
      pre_dive_pos = get_pos();
      // The estimated position we should dive to
      pre_dive_player_pos = estimated_player_pos;
      sprite->set_action(dir == LEFT ? "diving-left" : "diving-right");
      // Calculate coefficients for f(x) = ax^2 + bx + c
      std::vector< std::vector<double> > lineq(3);
      for(int i = 0;i<3;i++)
        lineq[i].resize(4);
      // 1st condition: f(pre_dive_pos.x) = pre_dive_pos.y
      filleq(lineq[0], pre_dive_pos.x, pre_dive_pos.y);
      // 2nd condition: f(pre_dive_player_pos.x) =  pre_dive_player_pos.y
      filleq(lineq[1], pre_dive_player_pos.x, pre_dive_player_pos.y);
      // 3rd condition: f'(player_pos) = 0
      lineq[2][0] = 2*pre_dive_player_pos.x;
      lineq[2][1] = 1;
      lineq[2][2] = lineq[2][3] = 0.0;
      // Solve using gaussian elimanation
      std::vector<double> solution = gauss(lineq);
      // Save solution for later use
      m_solution = solution;
      // Only dive if average moving rate is within interval
      // (else the curves are too steep)
      if(abs( ( pre_dive_pos.y - pre_dive_player_pos.y ) / ( pre_dive_pos.x-pre_dive_player_pos.x ) ) < 0.5 || abs((pre_dive_pos.y-pre_dive_player_pos.y)/(pre_dive_pos.x-pre_dive_player_pos.x))  > 3)
      {
        state = FLYING;
        sprite->set_action(dir == LEFT ? "left" : "right");
      }else{
        // Move down
        log_debug << solution[0] << std::endl;

        state = DIVING;
        SoundManager::current()->play("sounds/deathd.wav");
        physic.set_velocity_y((-get_pos().y+feval(get_pos().x,m_solution))/elapsed_time);
      }
    }
    BadGuy::active_update(elapsed_time);
    return;
  } else if (state == DIVING) {
    // Are we still within the range of [estimatedplayer_x;zeeklingpos]
    if (get_pos().y <= pre_dive_player_pos.y && (dir==LEFT) ? (get_pos().x >= pre_dive_player_pos.x ) : (get_pos().x <= pre_dive_player_pos.x)) {
      // We're at or above the player level, so decrease y velocity
        physic.set_velocity_y((-get_pos().y+feval(get_pos().x,m_solution))/elapsed_time);
    } else {
      log_debug << "Updating" << std::endl;
      // We're already below the player, or already flying horizontally, so go ahead and climb back up
      onBumpVertical();
    }
    BadGuy::active_update(elapsed_time);
    return;
  } else if (state == CLIMBING) {
    // stop climbing when we're back at initial height
    if (get_pos().y <= start_position.y) {
      state = FLYING;
      physic.set_velocity_y(0);
    }
    BadGuy::active_update(elapsed_time);
    return;
  } else {
    assert(false);
  }
}

void
Zeekling::freeze()
{
  BadGuy::freeze();
  physic.enable_gravity(true);
}

void
Zeekling::unfreeze()
{
  BadGuy::unfreeze();
  physic.enable_gravity(false);
  state = FLYING;
  initialize();
}

bool
Zeekling::is_freezable() const
{
  return true;
}

/* EOF */
