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

#ifndef HEADER_SUPERTUX_GUI_ITEM_FILE_HPP
#define HEADER_SUPERTUX_GUI_ITEM_FILE_HPP

#include "gui/menu_item.hpp"

class ItemFile final : public MenuItem
{
public:
  ItemFile(const std::string& text, std::string* filename,
           const std::vector<std::string>& extensions,
           const std::string& basedir,
           bool path_relative_to_basedir,
           int id = -1);

  /** Processes the menu action. */
  virtual void process_action(const MenuAction& action) override;

private:
  std::string* m_filename;
  std::vector<std::string> m_extensions;
  std::string m_basedir;
  bool m_path_relative_to_basedir;

private:
  ItemFile(const ItemFile&) = delete;
  ItemFile& operator=(const ItemFile&) = delete;
};

#endif

/* EOF */
