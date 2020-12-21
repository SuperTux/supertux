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

#include "interface/control.hpp"

#include "supertux/resources.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

InterfaceControl::InterfaceControl() :
  m_on_change(),
  m_label(),
  m_theme(UITheme(Color(0.5f, 0.5f, 0.5f, 1.f), // background color
                  Color(0.5f, 0.5f, 0.5f, 1.f), // background color on hover
                  Color(0.3f, 0.3f, 0.3f, 1.f), // background color on activation
                  Color(0.75f, 0.75f, 0.7f, 1.f), // background color on focus
                  Color(0.f, 0.f, 0.f, 1.f), // text color
                  Color(0.f, 0.f, 0.f, 1.f), // text color on hover
                  Color(0.f, 0.f, 0.f, 1.f), // text color on activation
                  Color(0.f, 0.f, 0.f, 1.f), // text color on focus
                  Resources::control_font)), // main font
  m_has_focus(),
  m_hovering(),
  m_mouse_down(false),
  m_mouse_pos(),
  m_rect()
{
}

bool
InterfaceControl::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  if (m_label)
    m_label->on_mouse_motion(motion);

  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  m_hovering = m_rect.contains(m_mouse_pos);

  return false;
}

std::tuple<Color, Color>
InterfaceControl::get_theme_colors() const
{
  Color bg_color = m_theme.bg_color;
  Color tx_color = m_theme.tx_color;

  if (m_hovering)
  {
    bg_color = m_theme.bg_hover_color;
    tx_color = m_theme.tx_hover_color;
  }

  if (m_has_focus)
  {
    bg_color = m_theme.bg_focus_color;
    tx_color = m_theme.tx_focus_color;
  }

  if (m_mouse_down)
  {
    bg_color = m_theme.bg_active_color;
    tx_color = m_theme.tx_active_color;
  }

  return std::make_tuple(bg_color, tx_color);
}

bool
InterfaceControl::on_mouse_button_up(const SDL_MouseButtonEvent& event)
{
  m_mouse_down = false;

  return false;
}

bool
InterfaceControl::on_mouse_button_down(const SDL_MouseButtonEvent& event)
{
  m_mouse_down = m_hovering;
  m_has_focus = m_hovering;

  return false;
}
