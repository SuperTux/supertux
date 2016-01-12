//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/text.hpp"

#include "object/text_object.hpp"

namespace scripting {

Text::Text(::TextObject* parent) :
  m_parent(parent)
{
}

Text::~Text()
{
}

void
Text::set_text(const std::string& text)
{
  m_parent->set_text(text);
}

void
Text::set_font(const std::string& fontname)
{
  m_parent->set_font(fontname);
}

void
Text::fade_in(float fadetime)
{
  m_parent->fade_in(fadetime);
}

void
Text::fade_out(float fadetime)
{
  m_parent->fade_out(fadetime);
}

void
Text::set_visible(bool visible)
{
  m_parent->set_visible(visible);
}

void
Text::set_centered(bool centered)
{
  m_parent->set_centered(centered);
}

void
Text::set_pos(float x, float y)
{
  m_parent->set_pos(Vector(x, y));
}

float
Text::get_pos_x()
{
  return m_parent->get_pos_x();
}

float
Text::get_pos_y()
{
  return m_parent->get_pos_y();
}

void
Text::set_anchor_point(int anchor)
{
  m_parent->set_anchor_point(anchor);
}

int
Text::get_anchor_point()
{
  return m_parent->get_anchor_point();
}

} // namespace scripting

/* EOF */
