//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "interface/control_checkbox.hpp"

#include "math/vector.hpp"
#include "supertux/resources.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

ControlCheckbox::ControlCheckbox() :
  m_value()
{
}

void
ControlCheckbox::draw(DrawingContext& context)
{
  InterfaceControl::draw(context);

  std::tuple<Color, Color> colors = get_theme_colors();
  Color bg_color, tx_color;
  std::tie(bg_color, tx_color) = colors;

  context.color().draw_filled_rect(m_rect, bg_color, m_theme.radius, LAYER_GUI);
  if (*m_value) {
    context.color().draw_text(m_theme.font,
                              "X", 
                              Vector((m_rect.get_left() + m_rect.get_right()) / 2 + 1.f,
                                     (m_rect.get_top() + m_rect.get_bottom()) / 2 - Resources::control_font->get_height() / 2),
                              FontAlignment::ALIGN_CENTER,
                              LAYER_GUI,
                              tx_color);
  }
}

bool
ControlCheckbox::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  if (InterfaceControl::on_mouse_button_up(button))
    return true;

  if (button.button != SDL_BUTTON_LEFT || !m_has_focus)
    return false;

  *m_value = !*m_value;

  if (m_on_change)
    (*m_on_change)();

  return true;
}
bool
ControlCheckbox::on_key_up(const SDL_KeyboardEvent& key)
{
  if (key.keysym.sym != SDLK_SPACE || !m_has_focus)
    return false;

  *m_value = !*m_value;

  if (m_on_change)
    (*m_on_change)();

  return true;
}

