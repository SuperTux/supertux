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

#include "supertux/autotile_parser.hpp"

#include <sstream>
#include <sexp/value.hpp>
#include <sexp/io.hpp>

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/file_system.hpp"

AutotileParser::AutotileParser(std::vector<std::unique_ptr<AutotileSet>>& autotilesets, const std::string& filename) :
  m_autotilesets(autotilesets),
  m_filename(filename),
  m_tiles_path()
{
}

void
AutotileParser::parse()
{
  m_tiles_path = FileSystem::dirname(m_filename);

  auto doc = ReaderDocument::from_file(m_filename);
  auto root = doc.get_root();

  if (root.get_name() != "supertux-autotiles") {
    throw std::runtime_error("File is not a supertux autotile configuration file (Root list doesn't start with 'supertux-autotiles').");
  }

  auto iter = root.get_mapping().get_iter();
  while (iter.next())
  {
    if (iter.get_key() == "autotileset")
    {
      ReaderMapping tile_mapping = iter.as_mapping();
      parse_autotileset(tile_mapping, false);
    }
    else if (iter.get_key() == "autotileset-corner")
    {
      ReaderMapping tile_mapping = iter.as_mapping();
      parse_autotileset(tile_mapping, true);
    }
    else
    {
      log_warning << "Unknown symbol '" << iter.get_key() << "' in autotile config file" << std::endl;
    }
  }
}

void
AutotileParser::parse_autotileset(const ReaderMapping& reader, bool corner)
{
  std::vector<Autotile*> autotiles;

  std::string name = "[unnamed]";
  if (!reader.get("name", name))
  {
    log_warning << "Unnamed autotileset parsed" << std::endl;
  }

  uint32_t default_id = 0;
  if (!reader.get("default", default_id))
  {
    log_warning << "No default tile for autotileset " << name << std::endl;
  }

  auto iter = reader.get_iter();
  while (iter.next())
  {
    if (iter.get_key() == "autotile")
    {
      ReaderMapping tile_mapping = iter.as_mapping();
      autotiles.push_back(parse_autotile(tile_mapping, corner));
    }
    else if (iter.get_key() != "name" && iter.get_key() != "default")
    {
      log_warning << "Unknown symbol '" << iter.get_key() << "' in autotile config file" << std::endl;
    }
  }

  std::unique_ptr<AutotileSet> autotileset = std::make_unique<AutotileSet>(autotiles, default_id, name, corner);

  if (g_config->developer_mode)
  {
    autotileset->validate();
  }

  m_autotilesets.push_back(std::move(autotileset));
}

Autotile*
AutotileParser::parse_autotile(const ReaderMapping& reader, bool corner)
{
  std::vector<AutotileMask> autotile_masks;
  std::vector<std::pair<uint32_t, float>> alt_ids;

  uint32_t tile_id;
  if (!reader.get("id", tile_id))
  {
    throw std::runtime_error("Missing 'id' parameter in autotileset config file.");
  }

  bool solid;
  if (!reader.get("solid", solid))
  {
    if (!corner)
      throw std::runtime_error("Missing 'solid' parameter in autotileset config file.");
  }
  else
  {
    if (corner)
      throw std::runtime_error("'solid' parameter not needed for corner-based tiles in autotileset config file.");
  }

  auto iter = reader.get_iter();
  while (iter.next())
  {
    if (iter.get_key() == "mask")
    {
      std::string mask;
      iter.get(mask);

      if (corner)
      {
        parse_mask_corner(mask, autotile_masks);
      }
      else
      {
        parse_mask(mask, autotile_masks, solid);
      }
    }
    else if (iter.get_key() == "alt-id")
    {
      ReaderMapping alt_reader = iter.as_mapping();

      uint32_t alt_id = 0;
      if (!alt_reader.get("id", alt_id))
      {
        log_warning << "No alt tile for autotileset" << std::endl;
      }

      float weight = 0.0f;
      if (!alt_reader.get("weight", weight))
      {
        log_warning << "No weight for alt tile id" << std::endl;
      }

      if (alt_id != 0 && weight != 0.0f)
      {
        alt_ids.push_back(std::pair<uint32_t, float>(alt_id, weight));
      }
    }
    else if (iter.get_key() != "id" && iter.get_key() != "solid")
    {
      log_warning << "Unknown symbol '" << iter.get_key() << "' in autotile config file" << std::endl;
    }
  }

  return new Autotile(tile_id, alt_ids, autotile_masks, solid);
}

void
AutotileParser::parse_mask(std::string mask, std::vector<AutotileMask>& autotile_masks, bool solid)
{
  if (mask.size() != 8)
  {
    throw std::runtime_error("Autotile config : mask isn't exactly 8 characters.");
  }

  std::vector<uint8_t> masks;

  masks.push_back(0);

  for (int i = 0; i < 8; i++)
  {
    std::vector<uint8_t> new_masks;
    switch (mask[i])
    {
    case '0':
      for (uint8_t val : masks)
      {
        new_masks.push_back(static_cast<uint8_t>(val * 2));
      }
      break;
    case '1':
      for (uint8_t val : masks)
      {
        new_masks.push_back(static_cast<uint8_t>(val * 2 + 1));
      }
      break;
    case '*':
      for (uint8_t val : masks)
      {
        new_masks.push_back(static_cast<uint8_t>(val * 2));
        new_masks.push_back(static_cast<uint8_t>(val * 2 + 1));
      }
      break;
    default:
      throw std::runtime_error("Autotile config : unrecognized character");
    }
    masks = new_masks;
  }

  for (uint8_t val : masks)
  {
    autotile_masks.push_back(AutotileMask(val, solid));
  }
}

void
AutotileParser::parse_mask_corner(std::string mask, std::vector<AutotileMask>& autotile_masks)
{
  if (mask.size() != 4)
  {
    throw std::runtime_error("Autotile config : corner-based mask isn't exactly 4 characters.");
  }

  std::vector<uint8_t> masks;

  masks.push_back(0);

  for (int i = 0; i < 4; i++)
  {
    std::vector<uint8_t> new_masks;
    switch (mask[i])
    {
    case '0':
      for (uint8_t val : masks)
      {
        new_masks.push_back(static_cast<uint8_t>(val * 2));
      }
      break;
    case '1':
      for (uint8_t val : masks)
      {
        new_masks.push_back(static_cast<uint8_t>(val * 2 + 1));
      }
      break;
    case '*':
      for (uint8_t val : masks)
      {
        new_masks.push_back(static_cast<uint8_t>(val * 2));
        new_masks.push_back(static_cast<uint8_t>(val * 2 + 1));
      }
      break;
    default:
      throw std::runtime_error("Autotile config : unrecognized character");
    }
    masks = new_masks;
  }

  for (uint8_t val : masks)
  {
    autotile_masks.push_back(AutotileMask(val, true));
  }
}

/* EOF */
