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

#include <fmt/format.h>

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/item_action.hpp"
#include "gui/menu_manager.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

EditorConvertersMenu::EditorConvertersMenu() :
  m_tile_conversion_file()
{
  auto tile_converter = Editor::current()->get_tile_converter();
  tile_converter->load_definitions();

  /** Load menu */
  add_label(_("Convert Tiles"));
  add_hl();

  add_file(_("Select Tile Conversion File"), &m_tile_conversion_file, { "sttc" }, "images/converters", false,
           [this, tile_converter](MenuItem& item, const std::string& file_path, bool in_basedir) {
              if (!in_basedir)
               return;

              auto converter_info = tile_converter->get_tile_converter_info_for_file(file_path);
              if (converter_info == nullptr)
                return;

              item.set_text("\"" + converter_info->title + "\"");
              item.set_help(converter_info->description + (converter_info->author.empty() ? "" :
                          "\n\n" + fmt::format(fmt::runtime(_("By: {}")), converter_info->author)));
             });

  add_entry(MNID_CONVERT_TILES, _("Convert Tiles By File"))
    .set_help(_("Convert all tiles in the current level by a file, specified above."));

  add_hl();
  add_back(_("Back"));
}

void
EditorConvertersMenu::menu_action(MenuItem& item)
{
  assert(item.get_id() == MNID_CONVERT_TILES);
  auto tile_converter = Editor::current()->get_tile_converter();

  if (m_tile_conversion_file.empty())
  {
    Dialog::show_message(_("No tile conversion file selected."));
    return;
  }

  Dialog::show_confirmation(_("This will convert all tiles in the level. Proceed?\n\nNote: This should not be ran more than once on a level.\nCreating a separate copy of the level is highly recommended."),
    [this, tile_converter]() {
      tile_converter->convert_tiles_by_file(m_tile_conversion_file);
      MenuManager::instance().clear_menu_stack();
    });
}
