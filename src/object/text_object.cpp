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

#include "editor/editor.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

TextObject::TextObject(const std::string& name) :
  GameObject(name),
  ExposedObject<TextObject, scripting::TextObject>(this),
  m_font(Resources::normal_font),
  m_text(),
  m_wrapped_text(),
  m_fade_progress(0),
  m_fadetime(0),
  m_visible(false),
  m_centered(false),
  m_anchor(ANCHOR_MIDDLE),
  m_anchor_offset(0, 0),
  m_pos(0, 0),
  m_wrap_width(500.f),
  m_front_fill_color(0.6f, 0.7f, 0.8f, 0.5f),
  m_back_fill_color(0.2f, 0.3f, 0.4f, 0.8f),
  m_text_color(1.f, 1.f, 1.f, 1.f),
  m_roundness(16.f),
  m_growing_in(),
  m_growing_out(),
  m_fading_in(),
  m_fading_out(),
  m_grower(),
  m_fader()
{
}

TextObject::~TextObject()
{
}

void
TextObject::set_font(const std::string& name)
{
  if (name == "normal") {
    m_font = Resources::normal_font;
  } else if (name == "big") {
    m_font = Resources::big_font;
  } else if (name == "small") {
    m_font = Resources::small_font;
  } else {
    log_warning << "Unknown font '" << name << "'." << std::endl;
    m_font = Resources::normal_font;
  }

  wrap_text();
}

void
TextObject::wrap_text()
{
  m_wrapped_text.clear();

  std::string rest = m_text;
  do {
    std::string overflow;
    m_wrapped_text += m_font->wrap_to_width(rest, m_wrap_width, &overflow);
    if (!overflow.empty()) {
      m_wrapped_text += "\n";
    }
    rest = overflow;
  } while (!rest.empty());
}

void
TextObject::set_text(const std::string& text)
{
  m_text = text;
  wrap_text();
}

void
TextObject::grow_in(float fadetime)
{
  m_fadetime = fadetime;
  m_visible = true;
  m_fade_progress = 0;
  m_growing_in = true;
  m_grower = true;
}

void
TextObject::grow_out(float fadetime)
{
  m_fadetime = fadetime;
  m_fade_progress = 1;
  m_growing_out = true;
}

void
TextObject::fade_in(float fadetime)
{
  m_fadetime = fadetime;
  m_visible = true;
  m_fade_progress = 0;
  m_fading_in = true;
  m_fader = true;
}

void
TextObject::fade_out(float fadetime)
{
  m_fadetime = fadetime;
  m_fade_progress = 1;
  m_fading_out = true;
}

void
TextObject::set_visible(bool visible)
{
  if (visible)
  {
    fade_in(0);
  }
  else
  {
    fade_out(0);
  }
}

void
TextObject::set_centered(bool centered)
{
  m_centered = centered;
}

void
TextObject::set_front_fill_color(Color frontfill)
{
  m_front_fill_color = frontfill;
}

void
TextObject::set_back_fill_color(Color backfill)
{
  m_back_fill_color = backfill;
}

void
TextObject::set_text_color(Color textcolor)
{
  m_text_color = textcolor;
}

void
TextObject::set_roundness(float roundness)
{
  m_roundness = roundness;
}

void
TextObject::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.transform().scale = 1.f;
  if (m_fader)
    context.set_alpha(m_fade_progress);

  if (!m_visible)
  {
    context.pop_transform();
    return;
  }

  float width  = m_font->get_text_width(m_wrapped_text) + 20.0f;
  float height = m_font->get_text_height(m_wrapped_text) + 20.0f;
  Vector spos = m_pos + get_anchor_pos(context.get_rect(),
                                       width, height, m_anchor) + m_anchor_offset;
  Vector sizepos = spos + (Vector(width / 2.f, height / 2.f)) - (Vector(width / 2.f, height / 2.f) * (m_fade_progress));

  if (m_fade_progress > 0.f)
  {
    const Rectf draw_rect(m_grower ? sizepos : spos, Sizef(width, height) * (m_fader ? 1.f : m_fade_progress));

    context.color().draw_filled_rect(draw_rect.grown(4.0f), m_back_fill_color, m_roundness + 4.f, LAYER_GUI + 50);
    context.color().draw_filled_rect(draw_rect, m_front_fill_color, m_roundness, LAYER_GUI + 50);
  }

  if (m_fader || (m_grower && m_fade_progress >= 1.f))
  {
    context.color().draw_text(m_font, m_wrapped_text,
                              spos + Vector(m_centered ? width / 2.f : 10.f, 10.f),
                              m_centered ? ALIGN_CENTER : ALIGN_LEFT,
                              LAYER_GUI + 60, m_text_color);
  }

  context.pop_transform();
}

void
TextObject::update(float dt_sec)
{
  if ((m_growing_in || m_fading_in) && m_fade_progress < 1.f)
  {
    m_fade_progress += dt_sec / m_fadetime;
    if (m_fade_progress >= 1.f)
    {
      m_fade_progress = 1.f;
      m_visible = true;
      m_growing_in = false;
      m_fading_in = false;
    }
  }
  else if ((m_growing_out || m_fading_out) && m_fade_progress > 0.f)
  {
    m_fade_progress -= dt_sec / m_fadetime;
    if (m_fade_progress <= 0.f)
    {
      m_fade_progress = 0.f;
      m_visible = false;
      m_growing_out = false;
      m_fading_out = false;
      m_grower = false;
      m_fader = false;
    }
  }
}

/* EOF */
