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

#include "interface/control_button.hpp"

#include "math/vector.hpp"
#include "supertux/resources.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

ControlButton::ControlButton(std::string label) :
  m_btn_label(label)
{
}

void
ControlButton::draw(DrawingContext& context)
{
  InterfaceControl::draw(context);

  std::tuple<Color, Color> colors = get_theme_colors();
  Color bg_color, tx_color;
  std::tie(bg_color, tx_color) = colors;

  context.color().draw_filled_rect(m_rect, bg_color, LAYER_GUI);

  context.color().draw_text(m_theme.font,
                            m_btn_label, 
                            Vector((m_rect.get_left() + m_rect.get_right()) / 2,
                                   (m_rect.get_top() + m_rect.get_bottom()) / 2 - Resources::control_font->get_height() / 2),
                            FontAlignment::ALIGN_CENTER,
                            LAYER_GUI,
                            tx_color);
}

bool
ControlButton::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  if (InterfaceControl::on_mouse_button_up(button))
    return true;

  if (button.button != SDL_BUTTON_LEFT || !m_has_focus || !m_hovering)
    return false;

  if (m_on_change)
    (*m_on_change)();

  return true;
}

bool
ControlButton::on_key_up(const SDL_KeyboardEvent& key)
{
  if (!m_has_focus)
    return false;

  if (key.keysym.sym == SDLK_SPACE) {
    if (m_on_change)
      (*m_on_change)();
    m_mouse_down = false;
    return true;
  }

  return false;
}

bool
ControlButton::on_key_down(const SDL_KeyboardEvent& key)
{
  if (!m_has_focus)
    return false;

  if (key.keysym.sym == SDLK_SPACE) {
    m_mouse_down = true;
    return true;
  }

  return false;
}

