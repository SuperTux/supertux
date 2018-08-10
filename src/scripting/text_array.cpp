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

TextArray::TextArray(::TextArrayObject* parent) : m_parent(parent)
{
}

void TextArray::add_text_duration(const std::string& text, float duration)
{
  m_parent->add_text(text, duration);
}

void TextArray::add_text(const std::string& text)
{
  m_parent->add_text(text);
}

void TextArray::clear()
{
  m_parent->clear();
}

void TextArray::set_fade_transition(bool fade_transition)
{
  m_parent->set_fade_transition(fade_transition);
}

void TextArray::set_fade_time(float fadetime)
{
  m_parent->set_fade_time(fadetime);
}

void TextArray::set_text_index(int index_)
{
  m_parent->set_text_index(index_);
}

void TextArray::next_text()
{
  m_parent->next_text();
}

void TextArray::prev_text()
{
  m_parent->prev_text();
}

void TextArray::set_keep_visible(bool keep_visible_)
{
  m_parent->set_keep_visible(keep_visible_);
}

void TextArray::set_done(bool done)
{
  m_parent->set_done(done);
}

void TextArray::set_auto(bool is_auto)
{
  m_parent->set_auto(is_auto);
}

/////////// text api

void TextArray::set_text(const std::string& text)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.set_text(text);
}

void TextArray::set_font(const std::string& fontname)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.set_font(fontname);
}

void TextArray::fade_in(float fadetime)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.fade_in(fadetime);
}

void TextArray::fade_out(float fadetime)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.fade_out(fadetime);
}

void TextArray::set_visible(bool visible)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.set_visible(visible);
}

void TextArray::set_centered(bool centered)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.set_centered(centered);
}

void TextArray::set_pos(float x, float y)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.set_pos(Vector(x, y));
}

float TextArray::get_pos_x() const
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    return textItem->text_object.get_pos_x();

  log_warning << "TextArray position is not set. Assuming (0,0)" << std::endl;
  return 0;
}

float TextArray::get_pos_y() const
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    return textItem->text_object.get_pos_y();

  log_warning << "TextArray position is not set. Assuming (0,0)" << std::endl;
  return 0;
}

void TextArray::set_anchor_point(int anchor)
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    textItem->text_object.set_anchor_point(anchor);
}

int TextArray:: get_anchor_point() const
{
  auto* textItem = m_parent->get_current_text_item();

  if (textItem != nullptr)
    return textItem->text_object.get_anchor_point();
  return -1;
}


}

/* EOF */
