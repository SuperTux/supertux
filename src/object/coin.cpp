#include <config.h>

#include "coin.h"
#include "resources.h"
#include "video/drawing_context.h"
#include "special/sprite_manager.h"
#include "player.h"
#include "sector.h"
#include "player_status.h"
#include "gameobjs.h"
#include "statistics.h"
#include "object_factory.h"

Coin::Coin(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("coin");
}

Coin::Coin(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
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

void
Coin::collect()
{
  Sector::current()->player->get_status()->incCoins();
  Sector::current()->add_object(new BouncyCoin(get_pos()));
  global_stats.add_points(COINS_COLLECTED_STAT, 1);
  remove_me();
}

HitResponse
Coin::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return ABORT_MOVE;

  collect();
  return ABORT_MOVE;
}

IMPLEMENT_FACTORY(Coin, "coin");
