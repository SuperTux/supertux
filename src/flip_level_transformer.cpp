#include <config.h>

#include "flip_level_transformer.h"
#include "object/tilemap.h"
#include "badguy/badguy.h"
#include "sector.h"
#include "tile_manager.h"

void
FlipLevelTransformer::transform_sector(Sector* sector)
{
  float height = sector->solids->get_height() 
    * sector->solids->get_tilemanager()->get_default_height();
  
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    GameObject* object = *i;

    TileMap* tilemap = dynamic_cast<TileMap*> (object);
    if(tilemap) {
      transform_tilemap(tilemap);
    }
    BadGuy* badguy = dynamic_cast<BadGuy*> (object);
    if(badguy) {
      transform_badguy(height, badguy);
    } else {
      MovingObject* mobject = dynamic_cast<MovingObject*> (object);
      if(mobject) {
        transform_moving_object(height, mobject);
      }
    }
  }
  for(Sector::SpawnPoints::iterator i = sector->spawnpoints.begin();
      i != sector->spawnpoints.end(); ++i) {
    transform_spawnpoint(height, *i);
  }
}

void
FlipLevelTransformer::transform_tilemap(TileMap* tilemap)
{
  for(size_t x = 0; x < tilemap->get_width(); ++x) {
    for(size_t y = 0; y < tilemap->get_height()/2; ++y) {
      // swap tiles
      int y2 = tilemap->get_height()-1-y;
      const Tile* t1 = tilemap->get_tile(x, y);
      const Tile* t2 = tilemap->get_tile(x, y2);
      tilemap->change(x, y, t2->getID());
      tilemap->change(x, y2, t1->getID());
    }
  }
  tilemap->set_drawing_effect(VERTICAL_FLIP);
}

void
FlipLevelTransformer::transform_badguy(float height, BadGuy* badguy)
{
  Vector pos = badguy->get_start_position();
  pos.y = height - pos.y;
  badguy->set_start_position(pos);
}

void
FlipLevelTransformer::transform_spawnpoint(float height, SpawnPoint* spawn)
{
  Vector pos = spawn->pos;
  pos.y = height - pos.y;
  spawn->pos = pos;
}

void
FlipLevelTransformer::transform_moving_object(float height, MovingObject*object)
{
  Vector pos = object->get_pos();
  pos.y = height - pos.y;
  object->set_pos(pos);
}

