#include <config.h>

#include "bomb.h"

static const float TICKINGTIME = 1;
static const float EXPLOSIONTIME = 1;

Bomb::Bomb(const Vector& pos, Direction dir)
{
  start_position = pos;
  bbox.set_pos(pos);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("bomb");
  state = 0;
  timer.start(TICKINGTIME);
  this->dir = dir;
  sprite->set_action(dir == LEFT ? "ticking-left" : "ticking-right");
}

void
Bomb::write(lisp::Writer& )
{
  // bombs are only temporarily so don't write them out...
}

HitResponse
Bomb::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5)
    physic.set_velocity_y(0);

  return CONTINUE;
}

HitResponse
Bomb::collision_player(Player& player, const CollisionHit& )
{
  if(state == 1) {
    player.kill(Player::SHRINK);
  }
  return ABORT_MOVE;
}

HitResponse
Bomb::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  if(state == 1)
    badguy.kill_fall();
  return ABORT_MOVE;
}

void
Bomb::active_action(float )
{
  switch(state) {
    case 0:
      if(timer.check()) {
        explode();
      }
      break;
    case 1:
      if(timer.check()) {
        remove_me();
      }
      break;
  } 
}

void
Bomb::explode()
{
  state = 1;
  sprite->set_action("explosion");
  SoundManager::get()->play_sound(IDToSound(SND_EXPLODE), get_pos(),
      Sector::current()->player->get_pos());
  timer.start(EXPLOSIONTIME);
}

void
Bomb::kill_fall()
{
  explode();
}

