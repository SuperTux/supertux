//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "editor/editor.hpp"
#include "editor/editor_tile_converter.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_stream.hpp"
#include "supertux/level.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

using ConverterInfo = EditorTileConverter::ConverterInfo;

EditorTileConverter::EditorTileConverter() :
  m_converters(),
  m_has_deprecated_tiles()
{
  load_definitions();
}

void
EditorTileConverter::load_definitions(const std::string& filepath)
{
  m_converters.clear();

  try
  {
    auto doc = ReaderDocument::from_file(filepath);
    auto root = doc.get_root();
    if (root.get_name() != "supertux-converter-data")
      throw std::runtime_error("File is not a 'supertux-converters-data' file.");

    auto iter = root.get_mapping().get_iter();
    while (iter.next())
    {
      if (iter.get_key().empty())
        continue;

      ConverterInfo converter;

      auto mapping = iter.as_mapping();
      mapping.get("title", converter.title);
      mapping.get("author", converter.author);
      mapping.get("description", converter.description);

      m_converters.insert({ iter.get_key(), converter });
    }
  }
  catch (std::exception& err)
  {
    log_warning << "Cannot read converter data from '" << filepath << "': " << err.what() << std::endl;
  }
}

const ConverterInfo*
EditorTileConverter::get_tile_converter_info_for_file(const std::string& file_path) const
{
  std::string basename = FileSystem::basename(file_path);
  auto it = m_converters.find(basename);
  if (it == m_converters.end())
    return nullptr;

  return &(it->second);
}

void
EditorTileConverter::check_deprecated_tiles(bool focus)
{
  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto level = editor_project->get_level();

  m_has_deprecated_tiles = false;
  for (const auto& sector : level->get_sectors())
  {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>())
    {
      int idx = -1;
      for (const uint32_t& tile_id : tilemap.get_tiles())
      {
        idx++;
        if (editor_project->get_tileset()->get(tile_id).is_deprecated())
        {
          // Focus on deprecated tile
          if (focus)
          {
            focus_on_tile(sector.get(), &tilemap, idx);
          }

          m_has_deprecated_tiles = true;
          return;
        }
      }
    }
  }
}

void
EditorTileConverter::focus_on_tile(Sector* sector, TileMap* tilemap, int pos)
{
  if (sector == nullptr || tilemap == nullptr)
    return;

  auto editor = Editor::current();
  auto layers_widget = editor->get_layers_widget();

  editor->set_sector(sector);
  layers_widget->set_selected_tilemap(tilemap);

  const int width = tilemap->get_width();
  auto screen_position = Vector(pos % width, pos / width) * 32.f;
  
  sector->get_camera().set_translation_centered(screen_position);
  editor->keep_camera_in_bounds();
}

void
EditorTileConverter::convert_tiles_by_file(const std::string& file)
{
  auto editor = Editor::current();
  auto level = editor->get_project()->get_level();

  std::unordered_map<int, int> tiles;

  try
  {
    IFileStream in(file);
    if (!in.good())
      throw std::runtime_error("Error opening file stream!");

    int a, b;
    std::string delimiter;
    while (in >> a >> delimiter >> b)
    {
      if (delimiter != "->")
        throw std::runtime_error("Expected '->' delimiter!");

      tiles[a] = b;
    }
  }
  catch (std::exception& err)
  {
    log_warning << "Couldn't parse conversion file '" << file << "': " << err.what() << std::endl;
    return;
  }

  for (const auto& sector : level->get_sectors())
  {
    for (auto& tilemap : sector->get_objects_by_type<TileMap>())
    {
      tilemap.save_state();
      // Can't use change_all(), if there's like `1 -> 2`and then
      // `2 -> 3`, it'll do a double replacement
      for (int x = 0; x < tilemap.get_width(); x++)
      {
        for (int y = 0; y < tilemap.get_height(); y++)
        {
          auto tile = tilemap.get_tile_id(x, y);
          try
          {
            tilemap.change(x, y, tiles.at(tile));
          }
          catch (std::out_of_range&)
          {
            // Expected for tiles that don't need to be replaced
          }
        }
      }
      tilemap.check_state();
    }
  }
}
