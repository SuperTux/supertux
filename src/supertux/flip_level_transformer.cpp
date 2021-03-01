//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/flip_level_transformer.hpp"

#include "badguy/badguy.hpp"
#include "object/block.hpp"
#include "object/camera.hpp"
#include "object/decal.hpp"
#include "object/flower.hpp"
#include "object/platform.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "supertux/sector.hpp"

void
FlipLevelTransformer::transform_sector(Sector& sector)
{
  float height = sector.get_height();

  for (auto& object : sector.get_objects()) {
    auto tilemap = dynamic_cast<TileMap*>(object.get());
    if (tilemap) {
      transform_tilemap(height, *tilemap);
    }
    auto player = dynamic_cast<Player*>(object.get());
    if (player) {
      Vector pos = player->get_pos();
      pos.y = height - pos.y - player->get_bbox().get_height();
      player->move(pos);
      continue;
    }
    auto badguy = dynamic_cast<BadGuy*>(object.get());
    if (badguy) {
      transform_badguy(height, *badguy);
    }
    auto flower = dynamic_cast<Flower*>(object.get());
    if (flower) {
      transform_flower(*flower);
    }
    auto platform = dynamic_cast<Platform*>(object.get());
    if (platform) {
      transform_platform(height, *platform);
    }
    auto block = dynamic_cast<Block*>(object.get());
    if (block) {
      transform_block(height, *block);
    }
    auto mobject = dynamic_cast<MovingObject*>(object.get());
    if (mobject) {
      transform_moving_object(height, *mobject);
    }
    auto decal = dynamic_cast<Decal*>(object.get());
    if (decal) {
      transform_decal(height, *decal);
    }
  }

  sector.get_camera().reset(sector.get_player().get_pos());
}

Flip
FlipLevelTransformer::transform_flip(Flip flip)
{
  if (flip & VERTICAL_FLIP) {
    return flip & ~VERTICAL_FLIP;
  } else {
    return flip | VERTICAL_FLIP;
  }
}

void
FlipLevelTransformer::transform_path(float height, float obj_height, Path& path)
{
  for (auto& node : path.m_nodes) {
    Vector& pos = node.position;
    pos.y = height - pos.y - obj_height;
  }
}

void
FlipLevelTransformer::transform_tilemap(float height, TileMap& tilemap)
{
  for (int x = 0; x < tilemap.get_width(); ++x) {
    for (int y = 0; y < tilemap.get_height()/2; ++y) {
      // swap tiles
      int y2 = tilemap.get_height()-1-y;
      uint32_t t1 = tilemap.get_tile_id(x, y);
      uint32_t t2 = tilemap.get_tile_id(x, y2);
      tilemap.change(x, y, t2);
      tilemap.change(x, y2, t1);
    }
  }
  tilemap.set_flip(transform_flip(tilemap.get_flip()));
  Vector offset = tilemap.get_offset();
  offset.y = height - offset.y - tilemap.get_bbox().get_height();
  tilemap.set_offset(offset);
  if (Path* const path = tilemap.get_path()) {
    transform_path(height, tilemap.get_bbox().get_height(), *path);
  }
}

void
FlipLevelTransformer::transform_badguy(float height, BadGuy& badguy)
{
  Vector pos = badguy.get_start_position();
  pos.y = height - pos.y;
  badguy.set_start_position(pos);
}

void
FlipLevelTransformer::transform_decal(float height, Decal& decal)
{
  decal.m_flip = transform_flip(decal.m_flip);
}

void
FlipLevelTransformer::transform_moving_object(float height, MovingObject& object)
{
  Vector pos = object.get_pos();
  pos.y = height - pos.y - object.get_bbox().get_height();
  object.set_pos(pos);
}

void
FlipLevelTransformer::transform_flower(Flower& flower)
{
  flower.flip = transform_flip(flower.flip);
}

void
FlipLevelTransformer::transform_platform(float height, Platform& platform)
{
  if (Path* const path = platform.get_path()) {
    transform_path(height, platform.get_bbox().get_height(), *path);
  }
}

void
FlipLevelTransformer::transform_block(float height, Block& block)
{
  if (block.m_original_y != -1) block.m_original_y = height - block.m_original_y - block.get_bbox().get_height();
}

/* EOF */
