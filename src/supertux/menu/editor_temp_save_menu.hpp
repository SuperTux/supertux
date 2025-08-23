//  SuperTux
//  Copyright (C) 2025 Hyland B. <me@ow.swag.toys>
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

#pragma once

#include "gui/menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"

class EditorTempSaveMenu final : public EditorLevelsetSelectMenu
{
public:
  EditorTempSaveMenu();
  ~EditorTempSaveMenu() override;

  void menu_action(MenuItem& item) override;
  void initialize();

private:
  EditorTempSaveMenu(const EditorTempSaveMenu&) = delete;
  EditorTempSaveMenu& operator=(const EditorTempSaveMenu&) = delete;
};
