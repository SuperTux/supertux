#include <config.h>

#include "invisible_tile.h"
#include "resources.h"
#include "special/sprite.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"

InvisibleTile::InvisibleTile(const Vector& pos)
  : Block(pos, sprite_manager->create("invisibleblock")), visible(false)
{
  flags |= FLAG_SOLID;
}

void
InvisibleTile::draw(DrawingContext& context)
{
}

void
InvisibleTile::hit(Player& )
{
}

