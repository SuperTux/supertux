#ifndef __FLIP_LEVEL_TRANSFORMER_H__
#define __FLIP_LEVEL_TRANSFORMER_H__

#include "level_transformer.h"

class TileMap;
class BadGuy;
class SpawnPoint;
namespace SuperTux {
class MovingObject;
}

using namespace SuperTux;

/** Vertically or horizontally flip a level */
class FlipLevelTransformer : public LevelTransformer
{
public:
  virtual void transform_sector(Sector* sector);

private:
  void transform_tilemap(TileMap* tilemap);
  void transform_moving_object(float height, MovingObject* object);
  void transform_badguy(float height, BadGuy* badguy);
  void transform_spawnpoint(float height, SpawnPoint* spawnpoint);
};

#endif

