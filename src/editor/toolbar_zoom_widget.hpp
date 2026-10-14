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
  virtual void draw(DrawingContext &context) override;
  virtual bool event(const SDL_Event &ev) override
  {
    return m_control_textbox->event(ev);
  }
  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override 
  {
    return m_control_textbox->on_mouse_button_up(button);
  }
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override
  {
    return m_control_textbox->on_mouse_button_down(button);
  }
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override
  {
    return m_control_textbox->on_mouse_motion(motion);
  }
  virtual bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override
  { 
    return m_control_textbox->on_mouse_wheel(wheel);
  }
  virtual bool on_key_up(const SDL_KeyboardEvent& key) override
  {
    return m_control_textbox->on_key_up(key);
  }
  virtual bool on_key_down(const SDL_KeyboardEvent& key) override
  { 
    return m_control_textbox->on_key_down(key);
  }

  virtual void set_position(const Vector &position) override;

protected:
  std::unique_ptr<ControlTextboxFloat> m_control_textbox;

private:
  EditorToolbarZoomWidget(const EditorToolbarZoomWidget&) = delete;
  EditorToolbarZoomWidget& operator=(const EditorToolbarZoomWidget&) = delete;
};
