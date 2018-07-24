//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_ADDON_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_ADDON_MENU_HPP

#include "gui/menu.hpp"

class Addon;
class AddonManager;

class AddonMenu : public Menu
{
private:
  enum {
    MNID_CHECK_ONLINE,
    MNID_NOTHING_NEW,
    MNID_LANGPACK_MODE,
    MNID_ADDON_LIST_START = 10
  };

private:
  AddonManager& m_addon_manager;
  std::vector<std::string> m_installed_addons;
  std::vector<std::string> m_repository_addons;
  std::unique_ptr<bool[]> m_addons_enabled;
  bool m_language_pack_mode;
  bool m_auto_install_langpack;

public:
  AddonMenu(bool language_pack_mode = false, bool auto_install_langpack = false);
  ~AddonMenu();

  void refresh() override;
  void menu_action(MenuItem* item) override;
  void check_online();
  void install_addon(const Addon& addon);
  void toggle_addon(const Addon& addon);

private:
  void rebuild_menu();
  bool addon_visible(const Addon& addon) const;

private:
  AddonMenu(const AddonMenu&);
  AddonMenu& operator=(const AddonMenu&);
};

#endif

/* EOF */
