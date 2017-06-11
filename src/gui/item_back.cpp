//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "gui/item_back.hpp"

#include <stdio.h>

#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

ItemBack::ItemBack(const std::string& text_, int _id) :
  MenuItem(text_, _id)
{
}

void
ItemBack::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  float text_width = Resources::normal_font->get_text_width(text);
  context.draw_text(Resources::normal_font, text,
                    Vector( pos.x + menu_width/2 , pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_CENTER, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());
  context.draw_surface(Resources::back,
                       Vector(pos.x + menu_width/2 + text_width/2  + 16, pos.y - 8),
                       LAYER_GUI);
}

int
ItemBack::get_width() const {
  return Resources::normal_font->get_text_width(text) + 32 + Resources::back->get_width();
}

void
ItemBack::process_action(const MenuAction& action) {
  if (action == MENU_ACTION_HIT) {
    if(MenuManager::instance().current_menu()->on_back_action())
      MenuManager::instance().pop_menu();
  }
}
/* EOF */
