//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "gui/item_script_line.hpp"

#include "control/input_manager.hpp"
#include "gui/menu_action.hpp"
#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

ItemScriptLine::ItemScriptLine(std::string* input_, int id_) :
  ItemTextField("", input_, id)
{
}

void
ItemScriptLine::draw(DrawingContext& context, Vector pos, int menu_width, bool active) {
  std::string r_input = *input;
  bool fl = active && (int(real_time*2)%2);
  if ( fl ) {
    r_input += "_";
  }
  context.draw_text(Resources::normal_font, r_input,
                    Vector(pos.x + 16, pos.y - int(Resources::normal_font->get_height()/2)),
                    ALIGN_LEFT, LAYER_GUI, ColorScheme::Menu::field_color);
}

int
ItemScriptLine::get_width() const {
  return Resources::normal_font->get_text_width(*input) + 16 + flickw;
}

void
ItemScriptLine::process_action(MenuAction action) {
  ItemTextField::process_action(action);
  Controller* controller = InputManager::current()->get_controller();
  if (action == MENU_ACTION_HIT && controller->pressed(Controller::MENU_SELECT)) {
    *input += "TODO: Add a new line!";
  }
}

/* EOF */
