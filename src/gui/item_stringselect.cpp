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

#include "gui/menu_manager.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

ItemStringSelect::ItemStringSelect(const std::string& text, const std::vector<std::string>& list_, int* selected_, int id) :
  MenuItem(text, id),
  list(list_),
  selected(selected_),
  m_callback()
{
}

void
ItemStringSelect::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active) {
  float roff = static_cast<float>(Resources::arrow_left->get_width()) * 1.0f;
  float sel_width = Resources::normal_font->get_text_width(list[*selected]);
  // Draw left side
  context.color().draw_text(Resources::normal_font, get_text(),
                              Vector(pos.x + 16.0f,
                                     pos.y - Resources::normal_font->get_height() / 2.0f),
                              ALIGN_LEFT, LAYER_GUI, active ? g_config->activetextcolor : get_color());

  // Draw right side
  context.color().draw_surface(Resources::arrow_left,
                               Vector(pos.x + static_cast<float>(menu_width) - sel_width - 2.0f * roff - 8.0f,
                                      pos.y - 8.0f),
                               LAYER_GUI);
  context.color().draw_surface(Resources::arrow_right,
                               Vector(pos.x + static_cast<float>(menu_width) - roff - 8.0f,
                                      pos.y - 8.0f),
                               LAYER_GUI);
  context.color().draw_text(Resources::normal_font, list[*selected],
                            Vector(pos.x + static_cast<float>(menu_width) - roff - 8.0f,
                                   pos.y - Resources::normal_font->get_height() / 2.0f),
                            ALIGN_RIGHT, LAYER_GUI, active ? g_config->activetextcolor : get_color());
}

int
ItemStringSelect::get_width() const
{
  float max_width = 0;
  for (auto const& item : list) {
    float const width = Resources::normal_font->get_text_width(item) + 64;
    max_width = std::max(width, max_width);
  }
  return Resources::normal_font->get_text_width(get_text()) + max_width;
}

void
ItemStringSelect::process_action(const MenuAction& action) {
  switch (action) {
    case MenuAction::LEFT:
      if ( (*selected) > 0) {
        (*selected)--;
      } else {
        (*selected) = static_cast<int>(list.size()) - 1;
      }
      MenuManager::instance().current_menu()->menu_action(*this);
      if (m_callback) {
        m_callback(*selected);
      }
      break;
    case MenuAction::RIGHT:
    case MenuAction::HIT:
      if ( (*selected)+1 < int(list.size())) {
        (*selected)++;
      } else {
        (*selected) = 0;
      }
      MenuManager::instance().current_menu()->menu_action(*this);
      if (m_callback) {
        m_callback(*selected);
      }
      break;
    default:
      break;
  }
}

/* EOF */
