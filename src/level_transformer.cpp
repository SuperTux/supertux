#include <config.h>

#include "level_transformer.h"
#include "level.h"

LevelTransformer::~LevelTransformer()
{
}

void
LevelTransformer::transform(Level* level)
{
  for(size_t i = 0; i < level->get_sector_count(); ++i) {
    transform_sector(level->get_sector(i));
  }
}

