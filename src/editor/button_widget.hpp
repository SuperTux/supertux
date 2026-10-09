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

class ButtonWidget : public Widget
{
public:
  ButtonWidget(SpritePtr sprite, const Vector& pos, std::function<void()> m_sig_click = {}, std::optional<Sizef> sprite_size = std::nullopt);
  ButtonWidget(const std::string& path, const Vector& pos, std::function<void()> callback = {}, std::optional<Sizef> sprite_size = std::nullopt) :
    ButtonWidget(SpriteManager::current()->create(path), std::move(pos), std::move(callback), std::move(sprite_size))
  {
  }


  virtual void draw(DrawingContext &context) override;
  virtual void update(float dt_sec) override;

  virtual void setup() override;
  virtual void on_window_resize() override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;

  void set_position(const Vector& pos);
  void set_sprite(const std::string& path);
  void set_sprite(SpritePtr sprite);

  void set_help_text(const std::string& help_text);
  
  inline void set_disabled(bool disabled) { m_disabled = disabled; }
  inline bool is_disabled() const { return m_disabled; }
  
  inline void set_flat(bool flat) { m_flat = flat; }
  inline bool is_flat() const { return m_flat; }

  inline bool is_hovered() const { return m_hover; }

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
  SpritePtr m_sprite;
  Rectf m_rect;
  bool m_grab;
  bool m_hover;
  bool m_disabled;
  bool m_flat;
  std::function<void()> m_sig_click;
  Vector m_mouse_pos;
  std::string m_help_text;
  std::unique_ptr<MenuListWidget> m_menu_list;
  bool m_show_menu;

private:
  ButtonWidget(const ButtonWidget&) = delete;
  ButtonWidget& operator=(const ButtonWidget&) = delete;
};

class EditorToolbarButtonWidget : public ButtonWidget
{
public:
  EditorToolbarButtonWidget(SpritePtr sprite, const Vector& pos, std::function<void()> m_sig_click = {}, std::optional<Sizef> sprite_size = std::nullopt) :
    ButtonWidget(std::move(sprite), std::move(pos), m_sig_click, std::move(sprite_size)),
    m_tile_mode_visible(true),
    m_object_mode_visible(true),
    m_visible(true)
  {
  }

  EditorToolbarButtonWidget(const std::string& path, const Vector& pos, std::function<void()> callback = {}, std::optional<Sizef> sprite_size = std::nullopt) :
    ButtonWidget(SpriteManager::current()->create(path), std::move(pos), std::move(callback), std::move(sprite_size)),
    m_tile_mode_visible(true),
    m_object_mode_visible(true),
    m_visible(true)
  {
  }
  
  EditorToolbarButtonWidget(const std::string& path, std::function<void()> callback = {}, const std::string& help_text = "", std::optional<Sizef> sprite_size = std::nullopt) :
    ButtonWidget(SpriteManager::current()->create(path), Vector(0,0), std::move(callback), std::move(sprite_size)),
    m_tile_mode_visible(true),
    m_object_mode_visible(true),
    m_visible(true)
  {
    set_help_text(help_text);
  }

  virtual void draw(DrawingContext& context) override
  {
    if (!get_visible())
      return;
    
    ButtonWidget::draw(context);

    if (get_show_menu())
    {
      m_menu_list->draw(context);
    }
  }

  virtual void update(float dt_sec) override
  {
    if (!get_visible())
      return;

    if (get_show_menu())
    {
      m_menu_list->update(dt_sec);
    }

    ButtonWidget::update(dt_sec);
  }

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override
  {
    if (!get_visible())
      return false;

    auto event_handled = false;
    if (get_show_menu())
    {
      event_handled |= m_menu_list->on_mouse_button_up(button);
    }

    event_handled |= ButtonWidget::on_mouse_button_up(button);

    return event_handled;
  }

  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override
  {
    if (!get_visible())
      return false;

    auto event_handled = false;
    if (get_show_menu())
    {
      event_handled |= m_menu_list->on_mouse_button_down(button);
    }

    event_handled |= ButtonWidget::on_mouse_button_down(button);
    return event_handled;
  }

  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override
  {
    if (!get_visible())
      return false;

    if (m_hover)
    {
      m_show_menu = m_menu_list != nullptr && m_menu_list->get_item_count() > 0;
    }
    else if (!m_menu_list->has_mouse_focus())
    {
      m_show_menu = false;
    }

    return ButtonWidget::on_mouse_motion(motion);
  }

  void set_visible_in_tile_mode(bool visible) { m_tile_mode_visible = visible; }
  bool get_visible_in_tile_mode() const { return m_tile_mode_visible; }

  void set_visible_in_object_mode(bool visible) { m_object_mode_visible = visible; }
  bool get_visible_in_object_mode() const { return m_object_mode_visible; } 

  void set_visible(bool visible) { m_visible = visible; }
  bool get_visible() const { return m_visible; }

private:
  bool m_tile_mode_visible;
  bool m_object_mode_visible;
  bool m_visible;

private:
  EditorToolbarButtonWidget(const EditorToolbarButtonWidget&) = delete;
  EditorToolbarButtonWidget& operator=(const EditorToolbarButtonWidget&) = delete;
};
