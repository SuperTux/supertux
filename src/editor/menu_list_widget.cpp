//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "editor/menu_list_widget.hpp"

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

void
MenuListWidget::draw(DrawingContext& context)
{
  context.color().draw_filled_rect(m_menu_area, m_menu_background_color, LAYER_GUI);
  context.color().draw_rect(m_menu_area, m_menu_border_color, LAYER_GUI);
  
  for(const auto& item : m_menu_items)
  {
    if (item->has_mouse_focus())
    {
      context.color().draw_filled_rect(item->get_rect(), Color::BLACK, LAYER_GUI);
    }

    auto item_selected_Handler = item->get_is_selected_handler();
    if (item_selected_Handler != nullptr)
    {
      auto target_rect = Rectf(item->get_rect().p1() + Vector(5, 5), item->get_rect().p1() + Vector(20, 20));
      if (item_selected_Handler())
      {
        context.color().draw_surface_scaled(Resources::checkbox_checked, target_rect, LAYER_GUI);
      }
      else
      {
        context.color().draw_surface_scaled(Resources::checkbox, target_rect, LAYER_GUI);
      }
    }

    // auto sprite = item->get_sprite();
    // if (sprite != nullptr)
    // {
    //   auto sprite_rect = Rectf(pos + m_sprite_offset, sprite->get_size());
    //   sprite->draw(context.color(), sprite_rect.p1(), LAYER_GUI);
    // }
    auto label_position = item->get_rect().p1() + m_label_offset;
    context.color().draw_text(Resources::editor_menu_font, item->get_label(), label_position, FontAlignment::ALIGN_LEFT, LAYER_GUI);

    if (item->get_keyboard_shortcut().size())
    {
      label_position += Vector(item->get_rect().get_width() - 30.f, 0);
      context.color().draw_text(Resources::editor_menu_font, item->get_keyboard_shortcut(), label_position, FontAlignment::ALIGN_RIGHT, LAYER_GUI);
    }
  }
}

void
MenuListWidget::update(float /* dt_sec */)
{
  const auto& mouse_pos = Editor::current()->get_event_handling()->get_mouse_pos();
}