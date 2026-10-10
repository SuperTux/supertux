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
#include "editor/widget.hpp"
#include "math/rectf.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

#include <functional>
#include <vector>

class MenuListItem
{
public:
  MenuListItem(const std::string& label, const std::function<void()> onclick_handler,
               const std::string& sprite_path, const Rectf& rect) :
    m_label(label),
    m_keyboard_shortcut(""),
    m_onclick_handler(onclick_handler),
    m_is_selected_handler(nullptr),
    m_is_enabled_handler(nullptr),
    m_sprite(SpriteManager::current()->create(sprite_path)),
    m_rect(rect)
  {
  }

  MenuListItem(const std::string& label) :
    m_label(label),
    m_keyboard_shortcut(""),
    m_onclick_handler([]{}),
    m_is_selected_handler(nullptr),
    m_is_enabled_handler(nullptr),
    m_sprite(nullptr),
    m_rect(Rectf(0, 0, 0, 0))
  {
  }

  MenuListItem(const std::string& label, const std::function<void()> onclick_handler) :
    m_label(label),
    m_keyboard_shortcut(""),
    m_onclick_handler(onclick_handler),
    m_is_selected_handler(nullptr),
    m_is_enabled_handler(nullptr),
    m_sprite(nullptr),
    m_rect(Rectf(0, 0, 0, 0))
  {
  }

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

  bool is_enabled() const
  {
    if (m_is_enabled_handler == nullptr)
    {
      return true;
    }

    return m_is_enabled_handler();
  }

  Sprite* get_sprite() const { return m_sprite.get(); }

  const Rectf &get_rect() const { return m_rect; }
  inline void set_rect(const Rectf &rect) { m_rect = rect; }

  inline bool has_mouse_focus() const
  {
    auto editor = Editor::current();
    auto mouse_pos = editor->get_event_handling()->get_mouse_pos();
    return m_rect.contains(mouse_pos);
  }

private:
  std::string m_label;
  std::string m_keyboard_shortcut;
  std::function<void()> m_onclick_handler;
  std::function<bool()> m_is_selected_handler;
  std::function<bool()> m_is_enabled_handler;
  SpritePtr m_sprite;
  Rectf m_rect;
};

class MenuListWidget : Widget
{
public:
  MenuListWidget() :
    Widget(),
    m_menu_area(),
    m_menu_items(),
    m_menu_background_color(Color(0.15, 0.15, 0.15)),
    m_menu_border_color(Color(0.25, 0.25, 0.25)),
    m_visible(),
    m_item_size(Sizef(200.f, 25.f)),
    m_menu_offset(),
    m_sprite_offset({2.f, 2.f}),
    m_label_offset({25.f, 7.5f}),
    m_selected_item_idx()
  {
  }

  ~MenuListWidget()
  {
  }

  virtual bool event(const SDL_Event &ev) override { return false; }
  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual void setup() override {}

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent &button) override { return false; }
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent &button) override 
  {
    for(const auto& item : m_menu_items)
    {
      if (item->is_enabled() && item->has_mouse_focus())
      {
        auto item_handler = item->get_onclick_handler();
        if (item_handler != nullptr)
        {
          item_handler();
          return true;
        }
      }
    }
    return false;
  }
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent &motion) override { return has_mouse_focus(); }

  void add_item(std::unique_ptr<MenuListItem> item)
  {
    m_menu_items.push_back(std::move(item));
  }

  size_t get_item_count() const { return m_menu_items.size(); }

  inline void set_position(const Vector& position)
  {
    int i = 0;
    for (const auto &item : m_menu_items)
    {
      auto item_pos = position + Vector(0, i * m_item_size.height);
      auto rect = Rectf(item_pos, m_item_size);
      item->set_rect(rect);

      i++;
    }

    auto menu_size = Vector(m_item_size.width, m_item_size.height * m_menu_items.size()) + m_menu_offset;
    m_menu_area = Rectf(position, position + menu_size);
  }

  inline const Rectf &get_area() const { return m_menu_area; }

  inline bool has_mouse_focus() const
  {
    auto editor = Editor::current();
    auto mouse_pos = editor->get_event_handling()->get_mouse_pos();
    return m_menu_area.contains(mouse_pos);
  }

private:
  Rectf m_menu_area;
  std::vector<std::unique_ptr<MenuListItem>> m_menu_items;
  Color m_menu_background_color;
  Color m_menu_border_color;
  bool m_visible;
  Sizef m_item_size;
  Vector m_menu_offset;
  Vector m_sprite_offset;
  Vector m_label_offset;
  int m_selected_item_idx;
};