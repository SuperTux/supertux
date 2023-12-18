//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
//                2023 Vankata453
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

#include "gui/mousecursor.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

// The time for the caret to change visibility (when it flashes).
static const float CONTROL_CURSOR_TIMER = 0.5f;

const float ControlTextbox::TEXT_X_OFFSET = 5.f;
const float ControlTextbox::TEXT_Y_OFFSET = 5.f;

static const size_t UNDO_STACK_SIZE = 10;

ControlTextbox::ControlTextbox(bool multiline) :
  m_validate_string(),
  m_multiline(multiline),
  m_charlist({ {} }),
  m_string(nullptr),
  m_internal_string_backup(""),
  m_previous_caret_backup(),
  m_undo_stack(),
  m_redo_stack(),
  m_cursor_timer(CONTROL_CURSOR_TIMER),
  m_caret(),
  m_secondary_caret(),
  m_shift_pressed(false),
  m_ctrl_pressed(false),
  m_mouse_pressed(),
  m_scrollbar(),
  m_offset(0.f, 0.f),
  m_box_offset(0.f, 0.f),
  m_box_rect(m_rect)
{
  m_scrollbar.reset(new ControlScrollbar(1.f, 1.f, m_offset.y, 35.f));
}

ControlTextbox::~ControlTextbox()
{
  parse_value();
  MouseCursor::current()->set_action(MouseCursorAction::SELECT);
}

void
ControlTextbox::bind_string(std::string* value)
{
  m_string = value;

  if (m_string)
  {
    m_internal_string_backup = *m_string;
    put_text(m_internal_string_backup);
  }
  else
  {
    m_charlist.clear();
    m_charlist.push_back({});
  }

  set_caret_pos(0);
  set_secondary_caret_pos(0);
}

void
ControlTextbox::update(float dt_sec)
{
  m_cursor_timer -= dt_sec;
  if (m_cursor_timer < -CONTROL_CURSOR_TIMER)
    m_cursor_timer = CONTROL_CURSOR_TIMER;

  m_scrollbar->set_total_region(static_cast<int>(m_charlist.size()) *
                                Resources::control_font->get_height() + TEXT_Y_OFFSET);

  // Apparently the stuff bugs from time to time.
  recenter_offset();
}

void
ControlTextbox::on_rect_change()
{
  m_box_rect = Rectf(m_rect.p1() + m_box_offset, m_rect.p2());

  m_scrollbar->set_rect(Rectf(Vector(m_box_rect.get_right() - 5.f, m_box_rect.get_top()),
                              m_box_rect.p2()));
  m_scrollbar->set_covered_region(m_box_rect.get_height());
}

