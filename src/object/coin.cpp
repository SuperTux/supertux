#include <config.h>

#include "coin.h"
#include "resources.h"
#include "video/drawing_context.h"
#include "special/sprite_manager.h"
#include "player.h"
#include "scene.h"

Coin::Coin(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("coin");
}

Coin::~Coin()
{
  delete sprite;
}

void
Coin::action(float )
{
}

void
Coin::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_TILES);
}

HitResponse
Coin::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return ABORT_MOVE;

  player->get_status().incCoins();
  remove_me();
  return ABORT_MOVE;
}

