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

InterfaceControl::InterfaceControl() :
  m_on_change(),
  m_label(),
  m_theme(UITheme(Color(0.5f, 0.5f, 0.5f, 1.f), // background color
                  Color(0.75f, 0.75f, 0.7f, 1.f), // background color on focus
                  Color(0.f, 0.f, 0.f, 1.f), // text color
                  Resources::control_font)), // main font
  m_has_focus(),
  m_rect()
{
}

