#include <config.h>

#include "stalactite.h"

static const int SHAKE_RANGE = 40;
static const float SHAKE_TIME = .8;
static const float SQUISH_TIME = 2;

Stalactite::Stalactite(const lisp::Lisp& lisp)
{
  lisp.get("x", start_position.x);
  lisp.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("stalactite");
  state = STALACTITE_HANGING;
}

void
Stalactite::write(lisp::Writer& writer)
{
  writer.start_list("stalactite");
  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.end_list("stalactite");
}

void
Stalactite::active_action(float elapsed_time)
{
  if(state == STALACTITE_HANGING) {
    Player* player = Sector::current()->player;
    if(player->get_bbox().p2.x > bbox.p1.x - SHAKE_RANGE
        && player->get_bbox().p1.x < bbox.p2.x + SHAKE_RANGE
        && player->get_bbox().p2.y > bbox.p1.y) {
      timer.start(SHAKE_TIME);
      state = STALACTITE_SHAKING;
    }
  } else if(state == STALACTITE_SHAKING) {
    if(timer.check()) {
      state = STALACTITE_FALLING;
      physic.enable_gravity(true);
    }
  } else if(state == STALACTITE_FALLING || state == STALACTITE_SQUISHED) {
    movement = physic.get_movement(elapsed_time);
    if(state == STALACTITE_SQUISHED && timer.check())
      remove_me();
  }
}

HitResponse
Stalactite::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(state != STALACTITE_FALLING && state != STALACTITE_SQUISHED)
    return FORCE_MOVE;
  
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    state = STALACTITE_SQUISHED;
    physic.set_velocity_y(0);
    sprite->set_action("squished");
    if(!timer.started())
      timer.start(SQUISH_TIME);
  }

  return CONTINUE;
}

HitResponse
Stalactite::collision_player(Player& player, const CollisionHit& )
{
  if(state != STALACTITE_SQUISHED) {
    player.kill(Player::SHRINK);
  }

  return FORCE_MOVE;
}

void
Stalactite::kill_fall()
{
}

void
Stalactite::draw(DrawingContext& context)
{
  if(get_state() != STATE_ACTIVE)
    return;
    
  if(state == STALACTITE_SHAKING) {
    sprite->draw(context, get_pos() + Vector((rand() % 6)-3, 0), LAYER_OBJECTS);
  } else {
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
  }
}

void
Stalactite::deactivate()
{
  if(state != STALACTITE_HANGING)
    remove_me();
}

bool
Stalactite::is_harmful()
{
    if (state == STALACTITE_SQUISHED)
        return false;
    else
        return true;
}

IMPLEMENT_FACTORY(Stalactite, "stalactite")
