//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
//                2022 Vankata453
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

#include <boost/algorithm/string.hpp>

#include "control/input_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_script.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/console.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

ItemScriptLine::ItemScriptLine(std::string* input_, int id_) :
  ItemTextField("", input_, id_)
{
}

void
ItemScriptLine::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active)
{
  const int index = active ? static_cast<int>(input->size()) - m_cursor_left_offset : -1;
  const std::string input_part_1 = active ? input->substr(0, index) : *input;
  const std::string input_part_2 = active ? input->substr(index) : "";
  const float input_part_1_width = Resources::console_font->get_text_width(input_part_1);
  context.color().draw_text(Resources::console_font, input_part_1,
                            Vector(pos.x + 16.0f,
                                   pos.y - Resources::console_font->get_height() / 2.0f),
                            ALIGN_LEFT, LAYER_GUI, ColorScheme::Menu::field_color);
  if (active && ((int(g_real_time * 2) % 2) || (m_cursor_left_offset != 0 && m_cursor_left_offset != static_cast<int>(input->size()))))
  {
    // Draw text cursor.
    context.color().draw_text(Resources::console_font, m_cursor_char_str,
                              Vector(pos.x + 15.0f + input_part_1_width,
                                     pos.y - Resources::console_font->get_height() / 2.0f),
                              ALIGN_LEFT, LAYER_GUI, Color::CYAN);
  }
  context.color().draw_text(Resources::console_font, input_part_2,
                            Vector(pos.x + 9.5f + input_part_1_width + m_cursor_char_width,
                                   pos.y - Resources::console_font->get_height() / 2.0f),
                            ALIGN_LEFT, LAYER_GUI, ColorScheme::Menu::field_color);
}

int
ItemScriptLine::get_width() const
{
  return static_cast<int>(Resources::console_font->get_text_width(*input) + 16.0f + m_cursor_char_width);
}

bool
ItemScriptLine::custom_event(const SDL_Event& ev)
{
  if (ev.type == SDL_KEYDOWN)
  {
    if (SDL_GetModState() & KMOD_CTRL) // Commands which require CTRL
    {
      if (ev.key.keysym.sym == SDLK_v) // Paste (multi-line support)
      {
        m_input_undo = *input;
        m_input_redo.clear();

        std::vector<std::string> paste_lines;
        char* clipboard_content = SDL_GetClipboardText();
        std::string clipboard_content_str(clipboard_content);
        boost::split(paste_lines, clipboard_content_str, boost::is_any_of("\n"));
        SDL_free(clipboard_content);

        if (paste_lines.empty()) return true;
        *input = input->substr(0, input->size() - m_cursor_left_offset) + paste_lines[0] +
          input->substr(input->size() - m_cursor_left_offset);
        for (std::size_t i = 1; i < paste_lines.size(); i++)
        {
          auto menu = dynamic_cast<ScriptMenu*>(MenuManager::instance().current_menu());
          if (!menu) break;
          menu->add_line()->change_input(paste_lines[i]);
        }
        return false;
      }
      else if (ev.key.keysym.sym == SDLK_d) // Duplicate line
      {
        auto menu = dynamic_cast<ScriptMenu*>(MenuManager::instance().current_menu());
        if (!menu) return true;
        menu->add_line()->change_input(*input);
        return false;
      }
    }
  }
  return true;
}

void
ItemScriptLine::process_action(const MenuAction& action)
{
  ItemTextField::process_action(action);
  const Controller& controller = InputManager::current()->get_controller();
  if (action == MenuAction::HIT && controller.pressed(Control::MENU_SELECT))
  {
    auto menu = dynamic_cast<ScriptMenu*>(MenuManager::instance().current_menu());
    if (!menu) return;
    menu->add_line();
  }
}

void
ItemScriptLine::invalid_remove()
{
  auto menu = dynamic_cast<ScriptMenu*>(MenuManager::instance().current_menu());
  if (!menu) return;
  menu->remove_line();
}

/* EOF */
