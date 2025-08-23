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

#include "util/gettext.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/compositor.hpp"

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
  add_toggle(-1, _("Autotile Mode"), &(g_config->editor_autotile_mode));
  add_toggle(-1, _("Enable Autotile Help"), &(g_config->editor_autotile_help));
  add_toggle(-1, _("Enable Object Undo Tracking"), &(g_config->editor_undo_tracking));
  add_toggle(-1, _("Show Properties Sidebar"), &(g_config->editor_show_properties_sidebar));
  add_toggle(-1, _("Show Toolbar"), &(g_config->editor_show_toolbar_widgets));
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
}
