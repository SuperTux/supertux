#include <config.h>

#include "invisible_block.h"
#include "resources.h"
#include "special/sprite.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"
#include "object_factory.h"

InvisibleBlock::InvisibleBlock(const Vector& pos)
  : Block(sprite_manager->create("invisibleblock")), visible(false)
{
  bbox.set_pos(pos);
  flags &= ~FLAG_SOLID;
}

void
InvisibleBlock::draw(DrawingContext& context)
{
  if(visible)
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

void
InvisibleBlock::hit(Player& )
{
  if(visible)
    return;

  sprite->set_action("empty");
  SoundManager::get()->play_sound(IDToSound(SND_BRICK));
  start_bounce();
  flags |= FLAG_SOLID;
  visible = true;
}

//IMPLEMENT_FACTORY(InvisibleBlock, "invisible_block");
