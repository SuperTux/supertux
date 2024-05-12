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

class FileSystemMenu final : public Menu
{
  struct MenuParams
  {
    std::string* filename;
    const std::vector<std::string>& extensions;
    const std::vector<std::string>& additional_extensions;
    const std::string& basedir;
    bool path_relative_to_basedir;
    const std::function<void(std::string)> callback = nullptr;
    const std::function<void (MenuItem&)>& item_processor = {};
  };

public:
  FileSystemMenu(MenuParams& params);
  ~FileSystemMenu() override;

  void menu_action(MenuItem& item) override;

private:
  void refresh_items();
  bool has_right_suffix(const std::string& file) const;

private:
  MenuParams& m_params;
  std::string m_directory;
  std::vector<std::string> m_directories;
  std::vector<std::string> m_files;

private:
  FileSystemMenu(const FileSystemMenu&) = delete;
  FileSystemMenu& operator=(const FileSystemMenu&) = delete;
};

#endif

/* EOF */
