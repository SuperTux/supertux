//  SuperTux
//  Copyright (C) 2025 bruhmoent
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

#ifndef HEADER_SUPERTUX_GUI_MENU_FILE_FILTERS_HPP
#define HEADER_SUPERTUX_GUI_MENU_FILE_FILTERS_HPP

#include "gui/menu.hpp"

#include "gui/menu_filesystem.hpp"

class MenuFileFilters final : public Menu
{
public:
  enum
  {
    MNID_APPLY,
    MNID_RESET,
    MNID_FILTER_TYPE_CHANGED
  };

  MenuFileFilters(FileSystemMenu::MenuParams* params, std::function<void()> on_apply_callback);

  void menu_action(MenuItem& item) override;
  bool on_back_action() override;
  bool is_sensitive() const override { return true; }

private:
  bool apply_changes();
  void reset_to_defaults();

private:
  enum class FilterType
  {
    NONE,
    CUSTOM
  };

private:
  FileSystemMenu::MenuParams* m_original_params;

  std::function<void()> m_on_apply_callback;

  std::string m_filename_filter;
  FilterType m_active_filter_type;
  std::string m_custom_extensions_str;

  const std::string m_initial_filename_filter;
  const std::vector<std::string> m_initial_additional_extensions;
};

#endif

/* EOF */
