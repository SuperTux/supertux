//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "flip_level_transformer.hpp"
#include "object/tilemap.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "badguy/badguy.hpp"
#include "sector.hpp"
#include "tile_manager.hpp"
#include "spawn_point.hpp"
#include "object/platform.hpp"
#include "object/block.hpp"

void
FlipLevelTransformer::transform_sector(Sector* sector)
{
  float height = sector->get_height();

  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    GameObject* object = *i;

    TileMap* tilemap = dynamic_cast<TileMap*> (object);
    if(tilemap) {
      transform_tilemap(tilemap);
    }
    Player* player = dynamic_cast<Player*> (object);
    if(player) {
      Vector pos = player->get_pos();
      pos.y = height - pos.y - player->get_bbox().get_height();
      player->move(pos);
      continue;
    }
    BadGuy* badguy = dynamic_cast<BadGuy*> (object);
    if(badguy) {
      transform_badguy(height, badguy);
    }
    Platform* platform = dynamic_cast<Platform*> (object);
    if(platform) {
      transform_platform(height, *platform);
    }
    Block* block = dynamic_cast<Block*> (object);
    if(block) {
      transform_block(height, *block);
    }
    MovingObject* mobject = dynamic_cast<MovingObject*> (object);
    if(mobject) {
      transform_moving_object(height, mobject);
    }
  }
  for(Sector::SpawnPoints::iterator i = sector->spawnpoints.begin();
      i != sector->spawnpoints.end(); ++i) {
    transform_spawnpoint(height, *i);
  }

  if(sector->camera != 0 && sector->player != 0)
    sector->camera->reset(sector->player->get_pos());
}

void
FlipLevelTransformer::transform_tilemap(TileMap* tilemap)
{
  for(size_t x = 0; x < tilemap->get_width(); ++x) {
    for(size_t y = 0; y < tilemap->get_height()/2; ++y) {
      // swap tiles
      int y2 = tilemap->get_height()-1-y;
      uint32_t t1 = tilemap->get_tile_id(x, y);
      uint32_t t2 = tilemap->get_tile_id(x, y2);
      tilemap->change(x, y, t2);
      tilemap->change(x, y2, t1);
    }
  }
  if(tilemap->get_drawing_effect() != 0) {
    tilemap->set_drawing_effect(NO_EFFECT);
  } else {
    tilemap->set_drawing_effect(VERTICAL_FLIP);
  }
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
  pos.y = height - pos.y - object->get_bbox().get_height();
  object->set_pos(pos);
}

void
FlipLevelTransformer::transform_platform(float height, Platform& platform)
{
  Path& path = platform.get_path();
  for (std::vector<Path::Node>::iterator i = path.nodes.begin(); i != path.nodes.end(); i++) {
    Vector& pos = i->position;
    pos.y = height - pos.y - platform.get_bbox().get_height();
  }
}

void
FlipLevelTransformer::transform_block(float height, Block& block)
{
  if (block.original_y != -1) block.original_y = height - block.original_y - block.get_bbox().get_height();
}
