//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#include "supertux/menu/editor_converters_menu.hpp"

#include <unordered_map>

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/item_action.hpp"
#include "gui/menu_manager.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_stream.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

EditorConvertersMenu::EditorConvertersMenu() :
  m_converters(),
  m_tile_conversion_file()
{
  try
  {
    auto doc = ReaderDocument::from_file("images/converters/data.stcd");
    auto root = doc.get_root();
    if (root.get_name() != "supertux-converter-data")
      throw std::runtime_error("File is not a 'supertux-converters-data' file.");

    auto iter = root.get_mapping().get_iter();
    while (iter.next())
    {
      if (iter.get_key().empty())
        continue;

      EditorConvertersMenu::Converter converter;

      auto mapping = iter.as_mapping();
      mapping.get("title", converter.title);
      mapping.get("desc", converter.desc);

      m_converters.insert({ iter.get_key(), converter });
    }
  }
  catch (std::exception& err)
  {
    log_warning << "Cannot read converter data from 'images/converters/data.stcd': " << err.what() << std::endl;
  }

  /** Load menu */
  add_label(_("Convert Tiles"));
  add_hl();

  add_file(_("Select Tile Conversion File"), &m_tile_conversion_file, { "sttc" }, "images/converters", false,
           [this](MenuItem& item) {
             auto it = m_converters.find(item.get_text());
             if (it == m_converters.end())
               return;

             item.set_text("\"" + _(it->second.title) + "\"");
             item.set_help(_(it->second.desc));
           });

  add_entry(MNID_CONVERT_TILES, _("Convert Tiles By File"))
    .set_help(_("Convert all tiles in the current level by a file, specified above."));

  add_hl();
  add_back(_("Back"));
}

void
EditorConvertersMenu::menu_action(MenuItem& item)
{
  if (item.get_id() != MNID_CONVERT_TILES)
  {
    log_warning << "Invalid item with ID " << item.get_id() << " pressed." << std::endl;
    return;
  }

  Dialog::show_confirmation(_("This will convert all tiles in the level. Proceed?\n \nNote: This should not be ran more than once on a level.\nCreating a separate copy of the level is highly recommended."),
    [this]() {
      convert_tiles(m_tile_conversion_file);
    });
}

void
EditorConvertersMenu::convert_tiles(const std::string& file)
{
  std::unordered_map<int, int> tiles;

  try
  {
    IFileStream in(file);
    if (!in.good())
    {
      log_warning << "Couldn't open conversion file '" << file << "'." << std::endl;
      return;
    }

    int a, b;
    std::string delimiter;
    while (in >> a >> delimiter >> b)
    {
      if (delimiter != "->")
      {
        log_warning << "Couldn't parse conversion file '" << file << "'." << std::endl;
        return;
      }

      tiles[a] = b;
    }
  }
  catch (std::exception& err)
  {
    log_warning << "Couldn't parse conversion file '" << file << "': " << err.what() << std::endl;
  }

  MenuManager::instance().clear_menu_stack();

  for (const auto& sector : Editor::current()->get_level()->get_sectors())
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

/* EOF */
