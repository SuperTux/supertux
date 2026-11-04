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

InterfaceControl::InterfaceControl() :
  m_on_activate_callbacks(),
  m_on_change_callbacks(),
  m_label(),
  m_has_focus(),
  m_rect(),
  m_parent(nullptr)
{
}

void
InterfaceControl::call_on_activate_callbacks() const
{
  for (const auto& on_activate : m_on_activate_callbacks)
    on_activate();
}

void
InterfaceControl::call_on_change_callbacks() const
{
  for (const auto& on_change : m_on_change_callbacks)
    on_change();
}

Color
InterfaceControl::get_widget_color() const
{
  if (m_has_focus)
  {
    return Color(0.75f, 0.75f, 0.7f, 1.f);
  }

  if (m_is_hovered)
  {
    return Color(0.6f, 0.6f, 0.6f, 1.f);
  }

  return Color(0.5f, 0.5f, 0.5f, 1.f);
}