void
ControlTextbox::draw(DrawingContext& context)
{
  InterfaceControl::draw(context);

  context.color().draw_filled_rect(m_box_rect,
                                   m_has_focus ? Color(0.75f, 0.75f, 0.7f, 1.f)
                                               : Color(0.5f, 0.5f, 0.5f, 1.f),
                                   LAYER_GUI);
  if (m_has_focus)
    m_scrollbar->draw(context);

  context.push_transform();
  context.set_viewport(m_box_rect.to_rect());
  context.set_translation(m_offset);

  if (m_caret.pos != m_secondary_caret.pos)
  {
    const bool main_caret_first = (m_caret.pos < m_secondary_caret.pos);
    const Caret& caret1 = (main_caret_first ? m_caret : m_secondary_caret);
    const Caret& caret2 = (main_caret_first ? m_secondary_caret : m_caret);

    for (int i = caret1.line; i <= caret2.line; i++)
    {
      int start = (i == caret1.line ? caret1.line_pos : 0);
      int end = (i == caret2.line ? caret2.line_pos : static_cast<int>(m_charlist[i].size()));

      float lgt1 = Resources::control_font->get_text_width(get_first_chars(i, start));
      float lgt2 = Resources::control_font->get_text_width(get_first_chars(i, end));

      context.color().draw_filled_rect(Rectf(Vector(lgt1 + TEXT_X_OFFSET, i * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                                             Vector(lgt2 + TEXT_X_OFFSET, (i + 1) * Resources::control_font->get_height() + TEXT_Y_OFFSET)),
                                       m_has_focus ? Color(1.f, 1.f, 0.9f, 0.75f)
                                                   : Color(1.f, 1.f, 0.9f, 0.5f),
                                       LAYER_GUI);
    }
  }

  for (int i = 0; i < static_cast<int>(m_charlist.size()); i++)
  {
    context.color().draw_text(Resources::control_font, get_contents(i),
                              Vector(TEXT_X_OFFSET, i * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                              FontAlignment::ALIGN_LEFT, LAYER_GUI,
                              Color::BLACK);
  }

  if (m_cursor_timer > 0 && m_has_focus)
  {
    float lgt = Resources::control_font->get_text_width(get_first_chars(m_caret.line, m_caret.line_pos));

    context.color().draw_line(Vector(lgt + TEXT_X_OFFSET, m_caret.line * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                              Vector(lgt + TEXT_X_OFFSET, (m_caret.line + 1) * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                              Color::BLACK,
                              LAYER_GUI + 1);
  }

  context.pop_transform();
}

bool
ControlTextbox::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  m_scrollbar->on_mouse_button_up(button);

  if (!m_mouse_pressed)
    return false;

  m_mouse_pressed = false;
  return true;
}

bool
ControlTextbox::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (m_scrollbar->on_mouse_button_down(button))
    return true;

  const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(button.x, button.y);
  if (m_rect.contains(mouse_pos))
  {
    m_has_focus = true;
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    set_caret_pos(get_text_position(mouse_pos));
    set_secondary_caret_pos(m_caret.pos);
    m_mouse_pressed = true;
    return true;
  }

  m_has_focus = false;
  return false;
}

bool
ControlTextbox::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  InterfaceControl::on_mouse_motion(motion);

  if (m_scrollbar->on_mouse_motion(motion))
  {
    MouseCursor::current()->set_state(MouseCursorState::NORMAL);
    MouseCursor::current()->set_action(MouseCursorAction::SELECT);
    return true;
  }

  const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  if (m_has_focus && (m_mouse_pressed || m_box_rect.contains(mouse_pos)))
  {
    MouseCursor::current()->set_action(MouseCursorAction::TEXT);
  }
  else
  {
    MouseCursor::current()->set_state(MouseCursorState::NORMAL);
    MouseCursor::current()->set_action(MouseCursorAction::SELECT);
  }

  if (!m_mouse_pressed)
    return false;

  set_caret_pos(get_text_position(mouse_pos));

  recenter_offset(true);
  m_cursor_timer = CONTROL_CURSOR_TIMER;
  return true;
}

bool
ControlTextbox::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (!m_has_focus)
    return false;

  return m_scrollbar->on_mouse_wheel(wheel);
}

bool
ControlTextbox::on_key_up(const SDL_KeyboardEvent& key)
{
  if (!m_has_focus)
    return false;

  if (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_RSHIFT)
  {
    m_shift_pressed = false;
    return true;
  }
  if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
  {
    m_ctrl_pressed = false;
    return true;
  }

  return false;
}

