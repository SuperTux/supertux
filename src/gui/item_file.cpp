//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "gui/item_file.hpp"

#include "gui/menu.hpp"
#include "gui/menu_filesystem.hpp"
#include "gui/menu_manager.hpp"

ItemFile::ItemFile(const std::string& text, std::string* filename_,
                   const std::vector<std::string>& extensions_, int id) :
  MenuItem(text, id),
  filename(filename_),
  extensions(extensions_)
{
}

void
ItemFile::process_action(const MenuAction& action) {
  if (action == MenuAction::HIT) {
    MenuManager::instance().push_menu(std::make_unique<FileSystemMenu>(filename, extensions));
  }
}

/* EOF */
