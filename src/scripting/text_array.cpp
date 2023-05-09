//  SuperTux
//  Copyright (C) 2018 Nir <goproducti@gmail.com>
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

#include "scripting/text_array.hpp"
#include "object/text_array_object.hpp"

#include "object/text_object.hpp"

namespace scripting {

void
TextArray::add_text_duration(const std::string& text, float duration)
{
  SCRIPT_GUARD_VOID;
  object.add_text(text, duration);
}

void
TextArray::add_text(const std::string& text)
{
  SCRIPT_GUARD_VOID;
  object.add_text(text);
}

void
TextArray::clear()
{
  SCRIPT_GUARD_VOID;
  object.clear();
}

void
TextArray::set_fade_transition(bool fade_transition)
{
  SCRIPT_GUARD_VOID;
  object.set_fade_transition(fade_transition);
}

void
TextArray::set_fade_time(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.set_fade_time(fadetime);
}

void
TextArray::set_text_index(int index)
{
  SCRIPT_GUARD_VOID;
  object.set_text_index(index);
}

void
TextArray::next_text()
{
  SCRIPT_GUARD_VOID;
  object.next_text();
}

void
TextArray::prev_text()
{
  SCRIPT_GUARD_VOID;
  object.prev_text();
}

void
TextArray::set_keep_visible(bool keep_visible)
{
  SCRIPT_GUARD_VOID;
  object.set_keep_visible(keep_visible);
}

void
TextArray::set_done(bool done)
{
  SCRIPT_GUARD_VOID;
  object.set_done(done);
}

void
TextArray::set_auto(bool is_auto)
{
  SCRIPT_GUARD_VOID;
  object.set_auto(is_auto);
}

/////////// text api

void
TextArray::set_text(const std::string& text)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_text(text);
  }
  else {
    object.add_text(text);
  }
}

void
TextArray::set_font(const std::string& fontname)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_font(fontname);
  }
}

void
TextArray::fade_in(float fadetime)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.fade_in(fadetime);
  }
}

void
TextArray::fade_out(float fadetime)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.fade_out(fadetime);
  }
}

void
TextArray::grow_in(float fadetime)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.grow_in(fadetime);
  }
}

void
TextArray::grow_out(float fadetime)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.grow_out(fadetime);
  }
}

void
TextArray::set_visible(bool visible)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_visible(visible);
  }
}

void
TextArray::set_centered(bool centered)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_centered(centered);
  }
}

void
TextArray::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_pos(Vector(x, y));
  }
}

float
TextArray::get_pos_x() const
{
  SCRIPT_GUARD_DEFAULT;

  if (auto* textItem = object.get_current_text_item()) {
    return textItem->text_object.get_pos().x;
  } else {
    log_warning << "TextArray position is not set. Assuming (0,0)" << std::endl;
    return 0;
  }
}

float
TextArray::get_pos_y() const
{
  SCRIPT_GUARD_DEFAULT;

  if (auto* textItem = object.get_current_text_item()) {
    return textItem->text_object.get_pos().y;
  } else {
    log_warning << "TextArray position is not set. Assuming (0,0)" << std::endl;
    return 0;
  }
}

void
TextArray::set_anchor_point(int anchor)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_anchor_point(static_cast<AnchorPoint>(anchor));
  }
}

int
TextArray::get_anchor_point() const
{
  SCRIPT_GUARD_DEFAULT;

  if (auto* textItem = object.get_current_text_item()) {
    return textItem->text_object.get_anchor_point();
  } else {
    return -1;
  }
}

void
TextArray::set_anchor_offset(float x, float y)
{
  SCRIPT_GUARD_VOID;

  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_anchor_offset(Vector(x, y));
  }
}

void
TextArray::set_front_fill_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_front_fill_color(Color(red, green, blue, alpha));
  }
}

void
TextArray::set_back_fill_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_back_fill_color(Color(red, green, blue, alpha));
  }
}

void
TextArray::set_text_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_text_color(Color(red, green, blue, alpha));
  }
}

void
TextArray::set_roundness(float roundness)
{
  SCRIPT_GUARD_VOID;
  if (auto* textItem = object.get_current_text_item()) {
    textItem->text_object.set_roundness(roundness);
  }
}

} // namespace scripting

/* EOF */
