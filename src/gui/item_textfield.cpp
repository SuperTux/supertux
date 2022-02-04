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

#include "gui/item_textfield.hpp"

#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

ItemTextField::ItemTextField(const std::string& text_, std::string* input_, int id_) :
  MenuItem(text_, id_),
  input(input_),
  flickw(static_cast<int>(Resources::normal_font->get_text_width("_")))
{
}

void
ItemTextField::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  std::string r_input = *input;
  bool fl = active && (int(g_real_time*2)%2);
  if ( fl ) {
    r_input += "_";
  }
  context.color().draw_text(Resources::normal_font, r_input,
                            Vector(pos.x + static_cast<float>(menu_width) - 16.0f - static_cast<float>(fl ? 0 : flickw),
                                   pos.y - Resources::normal_font->get_height() / 2.0f),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::field_color);
  context.color().draw_text(Resources::normal_font, get_text(),
                            Vector(pos.x + 16.0f,
                                   pos.y - static_cast<float>(Resources::normal_font->get_height()) / 2.0f),
                            ALIGN_LEFT, LAYER_GUI, active ? g_config->activetextcolor : get_color());
}

int
ItemTextField::get_width() const {
  return static_cast<int>(Resources::normal_font->get_text_width(get_text()) + Resources::normal_font->get_text_width(*input) + 16.0f + static_cast<float>(flickw));
}

void
ItemTextField::event(const SDL_Event& ev) {
  if (ev.type == SDL_TEXTINPUT) {
    *input += ev.text.text;
  }
}

void
ItemTextField::process_action(const MenuAction& action)
{
  if (action == MenuAction::REMOVE) {
    if (input->length()) {
      unsigned char last_char;
      do {
        last_char = *(--input->end());
        input->resize(input->length() - 1);
        if (input->length() == 0) {
          break;
        }
      } while ( (last_char & 128) && !(last_char & 64) );
    } else {
      invalid_remove();
    }
  }
}

/* EOF */
