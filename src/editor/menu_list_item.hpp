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
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

#include <functional>
#include <vector>

class MenuListItem : public MenuListBaseItem
{
private:
  static constexpr Vector m_sprite_offset = {2.f, 2.f};
  static constexpr Vector m_label_offset  = {25.f, 7.5f};

public:
  MenuListItem(const std::string &label, const std::function<void()> onclick_handler,
               const std::string &sprite_path, const Rectf &rect);

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

  bool is_enabled() const;

  bool has_mouse_focus() const;

  Sprite* get_sprite() const { return m_sprite.get(); }

private:
  std::string m_label;
  std::string m_keyboard_shortcut;
  std::function<void()> m_onclick_handler;
  std::function<bool()> m_is_selected_handler;
  std::function<bool()> m_is_enabled_handler;
  SpritePtr m_sprite;
};