bool
ControlTextbox::on_key_down(const SDL_KeyboardEvent& key)
{
  if (!m_has_focus)
    return false;

  if (key.keysym.sym == SDLK_LEFT)
  {
    if (!m_shift_pressed && m_secondary_caret.pos != m_caret.pos)
    {
      set_caret_pos(std::min(m_secondary_caret.pos, m_caret.pos));
      set_secondary_caret_pos(m_caret.pos);
    }
    else
    {
      set_caret_pos(m_caret.pos - 1);
      m_cursor_timer = CONTROL_CURSOR_TIMER;
      if (!m_shift_pressed)
        set_secondary_caret_pos(m_caret.pos);
    }

    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_RIGHT)
  {
    if (!m_shift_pressed && m_secondary_caret.pos != m_caret.pos)
    {
      set_caret_pos(std::max(m_secondary_caret.pos, m_caret.pos));
      set_secondary_caret_pos(m_caret.pos);
    }
    else
    {
      set_caret_pos(m_caret.pos + 1);
      m_cursor_timer = CONTROL_CURSOR_TIMER;
      if (!m_shift_pressed)
        set_secondary_caret_pos(m_caret.pos);
    }

    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_UP)
  {
    if (!m_shift_pressed && m_secondary_caret.pos != m_caret.pos)
    {
      set_caret_pos(std::min(m_secondary_caret.pos, m_caret.pos));
      set_secondary_caret_pos(m_caret.pos);
    }
    else if (m_caret.line > 0)
    {
      const int previous_line_size = static_cast<int>(m_charlist[m_caret.line - 1].size());
      set_caret_pos(get_line_start(m_caret.line - 1) + (m_caret.line_pos > previous_line_size ? previous_line_size : m_caret.line_pos));

      m_cursor_timer = CONTROL_CURSOR_TIMER;
      if (!m_shift_pressed)
        set_secondary_caret_pos(m_caret.pos);
    }

    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_DOWN)
  {
    if (!m_shift_pressed && m_secondary_caret.pos != m_caret.pos)
    {
      set_caret_pos(std::max(m_secondary_caret.pos, m_caret.pos));
      set_secondary_caret_pos(m_caret.pos);
    }
    else if (m_caret.line < static_cast<int>(m_charlist.size()) - 1)
    {
      const int next_line_size = static_cast<int>(m_charlist[m_caret.line + 1].size());
      set_caret_pos(get_line_start(m_caret.line + 1) + (m_caret.line_pos > next_line_size ? next_line_size : m_caret.line_pos));

      m_cursor_timer = CONTROL_CURSOR_TIMER;
      if (!m_shift_pressed)
        set_secondary_caret_pos(m_caret.pos);
    }

    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_RETURN) // ENTER
  {
    if (!m_multiline)
    {
      m_has_focus = false;
      return true;
    }

    erase_selected_text();

    const int current_line_index = (m_caret.pos < m_secondary_caret.pos ? m_caret.line : m_secondary_caret.line);
    auto& current_line = m_charlist[current_line_index];
    std::list<char> new_line;

    auto it = current_line.begin();
    advance(it, m_caret.line_pos);
    new_line.splice(new_line.end(), current_line, it, current_line.end());

    m_charlist.insert(m_charlist.begin() + (current_line_index + 1), std::move(new_line));

    set_caret_pos(m_caret.pos + 1); // Newline
    set_secondary_caret_pos(m_caret.pos);

    recenter_offset(true);
    parse_value();
    return true;
  }
  else if (key.keysym.sym == SDLK_TAB)
  {
    put_text("  "); // Tab key should insert 2 spaces
    parse_value();
  }
  else if (key.keysym.sym == SDLK_BACKSPACE)
  {
    if (!erase_selected_text())
      delete_char_before_caret();

    parse_value();
    return true;
  }
  else if (key.keysym.sym == SDLK_DELETE)
  {
    if (!erase_selected_text() && m_caret.pos < get_total_character_count())
    {
      set_caret_pos(m_caret.pos + 1);
      delete_char_before_caret();
    }

    parse_value();
    return true;
  }
  else if (key.keysym.sym == SDLK_HOME)
  {
    set_caret_pos(get_line_start(m_caret.line));
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    if (!m_shift_pressed)
      set_secondary_caret_pos(m_caret.pos);

    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_END)
  {
    set_caret_pos(get_line_start(m_caret.line) + static_cast<int>(m_charlist[m_caret.line].size()));
    m_cursor_timer = CONTROL_CURSOR_TIMER;
    if (!m_shift_pressed)
      set_secondary_caret_pos(m_caret.pos);

    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_LSHIFT || key.keysym.sym == SDLK_RSHIFT)
  {
    m_shift_pressed = true;
    return true;
  }
  else if (key.keysym.sym == SDLK_LCTRL || key.keysym.sym == SDLK_RCTRL)
  {
    m_ctrl_pressed = true;
    return true;
  }
  else if (key.keysym.sym == SDLK_c && m_ctrl_pressed)
  {
    copy();
    return true;
  }
  else if (key.keysym.sym == SDLK_v && m_ctrl_pressed)
  {
    paste();
    return true;
  }
  else if (key.keysym.sym == SDLK_a && m_ctrl_pressed)
  {
    set_caret_pos(std::numeric_limits<int>::max());
    set_secondary_caret_pos(0);
    recenter_offset(true);
    return true;
  }
  else if (key.keysym.sym == SDLK_z && m_ctrl_pressed)
  {
    if (m_undo_stack.empty())
      return true;

    const auto& change = m_undo_stack.back();
    revert_value(change.string);
    if (parse_value(true, TextAction::UNDO))
    {
      m_caret = std::move(change.carets.caret);
      m_secondary_caret = std::move(change.carets.secondary_caret);
    }
    m_undo_stack.pop_back();
    return true;
  }
  else if (key.keysym.sym == SDLK_y && m_ctrl_pressed)
  {
    if (m_redo_stack.empty())
      return true;

    const auto& change = m_redo_stack.back();
    revert_value(change.string);
    if (parse_value(true, TextAction::REDO))
    {
      m_caret = std::move(change.carets.caret);
      m_secondary_caret = std::move(change.carets.secondary_caret);
    }
    m_redo_stack.pop_back();
    return true;
  }

  return false;
}

