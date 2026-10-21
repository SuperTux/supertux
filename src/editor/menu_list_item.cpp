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

#include "editor/menu_list_item.hpp"

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

MenuListItem::MenuListItem(const std::string& label, const std::function<void()> onclick_handler,
  const std::string& surface_path, const Rectf& rect) :
    MenuListBaseItem(rect),
    m_label(label),
    m_label_offset(DEFAULT_LABEL_OFFSET),
    m_sprite_offset(DEFAULT_SPRITE_OFFSET),
    m_selected_indicator_offset(DEFAULT_SELECTED_INDICATOR_OFFSET),
    m_keyboard_shortcut(""),
    m_onclick_handler(onclick_handler),
    m_is_selected_handler(nullptr),
    m_is_enabled_handler(nullptr),
    m_surface(Surface::from_file(surface_path)),
    m_item_selected_surface(nullptr)
  {
  }

MenuListItem::MenuListItem(const std::string& label) :
  MenuListBaseItem(),
  m_label(label),
  m_label_offset(DEFAULT_LABEL_OFFSET),
  m_sprite_offset(DEFAULT_SPRITE_OFFSET),
  m_selected_indicator_offset(DEFAULT_SELECTED_INDICATOR_OFFSET),
  m_keyboard_shortcut(""),
  m_onclick_handler([]{}),
  m_is_selected_handler(nullptr),
  m_is_enabled_handler(nullptr),
  m_surface(nullptr),
  m_item_selected_surface(nullptr)
{
}

MenuListItem::MenuListItem(const std::string& label, const std::function<void()> onclick_handler) :
  MenuListBaseItem(),
  m_label(label),
  m_label_offset(DEFAULT_LABEL_OFFSET),
  m_sprite_offset(DEFAULT_SPRITE_OFFSET),
  m_selected_indicator_offset(DEFAULT_SELECTED_INDICATOR_OFFSET),
  m_keyboard_shortcut(""),
  m_onclick_handler(onclick_handler),
  m_is_selected_handler(nullptr),
  m_is_enabled_handler(nullptr),
  m_surface(nullptr),
  m_item_selected_surface(nullptr)
{
}

void
MenuListItem::draw(DrawingContext& context)
{
  if (is_enabled() && has_mouse_focus())
  {
    context.color().draw_filled_rect(get_rect(), Color::BLACK, LAYER_GUI);
  }

  auto& rect = get_rect();
  auto origin = rect.p1();

  auto surface = get_surface();

  auto item_selected_surface = get_item_selected_surface();
  auto item_selected_handler = get_is_selected_handler();
  bool draw_selected_indicator = is_enabled() && item_selected_handler != nullptr;

  if (surface != nullptr)
  {
    origin += m_sprite_offset;

    auto surface_size = Sizef(rect.get_height(), rect.get_height()).as_vector() - m_sprite_offset;
    auto surface_rect = Rectf(origin, origin + surface_size);
    context.color().draw_surface_scaled(surface, surface_rect, LAYER_GUI);

    if (item_selected_surface != nullptr && draw_selected_indicator)
    {
      if (item_selected_handler())
      {
        context.color().draw_surface_scaled(item_selected_surface, surface_rect, LAYER_GUI);
      }
    }
    else
    {
      origin += Vector(surface_size.x, 0);
    }
  }

  if (item_selected_surface == nullptr && draw_selected_indicator)
  {
    auto checkbox_origin = origin + m_selected_indicator_offset;
    auto target_rect = Rectf(checkbox_origin, checkbox_origin + Vector(15, 15));
    context.color().draw_surface_scaled(get_checkbox_surface(), target_rect, LAYER_GUI);
  }

  auto label_position = origin + m_label_offset;
  auto text_color = is_enabled() ? Color::WHITE : Color(0.5, 0.5, 0.5);
  context.color().draw_text(Resources::editor_menu_font, get_label(), label_position, FontAlignment::ALIGN_LEFT, LAYER_GUI, text_color);

  if (get_keyboard_shortcut().size())
  {
    label_position += Vector(get_rect().get_width() - 30.f, 0);
    context.color().draw_text(Resources::editor_menu_font, get_keyboard_shortcut(), label_position, FontAlignment::ALIGN_RIGHT, LAYER_GUI);
  }
}

bool
MenuListItem::on_mouse_button_down(const SDL_MouseButtonEvent &button)
{
  if (is_enabled() && has_mouse_focus())
  {
    auto item_handler = get_onclick_handler();
    if (item_handler != nullptr)
    {
      item_handler();
      return true;
    }
  }

  return false;
}

bool
MenuListItem::is_enabled() const
{
  if (m_is_enabled_handler == nullptr)
  {
    return true;
  }

  return m_is_enabled_handler();
}

bool
MenuListItem::has_mouse_focus() const
{
  auto editor = Editor::current();
  auto mouse_pos = editor->get_event_handling()->get_mouse_pos();
  return m_rect.contains(mouse_pos);
}

SurfacePtr
MenuListItem::get_checkbox_surface() const
{
  auto selected_handler = get_is_selected_handler();
  if (selected_handler == nullptr)
    return nullptr;

  return selected_handler() ? Resources::checkbox_checked : Resources::checkbox;
}