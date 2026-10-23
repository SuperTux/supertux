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

#pragma once

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/menu_list_base_item.hpp"
#include "editor/menu_list_separator.hpp"
#include "editor/widget.hpp"
#include "math/rectf.hpp"

#include <functional>
#include <vector>

class MenuListWidget : public Widget
{
public:
  MenuListWidget();
  ~MenuListWidget();

  virtual bool event(const SDL_Event &ev) override
  { 
    bool event_handled = false;
    for (const auto &item : m_menu_items)
    {
      event_handled |= item->event(ev);
    }
    return event_handled;
  }
  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual void setup() override {}

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent &button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent &button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent &motion) override;
  virtual bool on_key_up(const SDL_KeyboardEvent &key) override;
  virtual bool on_key_down(const SDL_KeyboardEvent &key) override;

  void clear_items() { m_menu_items.clear(); }

  void add_item(std::unique_ptr<MenuListBaseItem> item)
  {
    m_menu_items.push_back(std::move(item));
  }

  void add_separator()
  {
    add_item(std::make_unique<MenuListSeparator>());
  }

  const std::vector<std::unique_ptr<MenuListBaseItem>>& get_menu_items()
  {
    return m_menu_items;
  }
  size_t get_item_count() const { return m_menu_items.size(); }

  /**
   * Sets the position of the menu list
   * @param position Position of the menu list
   * @param anchor_bottom If `true`, the menu size will stay fixed at the bottom
   *                      and grow towards the top
   */
  void set_position(const Vector &position, bool anchor_bottom = false);

  const Vector &get_position() const { return m_menu_area.p1(); }

  inline const Rectf &get_area() const { return m_menu_area; }

  bool has_mouse_focus() const;

private:
  Rectf m_menu_area;
  std::vector<std::unique_ptr<MenuListBaseItem>> m_menu_items;
  Color m_menu_background_color;
  Color m_menu_border_color;
  bool m_visible;
  Vector m_menu_offset;
  int m_selected_item_idx;
};