bool
ControlTextbox::event(const SDL_Event& ev)
{
  Widget::event(ev);

  if (ev.type == SDL_TEXTINPUT && m_has_focus)
  {
    put_text(std::string(ev.text.text));
    parse_value();
  }

  return false;
}

bool
ControlTextbox::parse_value(bool call_on_change /* = true  (see header)*/, TextAction action /* = TextAction::NONE  (see header)*/)
{
  const std::string new_str = get_contents();
  if (m_internal_string_backup == new_str)
    return false;

  if (!validate_value())
  {
    revert_value(m_internal_string_backup);
    m_caret = m_previous_caret_backup.caret;
    m_secondary_caret = m_previous_caret_backup.secondary_caret;
    return false;
  }

  if (action == TextAction::UNDO)
  {
    // Update redo stack
    m_redo_stack.push_back({ m_internal_string_backup, std::move(m_previous_caret_backup) });
  }
  else
  {
    // Update undo stack
    m_undo_stack.push_back({ m_internal_string_backup, std::move(m_previous_caret_backup) });
    while (m_undo_stack.size() >= UNDO_STACK_SIZE)
      m_undo_stack.erase(m_undo_stack.begin());

    // Clear redo stack, if not currently performing a redo action
    if (action != TextAction::REDO)
      m_redo_stack.clear();
  }

  m_internal_string_backup = new_str;
  m_previous_caret_backup = { m_caret, m_secondary_caret };

  if (m_string)
    *m_string = new_str;

  if (call_on_change && m_on_change)
    m_on_change();

  return true;
}

bool
ControlTextbox::validate_value()
{
  // Abort if we have a validation function for the string, and the function
  // says the string is invalid.
  return !(m_validate_string && !m_validate_string(this, get_contents()));
}

void
ControlTextbox::revert_value(const std::string& str)
{
  m_charlist.clear();
  m_charlist.push_back({});

  int line = 0;
  for (char c : str)
  {
    if (c == '\n')
    {
      if (!m_multiline)
        continue;

      m_charlist.push_back({});
      line++;
      continue;
    }
    m_charlist[line].push_back(c);
  }

  set_caret_pos(0);
  set_secondary_caret_pos(m_caret.pos);

  recenter_offset(true);
}

std::string
ControlTextbox::get_string() const
{
  return m_internal_string_backup;
}

std::string
ControlTextbox::get_contents(int line) const
{
  std::string out;

  if (line < 0) // No line specified
  {
    for (const auto& line_chars : m_charlist)
    {
      for (char c : line_chars)
        out += c;

      out += '\n';
    }
    out.pop_back(); // Remove excess newline
  }
  else
  {
    for (char c : m_charlist[line])
      out += c;
  }

  return out;
}

std::string
ControlTextbox::get_first_chars(int line, int amount) const
{
  std::string out;

  for (char c : m_charlist[line])
  {
    if (!(amount--)) break;
    out += c;
  }

  return out;
}

int
ControlTextbox::get_text_position(Vector pos) const
{
  // Apply offset
  pos += m_offset;

  // Get line index
  int line = 0;
  while (line < static_cast<int>(m_charlist.size()) - 1 &&
         pos.y > m_box_rect.get_top() + (line + 1) * Resources::control_font->get_height() + TEXT_Y_OFFSET)
    line++;

  const float dist = pos.x - m_box_rect.get_left() + TEXT_X_OFFSET;

  int i = 1; // Count newline character
  float prev_width = 0.f;
  while (i < static_cast<int>(m_charlist[line].size()))
  {
    const float width = Resources::control_font->get_text_width(get_first_chars(line, i + 1));
    if (dist <= width)
    {
      // Go left, if provided position is to the left from the last character's middle
      if (dist < width - (width - prev_width) / 2) i--;
      break;
    }
    prev_width = width;
    i++;
  }

  // Add characters from previous lines to counter
  for (int y = 0; y < line; y++)
    i += static_cast<int>(m_charlist[y].size()) + 1;

  return i;
}

