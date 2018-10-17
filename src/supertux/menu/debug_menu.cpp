//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/debug_menu.hpp"

#include "supertux/debug.hpp"
#include "util/gettext.hpp"

DebugMenu::DebugMenu()
{
  add_label(_("Debug"));
  add_hl();
  add_toggle(-1, _("Show Collision Rects"), &g_debug.show_collision_rects);
  add_toggle(-1, _("Show Worldmap Path"), &g_debug.show_worldmap_path);
  add_toggle(-1, _("Use Bitmap Fonts"),
             []{ return g_debug.get_use_bitmap_fonts(); },
             [](bool value){ g_debug.set_use_bitmap_fonts(value); });
  add_hl();
  add_back(_("Back"));
}

void
DebugMenu::menu_action(MenuItem& item)
{
}

/* EOF */
