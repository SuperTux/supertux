//  SuperTux
//  Copyright (C) 2023 mrkubax10 <mrkubax10@onet.pl>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_INSTALL_ADDON_FROM_FILE_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_INSTALL_ADDON_FROM_FILE_MENU_HPP

#include "gui/menu.hpp"

class AddonMenu;

class InstallAddonFromFileMenu final : public Menu
{
public:
  InstallAddonFromFileMenu(AddonMenu* addon_menu);

  virtual void refresh() override;
  virtual void menu_action(MenuItem& item) override {}
  
  virtual void event(const SDL_Event& event) override;

private:
  AddonMenu* m_addon_menu;

private:
  InstallAddonFromFileMenu(const InstallAddonFromFileMenu&) = delete;
  InstallAddonFromFileMenu& operator=(const InstallAddonFromFileMenu&) = delete;
};

#endif

/* EOF */
