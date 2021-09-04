//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/text_object.hpp"

#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

TextObject::TextObject(const std::string& name) :
  GameObject(name),
  ExposedObject<TextObject, scripting::Text>(this),
  m_font(Resources::normal_font),
  m_text(),
  m_wrapped_text(),
  m_fading(0),
  m_fadetime(0),
  m_visible(false),
  m_centered(false),
  m_anchor(ANCHOR_MIDDLE),
  m_pos(0, 0)
{
}

TextObject::~TextObject()
{
}

void
TextObject::set_font(const std::string& name_)
{
  if (name_ == "normal") {
    m_font = Resources::normal_font;
  } else if (name_ == "big") {
    m_font = Resources::big_font;
  } else if (name_ == "small") {
    m_font = Resources::small_font;
  } else {
    log_warning << "Unknown font '" << name_ << "'." << std::endl;
    m_font = Resources::normal_font;
  }

  wrap_text();
}

void
TextObject::wrap_text()
{
  std::string rest;

  // strip all newlines except double ones (markdown'ish)
  char prev_c = ' ';
  for(char& c : m_text) {
    if (c == '\n') {
      if (prev_c == '\n') {
        rest += '\n';
      } else {
        rest += ' ';
      }
    } else {
      rest += c;
    }
  }

  m_wrapped_text.clear();

  do {
    std::string overflow;
    m_wrapped_text += m_font->wrap_to_width(rest, 500, &overflow);
    if (!overflow.empty()) {
      m_wrapped_text += "\n";
    }
    rest = overflow;
  } while (!rest.empty());
}

void
TextObject::set_text(const std::string& text_)
{
  m_text = text_;
  wrap_text();
}

void
TextObject::fade_in(float fadetime_)
{
  m_fadetime = fadetime_;
  m_fading = fadetime_;
}

void
TextObject::fade_out(float fadetime_)
{
  m_fadetime = fadetime_;
  m_fading = -fadetime_;
}

void
TextObject::set_visible(bool visible_)
{
  m_visible = visible_;
  m_fading = 0;
}

void
TextObject::set_centered(bool centered_)
{
  m_centered = centered_;
}

void
TextObject::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));
  if (m_fading > 0) {
    context.set_alpha((m_fadetime - m_fading) / m_fadetime);
  } else if (m_fading < 0) {
    context.set_alpha(-m_fading / m_fadetime);
  } else if (!m_visible) {
    context.pop_transform();
    return;
  }

  float width  = m_font->get_text_width(m_wrapped_text) + 20.0f;
  float height = m_font->get_text_height(m_wrapped_text) + 20.0f;
  Vector spos = m_pos + get_anchor_pos(Rectf(0, 0, static_cast<float>(context.get_width()), static_cast<float>(context.get_height() + (m_anchor == ANCHOR_MIDDLE ? SCREEN_HEIGHT : 0)) - (m_anchor == ANCHOR_MIDDLE ? 340.0f : 0)),
                                       width, height, m_anchor);

  context.color().draw_filled_rect(Rectf(spos, Sizef(width, height)),
                                   Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI+50);
  if (m_centered) {
    context.color().draw_center_text(m_font, m_wrapped_text, spos, LAYER_GUI+60, TextObject::default_color);
  } else {
    context.color().draw_text(m_font, m_wrapped_text, spos + Vector(10, 10), ALIGN_LEFT, LAYER_GUI+60, TextObject::default_color);
  }

  context.pop_transform();
}

void
TextObject::update(float dt_sec)
{
  if (m_fading > 0) {
    m_fading -= dt_sec;
    if (m_fading <= 0) {
      m_fading = 0;
      m_visible = true;
    }
  } else if (m_fading < 0) {
    m_fading += dt_sec;
    if (m_fading >= 0) {
      m_fading = 0;
      m_visible = false;
    }
  }
}

/* EOF */
