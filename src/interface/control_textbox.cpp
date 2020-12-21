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

#include "interface/control_textbox.hpp"

#include <math.h>

#include "math/vector.hpp"
#include "math/rectf.hpp"
#include "supertux/resources.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

// Shamelessly snatched values from src/gui/menu.cpp
static const float CONTROL_REPEAT_INITIAL = 0.4f;
static const float CONTROL_REPEAT_RATE    = 0.05f;
static const float CONTROL_CURSOR_TIMER   = 0.5f;

ControlTextbox::ControlTextbox() :
  m_validate_string(),
  m_charlist(),
  m_string(nullptr),
  m_internal_string_backup(""),
  m_backspace_remaining(CONTROL_REPEAT_INITIAL),
  m_cursor_timer(CONTROL_CURSOR_TIMER),
  m_caret_pos(),
  m_secondary_caret_pos(),
  m_shift_pressed(false),
  m_mouse_pressed(),
  m_current_offset(0)
{
}

void
ControlTextbox::update(float dt_sec, const Controller& controller)
{
  if (controller.pressed(Control::REMOVE)) {
    on_backspace();
  }

  if (controller.hold(Control::REMOVE)) {
    m_backspace_remaining -= dt_sec;
    while (m_backspace_remaining < 0) {
      on_backspace();
      m_backspace_remaining += CONTROL_REPEAT_RATE;
    }
  } else {
    m_backspace_remaining = CONTROL_REPEAT_INITIAL;
  }

  m_cursor_timer -= dt_sec;
  if (m_cursor_timer < -CONTROL_CURSOR_TIMER) {
    m_cursor_timer = CONTROL_CURSOR_TIMER;
  }

  // Apparently the stuff bugs from time to time
  recenter_offset();
}

void
ControlTextbox::on_backspace()
{
  if (m_charlist.size()) {
    if (m_caret_pos != m_secondary_caret_pos) {

      m_cursor_timer = CONTROL_CURSOR_TIMER;

      auto it = m_charlist.begin();
      advance(it, std::min(m_caret_pos, m_secondary_caret_pos));
      auto it2 = m_charlist.begin();
      advance(it2, std::max(m_caret_pos, m_secondary_caret_pos));
      m_charlist.erase(it, it2);

      m_caret_pos = std::min(m_caret_pos, m_secondary_caret_pos);
      m_secondary_caret_pos = m_caret_pos;

    } else if (m_caret_pos > 0) {
      m_caret_pos--;
      m_secondary_caret_pos = m_caret_pos;
      m_cursor_timer = CONTROL_CURSOR_TIMER;

      auto it = m_charlist.begin();
      advance(it, m_caret_pos);
      m_charlist.erase(it);
    }

    recenter_offset();
  }
}

void
ControlTextbox::draw(DrawingContext& context)
{
  InterfaceControl::draw(context);

  std::tuple<Color, Color> colors = get_theme_colors();
  Color bg_color, tx_color;
  std::tie(bg_color, tx_color) = colors;

  context.color().draw_filled_rect(m_rect, bg_color, LAYER_GUI);

  if (m_caret_pos != m_secondary_caret_pos) {
    float lgt1 = m_theme.font->get_text_width(get_first_chars_visible(std::max(
                                std::min(m_caret_pos, m_secondary_caret_pos) - m_current_offset,
                                0
                                )));

    float lgt2 = m_theme.font->get_text_width(get_first_chars_visible(std::min(
                                std::max(m_caret_pos, m_secondary_caret_pos) - m_current_offset,
                                int(get_contents_visible().size())
                                )));

    context.color().draw_filled_rect(Rectf(m_rect.p1() + Vector(lgt1 + 5.f, 0.f),
                                           m_rect.p1() + Vector(lgt2 + 5.f, m_rect.get_height())
                                           ),
                                     m_has_focus ? Color(1.f, 1.f, .9f, 0.75f)
                                                 : Color(1.f, 1.f, .9f, 0.5f),
                                     LAYER_GUI);
  }

  context.color().draw_text(m_theme.font,
                            get_contents_visible(), 
                            Vector(m_rect.get_left() + 5.f,
                                   (m_rect.get_top() + m_rect.get_bottom()) / 2 -
                                    m_theme.font->get_height() / 2),
                            FontAlignment::ALIGN_LEFT,
                            LAYER_GUI + 1,
                            tx_color);
  if (m_cursor_timer > 0 && m_has_focus) {
    float lgt = m_theme.font
                                ->get_text_width(get_first_chars_visible(m_caret_pos - m_current_offset));

    context.color().draw_line(m_rect.p1() + Vector(lgt + 5.f, 2.f),
                              m_rect.p1() + Vector(lgt + 5.f,
                                  m_theme.font->get_height() + 4.f),
                              tx_color,
                              LAYER_GUI + 1);
  }
}

bool
ControlTextbox::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  // TODO: Check how to call super without messing with the logic

  Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(button.x, button.y);
  if (m_rect.contains(mouse_pos)) {
    m_has_focus = true;
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    m_caret_pos = get_text_position(mouse_pos);
    m_secondary_caret_pos = m_caret_pos;
    m_mouse_pressed = true;
    return true;
  } else {
    if (m_has_focus) {
      parse_value();
    }
    m_has_focus = false;
  }
  return false;
}

bool
ControlTextbox::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  // TODO: Check how to call super without messing with the logic

  if (m_mouse_pressed) {
    m_mouse_pressed = false;
    return true;
  }
  return false;
}

bool
ControlTextbox::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  InterfaceControl::on_mouse_motion(motion);

  Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  if (m_mouse_pressed) {
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    m_caret_pos = get_text_position(mouse_pos);
    return true;
  }
  return false;
}

