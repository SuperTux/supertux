#include <config.h>

#include "badguy.h"
#include "object/camera.h"

static const float SQUISH_TIME = 2;
static const float X_OFFSCREEN_DISTANCE = 1600;
static const float Y_OFFSCREEN_DISTANCE = 1200;

BadGuy::BadGuy()
  : sprite(0), dir(LEFT), state(STATE_INIT)
{
}

BadGuy::~BadGuy()
{
  delete sprite;
}

void
BadGuy::draw(DrawingContext& context)
{
  if(!sprite)
    return;
  if(state == STATE_INIT || state == STATE_INACTIVE)
    return;
  if(state == STATE_FALLING) {
    sprite->draw(context, get_pos(), LAYER_OBJECTS, VERTICAL_FLIP);
  } else {
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
  }
}

void
BadGuy::action(float elapsed_time)
{
  if(!Sector::current()->inside(bbox)) {
    remove_me();
    return;
  }
  if(is_offscreen()) {
    set_state(STATE_INACTIVE);
  }
  
  switch(state) {
    case STATE_ACTIVE:
      active_action(elapsed_time);
      break;
    case STATE_INIT:
    case STATE_INACTIVE:
      inactive_action(elapsed_time);
      try_activate();
      break;
    case STATE_SQUISHED:
      if(state_timer.check()) {
        remove_me();
        break;
      }
      movement = physic.get_movement(elapsed_time);
      break;
    case STATE_FALLING:
      movement = physic.get_movement(elapsed_time);
      break;
  }
}

void
BadGuy::activate()
{
}

void
BadGuy::deactivate()
{
}

void
BadGuy::active_action(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

void
BadGuy::inactive_action(float )
{
}

HitResponse
BadGuy::collision(GameObject& other, const CollisionHit& hit)
{
  switch(state) {
    case STATE_INIT:
    case STATE_INACTIVE:
      return ABORT_MOVE;
    case STATE_ACTIVE: {
      if(other.get_flags() & FLAG_SOLID)
        return collision_solid(other, hit);

      BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
      if(badguy && badguy->state == STATE_ACTIVE)
        return collision_badguy(*badguy, hit);

      Player* player = dynamic_cast<Player*> (&other);
      if(player)
        return collision_player(*player, hit);

      return FORCE_MOVE;
    }
    case STATE_SQUISHED:
      if(other.get_flags() & FLAG_SOLID)
        return CONTINUE;
      return FORCE_MOVE;
    case STATE_FALLING:
      return FORCE_MOVE;
  }

  return ABORT_MOVE;
}

HitResponse
BadGuy::collision_solid(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

HitResponse
BadGuy::collision_player(Player& player, const CollisionHit& hit)
{
  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  }
  if(hit.normal.y > .9) {
    if(collision_squished(player))
      return ABORT_MOVE;
  }
  player.kill(Player::SHRINK);
  return FORCE_MOVE;
}

HitResponse
BadGuy::collision_badguy(BadGuy& , const CollisionHit& )
{
  return FORCE_MOVE;
}

bool
BadGuy::collision_squished(Player& )
{
  return false;
}

void
BadGuy::kill_squished(Player& player)
{
  SoundManager::get()->play_sound(IDToSound(SND_SQUISH), get_pos(),
      player.get_pos());
  physic.enable_gravity(true);
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  set_state(STATE_SQUISHED);
  player.bounce(*this);
}

void
BadGuy::kill_fall()
{
  SoundManager::get()->play_sound(IDToSound(SND_FALL), this,
      Sector::current()->player->get_pos());
  physic.set_velocity_y(0);
  physic.enable_gravity(true);
  set_state(STATE_FALLING);
}

void
BadGuy::set_state(State state)
{
  if(this->state == state)
    return;

  State laststate = this->state;
  this->state = state;
  switch(state) {
    case STATE_SQUISHED:
      state_timer.start(SQUISH_TIME);
      break;
    case STATE_ACTIVE:
      flags &= ~FLAG_NO_COLLDET;
      bbox.set_pos(start_position);
      break;
    case STATE_INACTIVE:
      // was the badguy dead anyway?
      if(laststate == STATE_SQUISHED || laststate == STATE_SQUISHED) {
        remove_me();
      }
      flags |= FLAG_NO_COLLDET;
      break;
    case STATE_FALLING:
      flags |= FLAG_NO_COLLDET;
      break;
    default:
      break;
  }
}

bool
BadGuy::is_offscreen()
{
  float scroll_x = Sector::current()->camera->get_translation().x;
  float scroll_y = Sector::current()->camera->get_translation().y;
     
  if(bbox.p2.x < scroll_x - X_OFFSCREEN_DISTANCE
      || bbox.p1.x > scroll_x + X_OFFSCREEN_DISTANCE
      || bbox.p2.y < scroll_y - Y_OFFSCREEN_DISTANCE
      || bbox.p1.y > scroll_y + Y_OFFSCREEN_DISTANCE)
    return true;

  return false;
}

void
BadGuy::try_activate()
{
  float scroll_x = Sector::current()->camera->get_translation().x;
  float scroll_y = Sector::current()->camera->get_translation().y;

  /* Activate badguys if they're just around the screen to avoid
   * the effect of having badguys suddenly popping up from nowhere.
   */
  if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
      start_position.x < scroll_x - bbox.get_width() &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = RIGHT;
    set_state(STATE_ACTIVE);
    activate();
  } else if (start_position.x > scroll_x &&
      start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } else if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
      start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
      ((start_position.y > scroll_y &&
        start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) ||
       (start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
        start_position.y < scroll_y))) {
    dir = start_position.x < scroll_x ? RIGHT : LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } else if(state == STATE_INIT
      && start_position.x > scroll_x - X_OFFSCREEN_DISTANCE
      && start_position.x < scroll_x + X_OFFSCREEN_DISTANCE
      && start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE
      && start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } 
}
