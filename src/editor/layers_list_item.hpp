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
#include "editor/editor_layer.hpp"
#include "editor/menu_list_item.hpp"
#include "editor/menu_list_separator.hpp"
#include "editor/widget.hpp"
#include "interface/control_textbox.hpp"
#include "math/rectf.hpp"

class LayersWidget;

class LayersListItem : public MenuListItem
{
public:
  LayersListItem(EditorLayersWidget* parent, EditorLayer* layer);

  virtual bool has_focus() const override;

  virtual bool event(const SDL_Event &ev) override;

  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent &button) override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override
  {
    if (m_show_layer_name_textbox)
    {
      return m_layer_name_textbox->on_mouse_button_up(button);
    }

    if (m_dragging)
    {
      m_dragging = false;
    }

    return MenuListItem::on_mouse_button_up(button);
  }
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent &motion) override;

  virtual void update(float dt_sec) override
  {
    MenuListItem::update(dt_sec);

    if (m_show_layer_name_textbox)
    {
      m_layer_name_textbox->update(dt_sec);
    }
  };

  virtual void draw(DrawingContext& context) override;

public:
  SurfacePtr get_state_icon() const;

private:
  void draw_visibility_icon(DrawingContext& context);

  bool has_visibility_settings() const;
  void toggle_visibility();

private:
  EditorLayersWidget* m_parent;
  EditorLayer *m_layer;
  Vector m_eye_offset;
  SurfacePtr m_layer_visible_surface;
  SurfacePtr m_layer_hidden_surface;
  SurfacePtr m_layer_transparent_surface;
  std::unique_ptr<ControlTextbox> m_layer_name_textbox;
  bool m_show_layer_name_textbox;
  std::string m_current_layer_name;
  bool m_dragging;
  Vector m_dragging_offset;
};