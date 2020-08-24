//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "supertux/autotile.hpp"

#include "supertux/autotile_parser.hpp"

// AutotileMask

AutotileMask::AutotileMask(uint8_t mask, bool center) :
  m_mask(mask),
  m_center(center)
{
}

bool
AutotileMask::matches(uint8_t mask, bool center) const
{
  return mask == m_mask && center == m_center;
}

// Autotile

Autotile::Autotile(uint32_t tile_id, std::vector<AutotileMask*> masks, bool solid) :
  m_tile_id(tile_id),
  m_masks(masks),
  m_solid(solid)
{
}

bool
Autotile::matches(uint8_t num_mask, bool center) const
{
  for (auto& l_mask : m_masks)
  {
    if (l_mask->matches(num_mask, center))
    {
      return true;
    }
  }
  return false;
}

uint32_t
Autotile::get_tile_id() const
{
  return m_tile_id;
}

bool
Autotile::is_solid() const
{
  return m_solid;
}


// AutotileSet

std::vector<AutotileSet*>* AutotileSet::m_autotilesets = new std::vector<AutotileSet*>();

AutotileSet::AutotileSet(std::vector<Autotile*> tiles, uint32_t default_tile) :
  m_autotiles(tiles),
  m_default(default_tile)
{
}

AutotileSet*
AutotileSet::get_tileset_from_tile(uint32_t tile_id)
{
  if (tile_id == 0)
  {
    return nullptr;
  }

  if (m_autotilesets->size() == 0)
  {
    // TODO: Add possibility to include external autotile config files
    AutotileParser* parser = new AutotileParser(m_autotilesets, "/images/autotiles.satc");
    parser->parse();
  }

  for (auto& ats : *m_autotilesets)
  {
    if (ats->is_member(tile_id))
    {
      return ats;
    }
  }
  return nullptr;
}

uint32_t
AutotileSet::get_autotile(uint32_t tile_id,
    bool top_left, bool top, bool top_right,
    bool left, bool center, bool right,
    bool bottom_left, bool bottom, bool bottom_right
  ) const
{
  //if (tile_id == 0) return 0;
  uint8_t num_mask = 0;

  if (bottom_right) num_mask += 0x01;
  if (bottom) num_mask += 0x02;
  if (bottom_left) num_mask += 0x04;
  if (right) num_mask += 0x08;
  if (left) num_mask += 0x10;
  if (top_right) num_mask += 0x20;
  if (top) num_mask += 0x40;
  if (top_left) num_mask += 0x80;

  for (auto& autotile : m_autotiles)
  {
    if (autotile->matches(num_mask, center))
    {
      return autotile->get_tile_id();
    }
  }

  return center ? get_default_tile() : 0;
}



uint32_t
AutotileSet::get_default_tile() const
{
  return m_default;
}

bool
AutotileSet::is_member(uint32_t tile_id) const
{
  for (auto& tile : m_autotiles)
  {
    if (tile->get_tile_id() == tile_id)
    {
      return true;
    }
  }
  return false;
}

bool
AutotileSet::is_solid(uint32_t tile_id) const
{
  if (!is_member(tile_id))
    return false;

  for (auto& tile : m_autotiles)
  {
    if (tile->get_tile_id() == tile_id)
    {
      return tile->is_solid();
    }
  }

  log_warning << "Called AutotileSet::is_solid() with a tile_id that isn't in the Autotileset, yet that returns is_member() = true." << std::endl;
  return false;
}

/* EOF */
