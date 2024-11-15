//  SuperTux
//  Copyright (C) 2024 bruhmoent
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

#include "color_clipboard.hpp"

ColorClipboard&
ColorClipboard::instance()
{
  static ColorClipboard instance;
  return instance;
}

void
ColorClipboard::set_color(const Color& color)
{
  m_color = std::make_unique<Color>(color);
}

const
Color* ColorClipboard::get_color() const
{
  return m_color.get();
}
