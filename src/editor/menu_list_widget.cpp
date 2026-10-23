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

MenuListWidget::MenuListWidget() :
  Widget(),
  m_menu_area(),
  m_menu_items(),
  m_menu_background_color(Color(0.15, 0.15, 0.15)),
  m_menu_border_color(Color(0.25, 0.25, 0.25)),
  m_visible(),
  m_menu_offset(),
  m_selected_item_idx()
{
}

MenuListWidget::~MenuListWidget()
{
}

void
MenuListWidget::draw(DrawingContext& context)
{
  context.color().draw_filled_rect(m_menu_area, m_menu_background_color, LAYER_GUI);
  context.color().draw_rect(m_menu_area, m_menu_border_color, LAYER_GUI);
  
  for(const auto& item : m_menu_items)
  {
    item->draw(context);
  }
}

void
MenuListWidget::update(float /* dt_sec */)
{
}

bool
MenuListWidget::on_mouse_button_up(const SDL_MouseButtonEvent &button)
{
  bool event_handled = false;
  for (const auto &item : m_menu_items)
  {
    event_handled |= item->on_mouse_button_up(button);
  }
  return event_handled;
}

bool
MenuListWidget::on_mouse_button_down(const SDL_MouseButtonEvent &button) 
{
  bool event_handled = false;
  for (const auto &item : m_menu_items)
  {
    event_handled |= item->on_mouse_button_down(button);
  }
  return event_handled;
}

bool
MenuListWidget::on_mouse_motion(const SDL_MouseMotionEvent &motion)
{
  for (const auto &item : m_menu_items)
  {
    item->on_mouse_motion(motion);
  }
  return has_mouse_focus();
}

bool
MenuListWidget::on_key_up(const SDL_KeyboardEvent &key)
{
  bool event_handled = false;
  for (const auto &item : m_menu_items)
  {
    event_handled |= item->on_key_up(key);
  }
  return event_handled;
}

bool
MenuListWidget::on_key_down(const SDL_KeyboardEvent &key)
{
  bool event_handled = false;
  for (const auto &item : m_menu_items)
  {
    event_handled |= item->on_key_down(key);
  }
  return event_handled;
}

void
MenuListWidget::set_position(const Vector& position, bool anchor_bottom)
{
  int i = 0;
  float max_item_width;
  float total_menu_height;
  auto menu_position = position;

  for (const auto &item : m_menu_items)
  {
    auto item_pos = menu_position + Vector(0, total_menu_height);
    auto item_size = item->get_size();

    if (item_size.width > max_item_width)
      max_item_width = item_size.width;

    total_menu_height += item_size.height;
    auto rect = Rectf(item_pos, item_size);
    item->set_rect(rect);

    i++;
  }

  auto menu_size = Vector(max_item_width, total_menu_height) + m_menu_offset;

  if (anchor_bottom)
  {
    auto offset = Vector(0, -total_menu_height);
    for (auto &item : m_menu_items)
    {
      auto& rect = item->get_rect();
      item->set_rect(rect.moved(offset));
    }
    menu_position += offset;
  }

  m_menu_area = Rectf(menu_position, menu_position + menu_size);
}

bool
MenuListWidget::has_mouse_focus() const
{
  auto editor = Editor::current();
  auto mouse_pos = editor->get_event_handling()->get_mouse_pos();
  return m_menu_area.contains(mouse_pos);
}