#include <config.h>

#include "infoblock.h"
#include "gameloop.h"
#include "resources.h"
#include "special/sprite_manager.h"
#include "object_factory.h"
#include "lisp/lisp.h"

InfoBlock::InfoBlock(const lisp::Lisp& lisp)
  : Block(sprite_manager->create("bonusblock"))
{
  Vector pos;
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  bbox.set_pos(pos);

  if(!lisp.get("message", message)) {
    std::cerr << "No message in InfoBlock!\n";
  }
}

InfoBlock::~InfoBlock()
{
}

void
InfoBlock::hit(Player& )
{
  GameSession::current()->display_info_box(message);
  start_bounce();
}

IMPLEMENT_FACTORY(InfoBlock, "infoblock")
