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

#include "editor/widget.hpp"

#include <functional>

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/toolbar_base_widget.hpp"
#include "editor/toolbar_widget.hpp"
#include "editor/menu_list_widget.hpp"
#include "interface/control_textbox_float.hpp"
#include "math/rectf.hpp"
#include "object/camera.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/resources.hpp"

class EditorToolbarZoomWidget : public EditorToolbarBaseWidget
{
public:
  EditorToolbarZoomWidget();

  //virtual bool on_mouse_button_up(const SDL_MouseButtonEvent &button) override;
  virtual void update(float dt_sec) override;
  virtual void setup() override;
  virtual void draw(DrawingContext &context) override;
  virtual bool event(const SDL_Event &ev) override
  {
    EditorToolbarBaseWidget::event(ev);
    return m_control_textbox->event(ev);
  }

  bool has_focus() const { return m_control_textbox->has_focus() || m_menu_list->has_mouse_focus(); }

  float get_scale_value() const { return m_control_textbox->get_value(); }
  inline void set_scale_value(float value) { m_control_textbox->set_value(value); }

  virtual void set_position(const Vector &position) override;

private:
  void set_dropdown_values();
  void set_control_callbacks();
  void on_scale_control_value_changed();
  void synchronize_scale_percentage();

protected:
  std::unique_ptr<ControlTextboxFloat> m_control_textbox;
  float *m_scale_ptr;
  float m_scale_percentage;

private:
  EditorToolbarZoomWidget(const EditorToolbarZoomWidget&) = delete;
  EditorToolbarZoomWidget& operator=(const EditorToolbarZoomWidget&) = delete;
};
