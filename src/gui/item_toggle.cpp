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

#include "gui/item_toggle.hpp"

#include <stdio.h>

#include "gui/menu_action.hpp"
#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

static const float FLICK_CURSOR_TIME = 0.5f;

ItemToggle::ItemToggle(const std::string& text_, bool* toggled_, int _id) :
  MenuItem(text_, _id),
  toggled(toggled_)
{
}

void
ItemToggle::draw(DrawingContext& context, Vector pos, int menu_width, bool active) {
  context.draw_text(Resources::normal_font, text,
                    Vector(pos.x + 16, pos.y - (Resources::normal_font->get_height()/2)),
                    ALIGN_LEFT, LAYER_GUI, active ? ColorScheme::Menu::active_color : get_color());

  if(*toggled) {
    context.draw_surface(Resources::checkbox_checked,
                         Vector(pos.x + menu_width-16 - Resources::checkbox->get_width(), pos.y - 8),
                         LAYER_GUI + 1);
  } else {
    context.draw_surface(Resources::checkbox,
                         Vector(pos.x + menu_width-16 - Resources::checkbox->get_width(), pos.y - 8),
                         LAYER_GUI + 1);
  }
}

int
ItemToggle::get_width() const {
  return Resources::normal_font->get_text_width(text) + 16 + Resources::checkbox->get_width();
}

void
ItemToggle::process_action(MenuAction action) {
  if (action == MENU_ACTION_HIT) {
    *toggled = !(*toggled);
  }
}

/* EOF */
