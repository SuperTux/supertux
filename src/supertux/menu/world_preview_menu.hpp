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

#include "math/sizef.hpp"
#include "supertux/savegame.hpp"
#include "supertux/timer.hpp"
#include "supertux/world.hpp"
#include "video/surface_ptr.hpp"

/* Represents a menu that should show world preview screenshots and progress. */
class WorldPreviewMenu : public Menu
{
protected:
  static const Sizef s_preview_size;
  static const float s_preview_fade_time;

public:
  WorldPreviewMenu(float center_x_offset = 1, float center_y_offset = 1);

  void menu_action(MenuItem& item) override;

protected:
  SurfacePtr find_preview(const std::string& preview_file, const std::string& basedir);

  void draw_additional(DrawingContext& context) override;

  bool is_valid_index() const;

protected:
  struct WorldEntry
  {
    const bool is_worldmap;
    const std::string folder;
    const SurfacePtr preview;
    const Savegame::Progress progress;
  };

protected:
  std::vector<WorldEntry> m_world_entries;

  int m_last_world_index;
  Timer m_preview_fade_timer;
  bool m_preview_fade_active;
  bool m_preview_fading_out;

private:
  WorldPreviewMenu(const WorldPreviewMenu&) = delete;
  WorldPreviewMenu& operator=(const WorldPreviewMenu&) = delete;
};

#endif

/* EOF */
