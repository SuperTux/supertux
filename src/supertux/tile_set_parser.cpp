//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
//                     Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/tile_set_parser.hpp"

#include <stdexcept>
#include <sstream>
#include <sexp/value.hpp>
#include <sexp/util.hpp>
#include <sexp/io.hpp>

#include "editor/editor.hpp"
#include "supertux/tile_set.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/file_system.hpp"

TileSetParser::TileSetParser(TileSet& tileset, const std::string& filename) :
  m_tileset(tileset),
  m_filename(filename),
  m_tiles_path()
{
}

void
TileSetParser::parse()
{
  m_tiles_path = FileSystem::dirname(m_filename);

  auto doc = ReaderDocument::parse(m_filename);
  auto root = doc.get_root();

  if(root.get_name() != "supertux-tiles") {
    throw std::runtime_error("file is not a supertux tiles file.");
  }

  auto iter = root.get_mapping().get_iter();
  while(iter.next())
  {
    if (iter.get_key() == "tile")
    {
      ReaderMapping tile_mapping = iter.as_mapping();
      parse_tile(tile_mapping);
    }
    else if (iter.get_key() == "tilegroup")
    {
      /* tilegroups are only interesting for the editor */
      ReaderMapping reader = iter.as_mapping();
      Tilegroup tilegroup;
      reader.get("name", tilegroup.name);
      reader.get("tiles", tilegroup.tiles);
      m_tileset.tilegroups.push_back(tilegroup);
    }
    else if (iter.get_key() == "tiles")
    {
      ReaderMapping tiles_mapping = iter.as_mapping();
      parse_tiles(tiles_mapping);
    }
    else
    {
      log_warning << "Unknown symbol '" << iter.get_key() << "' in tileset file" << std::endl;
    }
  }
}

void
TileSetParser::parse_tile(const ReaderMapping& reader)
{
  uint32_t id;
  if (!reader.get("id", id))
  {
    throw std::runtime_error("Missing tile-id.");
  }

  uint32_t attributes = 0;

  bool value = false;
  if(reader.get("solid", value) && value)
    attributes |= Tile::SOLID;
  if(reader.get("unisolid", value) && value)
    attributes |= Tile::UNISOLID | Tile::SOLID;
  if(reader.get("brick", value) && value)
    attributes |= Tile::BRICK;
  if(reader.get("ice", value) && value)
    attributes |= Tile::ICE;
  if(reader.get("water", value) && value)
    attributes |= Tile::WATER;
  if(reader.get("hurts", value) && value)
    attributes |= Tile::HURTS;
  if(reader.get("fire", value) && value)
    attributes |= Tile::FIRE;
  if(reader.get("fullbox", value) && value)
    attributes |= Tile::FULLBOX;
  if(reader.get("coin", value) && value)
    attributes |= Tile::COIN;
  if(reader.get("goal", value) && value)
    attributes |= Tile::GOAL;

  uint32_t data = 0;

  if(reader.get("north", value) && value)
    data |= Tile::WORLDMAP_NORTH;
  if(reader.get("south", value) && value)
    data |= Tile::WORLDMAP_SOUTH;
  if(reader.get("west", value) && value)
    data |= Tile::WORLDMAP_WEST;
  if(reader.get("east", value) && value)
    data |= Tile::WORLDMAP_EAST;
  if(reader.get("stop", value) && value)
    data |= Tile::WORLDMAP_STOP;

  reader.get("data", data);

  float fps = 10;
  reader.get("fps", fps);

  std::string object_name, object_data;
  reader.get("object-name", object_name);
  reader.get("object-data", object_data);

  if(reader.get("slope-type", data))
  {
    attributes |= Tile::SOLID | Tile::SLOPE;
  }

  std::vector<Tile::ImageSpec> editor_imagespecs;
  ReaderMapping editor_images;
  if(reader.get("editor-images", editor_images)) {
    editor_imagespecs = parse_imagespecs(editor_images);
  }

  std::vector<Tile::ImageSpec> imagespecs;
  ReaderMapping images;
  if(reader.get("images", images)) {
    imagespecs = parse_imagespecs(images);
  }

  std::unique_ptr<Tile> tile(new Tile(imagespecs, editor_imagespecs, attributes, data, fps,
                                      object_name, object_data));
  m_tileset.add_tile(id, std::move(tile));
}

