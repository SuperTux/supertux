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

#include "supertux/menu/editor_settings.hpp"

#include "math/util.hpp"
#include "gui/menu_item.hpp"
#include "util/gettext.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/compositor.hpp"

enum
{
  MNID_PREFER_FULL_VIEWPORT,
};

EditorSettings::EditorSettings()
{
  add_label(_("Editor Settings"));
  add_hl();

  std::vector<std::string> snap_grid_sizes;
  snap_grid_sizes.push_back(_("tiny tile (4px)"));
  snap_grid_sizes.push_back(_("small tile (8px)"));
  snap_grid_sizes.push_back(_("medium tile (16px)"));
  snap_grid_sizes.push_back(_("big tile (32px)"));

  add_string_select(-1, _("Grid Size"), &(g_config->editor_selected_snap_grid_size), snap_grid_sizes);
  add_toggle(-1, _("Show Grid"), &(g_config->editor_render_grid));
  add_toggle(-1, _("Grid Snapping"), &(g_config->editor_snap_to_grid));
  add_toggle(-1, _("Render Background"), &(g_config->editor_render_background));
  add_toggle(-1, _("Render Light"), &(Compositor::s_render_lighting));
  add_toggle(-1, _("Render Animations"), &(g_config->editor_render_animations));
  add_toggle(-1, _("Invert Shift+Scroll"), &(g_config->editor_invert_shift_scroll));
  add_toggle(-1, _("Centered Zoom"), &(g_config->editor_zoom_centered));
  add_toggle(-1, _("Autotile Mode"), &(g_config->editor_autotile_mode));
  add_toggle(-1, _("Enable Autotile Help"), &(g_config->editor_autotile_help));
  add_toggle(-1, _("Enable Object Undo Tracking"), &(g_config->editor_undo_tracking));
#if 0 // The option exists for those to mess with, but hidden because it's still a WIP
  add_toggle(-1, _("Show Properties Sidebar"), &(g_config->editor_show_properties_sidebar));
#endif
  add_toggle(-1, _("Show Toolbar"), &(g_config->editor_show_toolbar_widgets));
  add_toggle(-1, _("Remember Last Level"), &(g_config->editor_remember_last_level));
  add_toggle(MNID_PREFER_FULL_VIEWPORT, _("Prefer Full Viewport"), &(g_config->editor_max_viewport));
  add_intfield(_("Blur Amount"), &(g_config->editor_blur), -1, true, ItemIntFieldRange{0, 67});
  add_textfield(_("Preferred Text Editor"), &(g_config->preferred_text_editor));
  if (g_config->editor_undo_tracking)
  {
    add_intfield(_("Undo Stack Size"), &(g_config->editor_undo_stack_size), -1, true);
  }
  add_intfield(_("Autosave Frequency"), &(g_config->editor_autosave_frequency));

  add_hl();
  add_back(_("Back"));
}

EditorSettings::~EditorSettings()
{
}

void
EditorSettings::menu_action(MenuItem& item)
{
  switch (item.get_id())
  {
    case MNID_PREFER_FULL_VIEWPORT:
      if (!g_config->max_viewport)
        VideoSystem::current()->get_viewport().force_full_viewport(g_config->editor_max_viewport);

      break;

    default:
      break;
  }
}
