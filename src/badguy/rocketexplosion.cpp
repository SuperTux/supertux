#include <config.h>

#include "rocketexplosion.h"

static const float EXPLOSIONTIME = 1;

RocketExplosion::RocketExplosion(const Vector& pos, Direction dir)
{
  start_position = pos;
  bbox.set_pos(pos);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("rocketexplosion");
  this->dir = dir;
  explode();
}

void
RocketExplosion::write(lisp::Writer& )
{
  // bombs are only temporarily so don't write them out...
}

HitResponse
RocketExplosion::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5)
    physic.set_velocity_y(0);

  return CONTINUE;
}

HitResponse
RocketExplosion::collision_player(Player& player, const CollisionHit& )
{
  player.kill(Player::SHRINK);
  return ABORT_MOVE;
}

HitResponse
RocketExplosion::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
   badguy.kill_fall();
   return ABORT_MOVE;
}

void
RocketExplosion::active_action(float )
{
   if(timer.check()) {
      remove_me();
   }
}

void
RocketExplosion::explode()
{
  sprite->set_action(dir == LEFT ? "explosion-left" : "explosion-right");
  SoundManager::get()->play_sound(IDToSound(SND_EXPLODE), get_pos(),
      Sector::current()->player->get_pos());
  timer.start(EXPLOSIONTIME, true);
}

void
RocketExplosion::kill_fall()
{
  explode();
}