void
TileSetParser::parse_tiles(const ReaderMapping& reader)
{
  // List of ids (use 0 if the tile should be ignored)
  std::vector<uint32_t> ids;
  // List of attributes of the tile
  std::vector<uint32_t> attributes;
  // List of data for the tiles
  std::vector<uint32_t> datas;
  // Name used to report errors.
  std::string image_name;

  // width and height of the image in tile units, this is used for two
  // purposes:
  //  a) so we don't have to load the image here to know its dimensions
  //  b) so that the resulting 'tiles' entry is more robust,
  //  ie. enlarging the image won't break the tile id mapping
  // FIXME: height is actually not used, since width might be enough for
  // all purposes, still feels somewhat more natural this way
  unsigned int width  = 0;
  unsigned int height = 0;

  bool has_ids = reader.get("ids",        ids);
  bool has_attributes = reader.get("attributes", attributes);
  bool has_datas = reader.get("datas", datas);

  std::vector<Tile::ImageSpec> editor_imagespecs;
  ReaderMapping editor_images;
  if(reader.get("editor-images", editor_images)) {
    editor_imagespecs = parse_imagespecs(editor_images);
  }

  std::vector<Tile::ImageSpec> imagespecs;
  ReaderMapping images;
  if(reader.get("image", images) ||
     reader.get("images", images)) {
    imagespecs = parse_imagespecs(images);
  }

  if (imagespecs.size() > 0)
    image_name = imagespecs[0].file;
  else
    image_name = "(no image)";

  reader.get("width",      width);
  reader.get("height",     height);

  float fps = 10;
  reader.get("fps",     fps);

  if (ids.empty() || !has_ids)
  {
    throw std::runtime_error("No IDs specified.");
  }
  if (width == 0)
  {
    throw std::runtime_error("Width is zero.");
  }
  else if (height == 0)
  {
    throw std::runtime_error("Height is zero.");
  }
  else if (fps < 0)
  {
    throw std::runtime_error("Negative fps.");
  }
  else if (ids.size() != width*height)
  {
    std::ostringstream err;
    err << "Number of ids (" << ids.size() <<  ") and "
      "dimensions of image (" << width << "x" << height << " = " << width*height << ") "
      "differ for image " << image_name;
    throw std::runtime_error(err.str());
  }
  else if (has_attributes && (ids.size() != attributes.size()))
  {
    std::ostringstream err;
    err << "Number of ids (" << ids.size() <<  ") and attributes (" << attributes.size()
        << ") mismatch for image '" << image_name << "', but must be equal";
    throw std::runtime_error(err.str());
  }
  else if (has_datas && ids.size() != datas.size())
  {
    std::ostringstream err;
    err << "Number of ids (" << ids.size() <<  ") and datas (" << datas.size()
        << ") mismatch for image '" << image_name << "', but must be equal";
    throw std::runtime_error(err.str());
  }
  else
  {
    for(std::vector<uint32_t>::size_type i = 0; i < ids.size() && i < width*height; ++i)
    {
      if (ids[i] != 0)
      {
        int x = 32*(i % width);
        int y = 32*(i / width);

        std::vector<Tile::ImageSpec> tile_imagespecs;
        for(size_t j = 0; j < imagespecs.size(); ++j)
        {
          tile_imagespecs.push_back(Tile::ImageSpec(imagespecs[j].file,
                                                    Rectf(x + imagespecs[j].rect.get_left(),
                                                          y + imagespecs[j].rect.get_top(),
                                                          x + imagespecs[j].rect.get_left() + 32,
                                                          y + imagespecs[j].rect.get_top() + 32)));
        }

        std::vector<Tile::ImageSpec> tile_editor_imagespecs;
        for(size_t j = 0; j < editor_imagespecs.size(); ++j)
        {
          tile_editor_imagespecs.push_back(Tile::ImageSpec(editor_imagespecs[j].file,
                                                           Rectf(x + editor_imagespecs[j].rect.get_left(),
                                                                 y + editor_imagespecs[j].rect.get_top(),
                                                                 x + editor_imagespecs[j].rect.get_left() + 32,
                                                                 y + editor_imagespecs[j].rect.get_top() + 32)));
        }

        std::unique_ptr<Tile> tile(new Tile(tile_imagespecs, tile_editor_imagespecs,
                                            (has_attributes ? attributes[i] : 0),
                                            (has_datas ? datas[i] : 0),
                                            fps));

        m_tileset.add_tile(ids[i], std::move(tile));
      }
    }
  }
}

std::vector<Tile::ImageSpec>
TileSetParser::parse_imagespecs(const ReaderMapping& images_lisp) const
{
  std::vector<Tile::ImageSpec> imagespecs;

  // (images "foo.png" "foo.bar" ...)
  // (images (region "foo.png" 0 0 32 32))
  auto iter = images_lisp.get_iter();
  while(iter.next())
  {
    if(iter.is_string())
    {
      std::string file = iter.as_string_item();
      imagespecs.push_back(Tile::ImageSpec(m_tiles_path + file, Rectf(0, 0, 0, 0)));
    }
    else if(iter.is_pair() && iter.get_key() == "region")
    {
      auto const& sx = iter.as_mapping().get_sexp();
      auto const& arr = sx.as_array();
      if (arr.size() != 6)
      {
        log_warning << "(region X Y WIDTH HEIGHT) tag malformed: " << sx << std::endl;
      }
      else
      {
        std::string file = arr[1].as_string();
        float x = arr[2].as_float();
        float y = arr[3].as_float();
        float w = arr[4].as_float();
        float h = arr[5].as_float();

        imagespecs.push_back(Tile::ImageSpec(m_tiles_path + file, Rectf(x, y, x+w, y+h)));
      }
    }
    else
    {
      log_warning << "Expected string or list in images tag" << std::endl;
    }
  }

  return imagespecs;
}

/* EOF */