std::string
ControlTextbox::get_truncated_text(std::string text) const
{
  while (!text.empty() && !fits(text))
    text.pop_back();

  return text;
}

bool
ControlTextbox::fits(const std::string& text) const
{
  return Resources::control_font->get_text_width(text) <= m_box_rect.get_width() - TEXT_X_OFFSET * 2;
}

void
ControlTextbox::recenter_offset(bool focus_on_line)
{
  /** Focus the X offset, where the main caret is on the current line. */
  const float char_pos = Resources::control_font->get_text_width(get_first_chars(m_caret.line, m_caret.line_pos));
  if (char_pos - m_offset.x < 0.f)
    m_offset.x = char_pos;
  else if (char_pos - m_offset.x > m_box_rect.get_width() - TEXT_X_OFFSET * 2)
    m_offset.x = char_pos - (m_box_rect.get_width() - TEXT_X_OFFSET * 2);

  /** If allowed, focus the Y offset on the line, where the main caret is. */
  if (focus_on_line)
  {
    float line_pos = m_caret.line * Resources::control_font->get_height() + TEXT_Y_OFFSET;
    if (line_pos - m_offset.y < 0.f)
    {
      m_offset.y = line_pos;
      return;
    }

    line_pos += Resources::control_font->get_height(); // Go to bottom position of line
    if (line_pos - m_offset.y > m_box_rect.get_height())
      m_offset.y = line_pos - m_box_rect.get_height();
  }
}

bool
ControlTextbox::copy() const
{
  if (m_caret.pos == m_secondary_caret.pos)
    return false;

  SDL_ClearError();
  int ret = SDL_SetClipboardText(get_selected_text().c_str());

  if (ret)
    log_warning << "Couldn't copy text to clipboard: " << SDL_GetError() << std::endl;

  return ret == 0;
}

bool
ControlTextbox::paste()
{
  if (!SDL_HasClipboardText())
    return false;

  char* txt = SDL_GetClipboardText();

  if (txt)
  {
    put_text(std::string(txt));
    parse_value();
  }
  else
  {
    log_warning << "Couldn't paste text from clipboard: " << SDL_GetError() << std::endl;
  }

  return txt != nullptr;
}

std::string
ControlTextbox::get_selected_text() const
{
  // There's probably a cleaner way to do this...
  std::string out;
  const int start = std::min(m_caret.pos - m_caret.line, m_secondary_caret.pos - m_secondary_caret.line),
            end = std::max(m_caret.pos - m_caret.line, m_secondary_caret.pos - m_secondary_caret.line);
  int pos = 0;

  for (const auto& line_chars : m_charlist)
  {
    std::string temp;

    for (char c : line_chars)
    {
      if (pos++ < start)
        continue;

      if (pos > end)
        break;

      temp += c;
    }

    if (!temp.empty())
      out += temp + '\n';
  }
  out.pop_back(); // Remove excess newline

  return out;
}

bool
ControlTextbox::put_text(const std::string& text)
{
  const bool has_erased_text = erase_selected_text();

  int line_pos = m_caret.line_pos; // Local variable to avoid setting caret pos for every character
  for (char c : text)
  {
    if (c == '\n')
    {
      if (!m_multiline)
        continue;

      if (m_caret.line == static_cast<int>(m_charlist.size()) - 1)
        m_charlist.push_back({});

      auto& current_line = m_charlist[m_caret.line];
      auto& next_line = m_charlist[m_caret.line + 1];

      auto it = current_line.begin();
      advance(it, line_pos);
      next_line.splice(next_line.begin(), current_line, it, current_line.end());

      set_caret_pos(m_caret.pos + line_pos + 1); // Newline
      line_pos = 0;
      continue;
    }

    auto it = m_charlist[m_caret.line].begin();
    advance(it, line_pos);
    m_charlist[m_caret.line].insert(it, c);

    line_pos++;
  }

  set_caret_pos(m_caret.pos - m_caret.line_pos + line_pos);
  set_secondary_caret_pos(m_caret.pos);
  m_cursor_timer = CONTROL_CURSOR_TIMER;

  recenter_offset(true);

  return has_erased_text;
}

