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
#include "editor/widget.hpp"
#include "math/rectf.hpp"
#include "video/surface.hpp"

#include <functional>
#include <vector>

namespace
{
  static constexpr Vector DEFAULT_SPRITE_OFFSET = {2.f, 2.f};
  static constexpr Vector DEFAULT_LABEL_OFFSET  = {25.f, 7.5f};
  static constexpr Vector DEFAULT_SELECTED_INDICATOR_OFFSET = {5.f, 5.f};
}

class MenuListItem : public MenuListBaseItem
{

public:
  MenuListItem(const std::string &label, const std::function<void()> onclick_handler,
               const std::string &surface_path, const Rectf &rect);

  MenuListItem(const std::string &label);

  MenuListItem(const std::string &label, const std::function<void()> onclick_handler);

  virtual void draw(DrawingContext& context) override;

  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent &button) override;

  const std::string &get_label() const { return m_label; }

  const std::string &get_keyboard_shortcut() const { return m_keyboard_shortcut; }
  void set_keyboard_shortcut(const std::string &shortcut) { m_keyboard_shortcut = shortcut; }

  const std::function<void()>& get_onclick_handler() const { return m_onclick_handler; }
  void set_onclick_handler(const std::function<void()>& onclick_handler)
  {
    m_onclick_handler = onclick_handler;
  }

  const std::function<bool()>& get_is_selected_handler() const { return m_is_selected_handler; }
  void set_is_selected_handler(const std::function<bool()>& is_selected_handler)
  {
    m_is_selected_handler = is_selected_handler;
  }

  const std::function<bool()>& get_is_enabled_handler() const { return m_is_enabled_handler; }
  void set_is_enabled_handler(const std::function<bool()>& is_enabled_handler)
  {
    m_is_enabled_handler = is_enabled_handler;
  }

  /**
   * Returns true if the menu list item is enabled and can be selected
   */
  bool is_enabled() const;

  /**
   * Returns true if the menu list item currently has mouse focus
   */
  bool has_mouse_focus() const;

  /**
   * Returns the current surface that represents the
   * icon of this menu list item
   */
  SurfacePtr get_surface() const { return m_surface; }

  /**
   * Returns the current checkbox surface for the item
   */
  SurfacePtr get_checkbox_surface() const;

  /**
   * Returns the custom checked icon overlay (for widgets that 
   * don't use a checkmark for the currently selected item, e.g. layers widget)
   * TODO: Use better, more descriptive name
   */
  SurfacePtr get_item_selected_surface() const
  {
    return m_item_selected_surface;
  }

  /**
   * Sets the custom checked surface (for widgets that 
   * don't use a checkmark, e.g. layers widget)
   * TODO: Use better, more descriptive name
   * @param surface SurfacePtr instance for a custom item selected surface
   */
  void set_item_selected_surface(SurfacePtr surface)
  {
    m_item_selected_surface = surface;
  }

  const Vector &get_sprite_offset() const { return m_sprite_offset; }
  void set_sprite_offset(const Vector& sprite_offset) { m_sprite_offset = sprite_offset; }

  const Vector &get_label_offset() const { return m_label_offset; }
  void set_label_offset(const Vector& label_offset) { m_label_offset = label_offset; }

  const Vector &get_selected_indicator_offset() const { return m_selected_indicator_offset; }
  void set_selected_indicator_offset(const Vector& selected_indicator_offset) { m_selected_indicator_offset = selected_indicator_offset; }

private:
  std::string m_label;
  std::string m_keyboard_shortcut;
  Vector m_sprite_offset;
  Vector m_label_offset;
  Vector m_selected_indicator_offset;
  std::function<void()> m_onclick_handler;
  std::function<bool()> m_is_selected_handler;
  std::function<bool()> m_is_enabled_handler;
  SurfacePtr m_surface;
  SurfacePtr m_item_selected_surface;
};