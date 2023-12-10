//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#ifndef HEADER_SUPERTUX_INTERFACE_CONTROL_SCRIPTBOX_HPP
#define HEADER_SUPERTUX_INTERFACE_CONTROL_SCRIPTBOX_HPP

#include "interface/control_textbox.hpp"

#include <vector>

#include "interface/control_scrollbar.hpp"
#include "squirrel/autocomplete.hpp"

class ControlScriptbox final : public ControlTextbox
{
public:
  ControlScriptbox();

  void draw(DrawingContext& context) override;
  void update(float dt_sec) override;

  bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;
  bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override;
  bool on_key_down(const SDL_KeyboardEvent& key) override;

protected:
  void on_caret_move() override;

private:
  void update_description();
  void autocomplete();

private:
  squirrel::SuggestionStack m_suggestions;
  Rectf m_suggestions_rect;
  std::unique_ptr<ControlScrollbar> m_suggestions_scrollbar;
  float m_suggestions_offset;

  size_t m_selected_suggestion;
  std::string m_suggestion_description;

private:
  ControlScriptbox(const ControlScriptbox&) = delete;
  ControlScriptbox& operator=(const ControlScriptbox&) = delete;
};

#endif

/* EOF */
