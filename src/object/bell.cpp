#include <config.h>

#include "bell.h"
#include "resources.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"
#include "player.h"
#include "object_factory.h"
#include "gameloop.h"
#include "sector.h"

Bell::Bell(const lisp::Lisp& lisp)
  : ringing(false)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("bell");
}

Bell::~Bell()
{
  delete sprite;
}

void
Bell::write(lisp::Writer& writer)
{
  writer.start_list("bell");
  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.end_list("bell");
}

void
Bell::action(float )
{
}

void
Bell::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_TILES);
}

HitResponse
Bell::collision(GameObject& other, const CollisionHit& )
{
  if(ringing)
    return ABORT_MOVE;
  
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    ringing = true;
    // TODO play sound
    sprite->set_action("ringing");
    GameSession::current()->set_reset_point(Sector::current()->get_name(),
        get_pos());
  }
  
  return ABORT_MOVE;
}

IMPLEMENT_FACTORY(Bell, "bell");
