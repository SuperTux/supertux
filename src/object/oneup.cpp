#include <config.h>

#include "oneup.h"
#include "resources.h"
#include "player.h"
#include "scene.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"

OneUp::OneUp(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("1up");
  physic.set_velocity(100, 400);
}

OneUp::~OneUp()
{
  delete sprite;
}

void
OneUp::action(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time); 
}

void
OneUp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

HitResponse
OneUp::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    player->get_status().incLives();
    remove_me();
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