bool
ControlTextbox::on_key_up(const SDL_KeyboardEvent& key)
{

  if (m_has_focus &&
    (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_RSHIFT)) {

    m_shift_pressed = false;
    return true;

  }

  return false;
}

bool
ControlTextbox::on_key_down(const SDL_KeyboardEvent& key)
{

  if (m_has_focus && key.keysym.sym == SDLK_LEFT
   && m_caret_pos > 0) {

    m_caret_pos--;
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    if (!m_shift_pressed) {
      m_secondary_caret_pos = m_caret_pos;
    }

    recenter_offset();
    return true;

  } else if (m_has_focus && key.keysym.sym == SDLK_RIGHT
   && m_caret_pos < int(m_charlist.size())) {

    m_caret_pos++;
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    if (!m_shift_pressed) {
      m_secondary_caret_pos = m_caret_pos;
    }

    recenter_offset();
    return true;

  } else if (m_has_focus && key.keysym.sym == SDLK_HOME) {

    m_caret_pos = 0;
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    if (!m_shift_pressed) {
      m_secondary_caret_pos = m_caret_pos;
    }

    recenter_offset();
    return true;

  } else if (m_has_focus && key.keysym.sym == SDLK_END) {

    m_caret_pos = int(m_charlist.size());
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    if (!m_shift_pressed) {
      m_secondary_caret_pos = m_caret_pos;
    }

    recenter_offset();
    return true;

  } else if (m_has_focus &&
    (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_RSHIFT)) {

    m_shift_pressed = true;
    return true;

  } else if (m_has_focus && key.keysym.sym == SDLK_RETURN) {

    parse_value();
    return true;

  }

  return false;
}

bool
ControlTextbox::event(const SDL_Event& ev) {
  Widget::event(ev);

  if (ev.type == SDL_TEXTINPUT && m_has_focus) {

    if (m_secondary_caret_pos != m_caret_pos) {
      m_cursor_timer = CONTROL_CURSOR_TIMER;

      auto it = m_charlist.begin();
      advance(it, std::min(m_caret_pos, m_secondary_caret_pos));
      auto it2 = m_charlist.begin();
      advance(it2, std::max(m_caret_pos, m_secondary_caret_pos));
      m_charlist.erase(it, it2);

      m_caret_pos = std::min(m_caret_pos, m_secondary_caret_pos);
      m_secondary_caret_pos = m_caret_pos;
    }

    auto it = m_charlist.begin();
    advance(it, m_caret_pos);
    m_charlist.insert(it, ev.text.text[0]);

    m_caret_pos++;
    m_secondary_caret_pos = m_caret_pos;
    m_cursor_timer = CONTROL_CURSOR_TIMER;

    recenter_offset();
  }

  return false;
}

bool
ControlTextbox::parse_value(bool call_on_change /* = true  (see header)*/)
{
  // Abort if we have a validation function for the string, and the function
  // says the string is invalid.
  if (m_validate_string) {
    if (!m_validate_string(this, get_contents())) {
      revert_value();
      return false;
    }
  }

  std::string new_str = get_string();
  if (m_internal_string_backup != new_str) {
    m_internal_string_backup = new_str;

    if (m_string)
      *m_string = new_str;

    if (call_on_change && m_on_change)
      (*m_on_change)();
  }

  return true;
}

void
ControlTextbox::revert_value()
{
  std::string str = m_internal_string_backup;

  m_charlist.clear();
  for (char c : str) {
    m_charlist.push_back(c);
  }

  m_caret_pos = 0;
  m_secondary_caret_pos = 0;

  recenter_offset();
}

std::string
ControlTextbox::get_string() const
{
  return m_internal_string_backup;
}

std::string
ControlTextbox::get_contents() const
{
  std::string temp;

  for (char c : m_charlist) {
    temp += c;
  }
  temp += '\0';

  return temp;
}

std::string
ControlTextbox::get_first_chars(int amount) const
{
  std::string temp;

  for (char c : m_charlist) {
    if (!(amount--)) break;
    temp += c;
  }
  temp += '\0';

  return temp;
}

std::string
ControlTextbox::get_contents_visible() const
{
  std::string temp;
  int remaining = m_current_offset;

  for (char c : m_charlist) {
    if (--remaining < 0) {
      temp += c;
    }
  }
  temp += '\0';

  return get_truncated_text(temp);
}

std::string
ControlTextbox::get_first_chars_visible(int amount) const
{
  return get_contents_visible().substr(0, amount);
}

int
ControlTextbox::get_text_position(Vector pos) const
{
  float dist = pos.x - m_rect.get_left();
  int i = 0;
  
  while (m_theme.font->get_text_width(get_first_chars_visible(i)) < dist
         && i <= int(m_charlist.size()))
    i++;

  return std::max(i - 1 + m_current_offset, 0);
}

std::string
ControlTextbox::get_truncated_text(std::string text) const
{
  if (fits(text)) return text;

  std::string temp = text;
  while (!temp.empty() && !fits(temp))
    temp.pop_back();

  return temp;
}

bool
ControlTextbox::fits(std::string text) const
{
  return m_theme.font->get_text_width(text) <= m_rect.get_width() - 10.f;
}

void
ControlTextbox::recenter_offset()
{
  while (m_caret_pos < m_current_offset && m_current_offset > 0) {
    m_current_offset--;
  }

  while (m_caret_pos > m_current_offset + int(get_contents_visible().size()) && m_current_offset < int(get_contents().size())) {
    m_current_offset++;
  }

  while (m_current_offset > 0 && fits(get_contents().substr(m_current_offset - 1))) {
    m_current_offset--;
  }
}

/* EOF */
