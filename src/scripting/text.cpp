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
#include "supertux/sector.hpp"
#include "worldmap/worldmap.hpp"

namespace scripting {

void
Text::set_text(const std::string& text)
{
  SCRIPT_GUARD_VOID;
  object.set_text(text);
}

void
Text::set_font(const std::string& fontname)
{
  SCRIPT_GUARD_VOID;
  object.set_font(fontname);
}

void
Text::fade_in(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.fade_in(fadetime);
}

void
Text::fade_out(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.fade_out(fadetime);
}

void
Text::grow_in(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.grow_in(fadetime);
}

void
Text::grow_out(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.grow_out(fadetime);
}

void
Text::set_visible(bool visible)
{
  SCRIPT_GUARD_VOID;
  object.set_visible(visible);
}

void
Text::set_centered(bool centered)
{
  SCRIPT_GUARD_VOID;
  object.set_centered(centered);
}

void
Text::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_pos(Vector(x, y));
}

float
Text::get_pos_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos().x;
}

float
Text::get_pos_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos().y;
}

void
Text::set_anchor_point(int anchor)
{
  SCRIPT_GUARD_VOID;
  object.set_anchor_point(static_cast<AnchorPoint>(anchor));
}

int
Text::get_anchor_point() const
{
  SCRIPT_GUARD_DEFAULT;
  return static_cast<int>(object.get_anchor_point());
}

void
Text::set_anchor_offset(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_anchor_offset(x, y);
}

void
Text::set_front_fill_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  object.set_front_fill_color(Color(red, green, blue, alpha));
}

void
Text::set_back_fill_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  object.set_back_fill_color(Color(red, green, blue, alpha));
}

void
Text::set_text_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  object.set_text_color(Color(red, green, blue, alpha));
}

void
Text::set_roundness(float roundness)
{
  SCRIPT_GUARD_VOID;
  object.set_roundness(roundness);
}

} // namespace scripting

/* EOF */
