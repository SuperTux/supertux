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

#include "gui/item_stringselect.hpp"

#include <stdio.h>

#include "gui/menu_action.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

ItemStringSelect::ItemStringSelect(const std::string& text_, const std::vector<std::string>& list_, int* selected_, int _id) :
  MenuItem(text_, _id),
  list(list_),
  selected(selected_)
{
}

void
ItemStringSelect::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  float roff = Resources::arrow_left->get_width();
  float sel_width = Resources::normal_font->get_text_width(list[*selected]);
  // Draw left side
  context.draw_text(Resources::normal_font, text,
                    Vector(pos.x + 16, pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_LEFT, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());

  // Draw right side
  context.draw_surface(Resources::arrow_left,
                       Vector(pos.x + menu_width - sel_width - 2*roff - 8, pos.y - 8),
                       LAYER_GUI);
  context.draw_surface(Resources::arrow_right,
                       Vector(pos.x + menu_width - roff - 8, pos.y - 8),
                       LAYER_GUI);
  context.draw_text(Resources::normal_font, list[*selected],
                    Vector(pos.x + menu_width - roff - 8, pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_RIGHT, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());
}

int
ItemStringSelect::get_width() const {
  return Resources::normal_font->get_text_width(text) + Resources::normal_font->get_text_width(list[*selected]) + 64;
}

void
ItemStringSelect::process_action(const MenuAction& action) {
  switch (action) {
    case MENU_ACTION_LEFT:
      if( (*selected) > 0) {
        (*selected)--;
      } else {
        (*selected) = list.size()-1;
      }
      MenuManager::instance().current_menu()->menu_action(this);
      break;
    case MENU_ACTION_RIGHT:
      if( (*selected)+1 < int(list.size())) {
        (*selected)++;
      } else {
        (*selected) = 0;
      }
      MenuManager::instance().current_menu()->menu_action(this);
      break;
    default:
      break;
  }
}

/* EOF */
