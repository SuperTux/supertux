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

#include <bitset>

#include "supertux/autotile.hpp"

//#include "supertux/autotile_parser.hpp"

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

uint8_t
AutotileMask::get_mask() const
{
  return m_mask;
}

// Autotile

Autotile::Autotile(uint32_t tile_id, std::vector<std::pair<uint32_t, float>> alt_tiles, std::vector<std::shared_ptr<AutotileMask>> masks, bool solid) :
  m_tile_id(tile_id),
  m_alt_tiles(std::move(alt_tiles)),
  m_masks(std::move(masks)),
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

uint32_t
Autotile::pick_tile(int x, int y) const
{
  // Needed? Not needed?
  // Could avoid pointless computation
  if (m_alt_tiles.size() == 0)
    return m_tile_id;

  // srand() and rand() are inconsistent across platforms (Windows)
  //srand(x * 32768 + y);
  //float rnd_val = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

  float rnd_val = static_cast<float>(
    (
      x * 371 + y * 49173 + (x * x * y) % 32769 + (y * y * x + x * x * x ) % 65537
    ) % 256
  ) / 256.0f;

  for (auto& pair : m_alt_tiles)
  {
    rnd_val -= pair.second;
    if (rnd_val <= 0)
    {
      return pair.first;
    }
  }

  return m_tile_id;
}

bool
Autotile::is_amongst(uint32_t tile) const
{
  if (tile == m_tile_id)
    return true;

  for (auto& pair : m_alt_tiles)
    if (pair.first == tile)
      return true;

  return false;
}

uint8_t
Autotile::get_first_mask() const
{
  if (!m_masks.empty())
    return m_masks[0]->get_mask();
  return 0;
}

std::vector<std::pair<uint32_t, float>>
Autotile::get_all_tile_ids() const
{
  return m_alt_tiles;
}

bool
Autotile::is_solid() const
{
  return m_solid;
}


// AutotileSet

std::vector<std::unique_ptr<AutotileSet>>* AutotileSet::m_autotilesets = new std::vector<std::unique_ptr<AutotileSet>>();

AutotileSet::AutotileSet(std::vector<std::shared_ptr<Autotile>> tiles, uint32_t default_tile, std::string name, bool corner) :
  m_autotiles(std::move(tiles)),
  m_default(default_tile),
  m_name(std::move(name)),
  m_corner(corner)
{
}

/*
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
*/

uint32_t
AutotileSet::get_autotile(uint32_t tile_id,
    bool top_left, bool top, bool top_right,
    bool left, bool center, bool right,
    bool bottom_left, bool bottom, bool bottom_right,
    int x, int y
  ) const
{
  uint8_t num_mask = 0;

  if (m_corner)
  {
    if (bottom_right) num_mask = static_cast<uint8_t>(num_mask + 0x01);
    if (bottom_left)  num_mask = static_cast<uint8_t>(num_mask + 0x02);
    if (top_right)    num_mask = static_cast<uint8_t>(num_mask + 0x04);
    if (top_left)     num_mask = static_cast<uint8_t>(num_mask + 0x08);
    center = true;
  }
  else
  {
    // num_mask += 0x01;
    //   clang will complain
    // num_mask += static_cast<uint8_t>(0x01);
    //   gcc will complain
    // num_mask = (num_mask + 0x01) & 0xff;
    //   (from a stackoverflow.com question) gcc still complains
    // EDIT : It appears that GCC makes all integers calculations in "int" type,
    //        so you have to re-cast every single time :^)
    if (bottom_right) num_mask = static_cast<uint8_t>(num_mask + 0x01);
    if (bottom)       num_mask = static_cast<uint8_t>(num_mask + 0x02);
    if (bottom_left)  num_mask = static_cast<uint8_t>(num_mask + 0x04);
    if (right)        num_mask = static_cast<uint8_t>(num_mask + 0x08);
    if (left)         num_mask = static_cast<uint8_t>(num_mask + 0x10);
    if (top_right)    num_mask = static_cast<uint8_t>(num_mask + 0x20);
    if (top)          num_mask = static_cast<uint8_t>(num_mask + 0x40);
    if (top_left)     num_mask = static_cast<uint8_t>(num_mask + 0x80);
  }

  for (auto& autotile : m_autotiles)
  {
    if (autotile->matches(num_mask, center))
    {
      return autotile->pick_tile(x, y);
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
    else
    {
      for (auto& pair : tile->get_all_tile_ids())
      {
        if (pair.first == tile_id)
        {
          return true;
        }
      }
    }
  }
  // m_default should *never* be 0 (always a valid solid tile,
  //   even if said tile isn't part of the tileset)
  return tile_id == m_default && m_default != 0;
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
    else
    {
      for (auto& pair : tile->get_all_tile_ids())
      {
        if (pair.first == tile_id)
        {
          return tile->is_solid();
        }
      }
    }
  }

  //log_warning << "Called AutotileSet::is_solid() with a tile_id that isn't in the Autotileset, yet that returns is_member() = true." << std::endl;

  // m_default should *never* be 0 (always a valid solid tile,
  //   even if said tile isn't part of the tileset)
  return tile_id == m_default && m_default != 0;
}

uint8_t
AutotileSet::get_mask_from_tile(uint32_t tile) const
{
  for (auto& autotile : m_autotiles)
  {
    if (autotile->is_amongst(tile)) {
      return autotile->get_first_mask();
    }
  }
  return static_cast<uint8_t>(0);
}

void
AutotileSet::validate() const
{
  // Corner autotiles are always empty if all 4 corners are, but regular
  // autotiles should have a valid tile ID that can be surrounded by emptiness
  for (int mask = (m_corner ? 1 : 0); mask <= (m_corner ? 15 : 255); mask++)
  {
    uint8_t num_mask = static_cast<uint8_t>(mask);
    bool tile_exists = false;
    uint32_t tile_with_that_mask = 0; // Used to help users debug

    for (auto& autotile : m_autotiles)
    {
      if (autotile->matches(num_mask, true))
      {
        if (tile_exists)
        {
          log_warning << "Autotileset '" << m_name << "': mask " << (m_corner ? std::bitset<4>(mask).to_string() : std::bitset<8>(mask).to_string()) << " corresponds both to tile " << tile_with_that_mask << " and " << autotile->get_tile_id() << std::endl;
        }
        else
        {
          tile_exists = true;
          tile_with_that_mask = autotile->get_tile_id();
        }
      }
    }

    if (!tile_exists)
    {
      log_warning << "Autotileset '" << m_name << "': mask " << (m_corner ? std::bitset<4>(mask).to_string() : std::bitset<8>(mask).to_string()) << " has no corresponding tile" << std::endl;
    }
  }
}

/* EOF */