void
ControlTextbox::delete_char_before_caret()
{
  if (m_caret.pos == 0)
    return;

  auto& current_line = m_charlist[m_caret.line];
  if (m_caret.line_pos == 0)
  {
    auto& previous_line = m_charlist[m_caret.line - 1];
    previous_line.splice(previous_line.end(), current_line);
    m_charlist.erase(m_charlist.begin() + m_caret.line);
  }
  else
  {
    auto it = current_line.begin();
    advance(it, m_caret.line_pos - 1);
    current_line.erase(it);
  }

  set_caret_pos(m_caret.pos - 1);
  set_secondary_caret_pos(m_caret.pos);

  recenter_offset(true);
  m_cursor_timer = CONTROL_CURSOR_TIMER;
}

bool
ControlTextbox::erase_selected_text()
{
  if (m_caret.pos == m_secondary_caret.pos)
    return false;

  const int erase_start = std::min(m_caret.pos, m_secondary_caret.pos);
  const int erase_end = std::max(m_caret.pos, m_secondary_caret.pos);

  // Preserve the initial start and end of every line, since they will be edited
  std::vector<std::pair<int, int>> line_limits;
  {
    int next_line_start = 0;
    for (const auto& line_chars : m_charlist)
    {
      line_limits.push_back({ next_line_start, next_line_start + static_cast<int>(line_chars.size()) });
      next_line_start += static_cast<int>(line_chars.size()) + 1;
    }
  }

  for (size_t i = 0; i < m_charlist.size(); i++)
  {
    const int line_start = line_limits[i].first;
    const int line_end = line_limits[i].second;

    if (erase_start < line_start && erase_end > line_end) // Full line is selected
    {
      m_charlist.erase(m_charlist.begin() + i);
      line_limits.erase(line_limits.begin() + i);
      i--;
      continue;
    }

    auto& current_line = m_charlist[i];

    // Check if line is partially selected
    if (erase_start >= line_start && erase_start <= line_end)
    {
      if (erase_end >= line_start && erase_end <= line_end)
      {
        auto it = current_line.begin();
        advance(it, erase_start - line_start);
        auto it2 = current_line.begin();
        advance(it2, erase_end - line_start);
        current_line.erase(it, it2);
      }
      else
      {
        auto it = current_line.begin();
        advance(it, erase_start - line_start);
        current_line.erase(it, current_line.end());
      }
    }
    else if (erase_end >= line_start && erase_end <= line_end)
    {
      auto it = current_line.begin();
      advance(it, erase_end - line_start);

      // Move contents of current line to previous to remove newline
      auto& previous_line = m_charlist[i - 1];
      previous_line.splice(previous_line.end(), current_line, it, current_line.end());
      m_charlist.erase(m_charlist.begin() + i);
    }
  }

  set_caret_pos(erase_start);
  set_secondary_caret_pos(m_caret.pos);

  recenter_offset(true);
  m_cursor_timer = CONTROL_CURSOR_TIMER;
  return true;
}

int
ControlTextbox::get_total_character_count() const
{
  int count = 0;
  for (const auto& line_chars : m_charlist)
    count += static_cast<int>(line_chars.size());

  // Add all newlines
  count += static_cast<int>(m_charlist.size()) - 1;

  return count;
}

int
ControlTextbox::get_line_start(int line) const
{
  int next_line_start = 0;
  for (int i = 0; i < static_cast<int>(m_charlist.size()); i++)
  {
    if (i == line)
      return next_line_start;

    next_line_start += static_cast<int>(m_charlist[i].size()) + 1;
  }

  return 0;
}

void
ControlTextbox::set_caret_pos(int pos)
{
  m_caret.pos = std::max(0, std::min(pos, get_total_character_count()));
  configure_caret(m_caret);

  on_caret_move();
}

void
ControlTextbox::set_secondary_caret_pos(int pos)
{
  m_secondary_caret.pos = std::max(0, std::min(pos, get_total_character_count()));
  configure_caret(m_secondary_caret);

  on_secondary_caret_move();
}

void
ControlTextbox::configure_caret(Caret& caret)
{
  int line_start = 0;
  for (int i = 0; i < static_cast<int>(m_charlist.size()); i++)
  {
    const int line_length = static_cast<int>(m_charlist[i].size()) + 1;
    if (caret.pos < line_start + line_length)
    {
      caret.line = i;
      caret.line_pos = caret.pos - line_start;
      break;
    }
    line_start += line_length;
  }
}

/* EOF */
