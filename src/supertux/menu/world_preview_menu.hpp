//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_WORLD_PREVIEW_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_WORLD_PREVIEW_MENU_HPP

#include "gui/menu.hpp"

#include "supertux/savegame.hpp"
#include "video/surface_ptr.hpp"

/** Represents a menu, which shows world preview screenshots and progress. */
class WorldPreviewMenu : public Menu
{
public:
  WorldPreviewMenu();

  void menu_action(MenuItem& item) override;

protected:
  ItemAction* add_world(const std::string& title, const std::string& folder,
                        SurfacePtr preview = nullptr, Savegame::Progress progress = { -1, -1 });

  SurfacePtr find_preview(const std::string& preview_file, const std::string& basedir);
  void draw_preview_data(DrawingContext& context, const Rectf& preview_rect, const float& alpha) override;

private:
  struct WorldEntry
  {
    const std::string folder;
    const Savegame::Progress progress;
  };

private:
  std::vector<WorldEntry> m_world_entries;

private:
  WorldPreviewMenu(const WorldPreviewMenu&) = delete;
  WorldPreviewMenu& operator=(const WorldPreviewMenu&) = delete;
};

#endif

/* EOF */
