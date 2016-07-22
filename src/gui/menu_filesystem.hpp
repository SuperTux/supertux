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

#ifndef HEADER_SUPERTUX_GUI_MENU_FILESYSTEM_HPP
#define HEADER_SUPERTUX_GUI_MENU_FILESYSTEM_HPP

#include "gui/menu.hpp"

class FileSystemMenu : public Menu
{
public:
  FileSystemMenu(std::string* filename_, const std::vector<std::string>& extensions_);
  ~FileSystemMenu();

  void menu_action(MenuItem* item) override;

private:
  std::string* filename;
  std::string directory;
  std::vector<std::string> extensions;
  std::vector<std::string> directories;
  std::vector<std::string> files;

  void refresh_items();
  bool has_right_suffix(const std::string& file) const;

  FileSystemMenu(const FileSystemMenu&);
  FileSystemMenu& operator=(const FileSystemMenu&);
};

#endif

/* EOF */
