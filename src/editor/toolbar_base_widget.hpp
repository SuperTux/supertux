//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "editor/widget.hpp"

#include <functional>

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/toolbar_widget.hpp"
#include "editor/menu_list_widget.hpp"
#include "math/rectf.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/resources.hpp"

class EditorToolbarBaseWidget : public Widget
{
public:
  EditorToolbarBaseWidget() :
    m_tile_mode_visible(true),
    m_object_mode_visible(true),
    m_visible(true),
    m_disabled(false),
    m_grab(false),
    m_hover(false),
    m_mouse_pos(),
    m_help_text(),
    m_menu_list(std::make_unique<MenuListWidget>())
  {
  }
  
  EditorToolbarBaseWidget(const std::string& help_text) :
    EditorToolbarBaseWidget()
  {
    set_help_text(help_text);
  }

  virtual void setup() override;
  virtual void on_window_resize() override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent &button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent &button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent &motion) override;

  virtual void draw(DrawingContext &context) override;

  virtual void update(float dt_sec) override;

  void set_position(const Vector& pos);

  void set_visible_in_tile_mode(bool visible) { m_tile_mode_visible = visible; }
  bool get_visible_in_tile_mode() const { return m_tile_mode_visible; }

  void set_visible_in_object_mode(bool visible) { m_object_mode_visible = visible; }
  bool get_visible_in_object_mode() const { return m_object_mode_visible; } 

  void set_visible(bool visible) { m_visible = visible; }
  bool get_visible() const { return m_visible; }

  inline bool is_hovered() const { return m_hover; }
  
  void set_help_text(const std::string& help_text);
  
  inline void set_disabled(bool disabled) { m_disabled = disabled; }
  inline bool is_disabled() const { return m_disabled; }

  inline void set_menu_list(std::unique_ptr<MenuListWidget> menu_list_widget)
  {
    m_menu_list = std::move(menu_list_widget);
  }

  MenuListWidget* get_menu_list() const
  {
    return m_menu_list.get();
  }

  bool get_show_menu() const { return m_show_menu; }

protected:
  bool m_tile_mode_visible;
  bool m_object_mode_visible;
  bool m_visible;
  
  std::string m_help_text;
  std::unique_ptr<MenuListWidget> m_menu_list;
  bool m_show_menu;
  Vector m_mouse_pos;

  Rectf m_rect;
  bool m_grab;
  bool m_hover;
  bool m_disabled;

private:
  EditorToolbarBaseWidget(const EditorToolbarBaseWidget&) = delete;
  EditorToolbarBaseWidget& operator=(const EditorToolbarBaseWidget&) = delete;
};