#include <config.h>

#include "unstable_tile.h"
#include "lisp/lisp.h"
#include "object_factory.h"
#include "player.h"
#include "sector.h"
#include "resources.h"
#include "special/sprite_manager.h"
#include "special/sprite.h"

static const float CRACKTIME = 0.3;
static const float FALLTIME = 0.8;

UnstableTile::UnstableTile(const lisp::Lisp& lisp)
  : hit(false), falling(false)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("unstable_tile");
  flags |= FLAG_SOLID;
}

UnstableTile::~UnstableTile()
{
  delete sprite;
}

HitResponse
UnstableTile::collision(GameObject& other, const CollisionHit& hitdata)
{
  if(hitdata.normal.y < 0.8)
    return FORCE_MOVE;

  Player* player = dynamic_cast<Player*> (&other);
  if(player)
    hit = true;

  return FORCE_MOVE;
}

void
UnstableTile::draw(DrawingContext& context)
{
  Vector pos = get_pos();
  // shacking
  if(timer.get_timegone() > CRACKTIME) {
    pos.x += (rand() % 6) - 3;
  } 

  sprite->draw(context, pos, LAYER_TILES);
}

void
UnstableTile::action(float elapsed_time)
{
  if(falling) {
    movement = physic.get_movement(elapsed_time);
    if(!Sector::current()->inside(bbox)) {
      remove_me();
      return;
    }
  } else if(hit) {
    if(timer.check()) {
      falling = true;
      physic.enable_gravity(true);      
      flags &= ~FLAG_SOLID;
      timer.stop();
    } else if(!timer.started()) {
      timer.start(FALLTIME);
    }
  } else {
    timer.stop();
  }
  hit = false;
}

IMPLEMENT_FACTORY(UnstableTile, "unstable_tile");
