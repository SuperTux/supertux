//  SuperTux
//  Copyright (C) 2025 Vasco Rodrigues <vasco.a.a.rodrigues@tecnico.ulisboa.pt>
//                2025 Afonso Mateus   <afonso.mateus@tecnico.ulisboa.pt> 
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_EDITOR_TILESUBGROUP_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_EDITOR_TILESUBGROUP_MENU_HPP

#include "gui/menu.hpp"

class EditorTilesubgroupMenu final : public Menu
{
public:
  EditorTilesubgroupMenu(std::string parent_group);
  ~EditorTilesubgroupMenu() override;

  void menu_action(MenuItem& item) override;

private:
  EditorTilesubgroupMenu(const EditorTilesubgroupMenu&) = delete;
  EditorTilesubgroupMenu& operator=(const EditorTilesubgroupMenu&) = delete;
  
};

#endif

/* EOF */
