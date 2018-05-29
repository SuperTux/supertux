//  SuperTux
//  Copyright (C) 2018 christ2go <christian@hagemeier.ch>
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
#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_ADDONWIZARD_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_ADDONWIZARD_HPP

#include "gui/menu.hpp"

class EditorAddonWizard : public Menu {
public:
  EditorAddonWizard(const std::string& dir);

  void menu_action(MenuItem* item) override;

private:
  std::string name, author, license, m_dir, id;

  int vers = 0;

  enum MenuIDs {
    MNID_PACK
  };

  EditorAddonWizard(const EditorAddonWizard&);
  EditorAddonWizard& operator=(const EditorAddonWizard&);
};

#endif
