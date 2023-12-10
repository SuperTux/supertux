//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#include "interface/control_scriptbox.hpp"

#include "supertux/resources.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

static const float LINE_NUMBER_RECT_WIDTH = 20.f;
static const float SUGGESTION_RECT_HEIGHT = 100.f;

ControlScriptbox::ControlScriptbox() :
  ControlTextbox(true),
  m_suggestions(),
  m_suggestions_rect(),
  m_suggestions_scrollbar(),
  m_suggestions_offset(0.f),
  m_selected_suggestion(0),
  m_suggestion_description()
{
  m_suggestions_scrollbar.reset(new ControlScrollbar(1.f, SUGGESTION_RECT_HEIGHT, m_suggestions_offset, 35.f));

  // Make space for line number rectangle
  m_box_offset.x += LINE_NUMBER_RECT_WIDTH;
}

void
ControlScriptbox::draw(DrawingContext& context)
{
  ControlTextbox::draw(context);

  /** Line numbers */
  const Rectf line_number_rect(m_rect.p1(),
                               m_rect.p1() + Vector(LINE_NUMBER_RECT_WIDTH, m_rect.get_height()));
  context.color().draw_filled_rect(line_number_rect,
                                   m_has_focus ? Color(0.8f, 0.8f, 0.8f, 1.0f)
                                               : Color(0.6f, 0.6f, 0.6f, 1.f),
                                   LAYER_GUI);

  context.push_transform();
  context.set_viewport(line_number_rect.to_rect());
  context.set_translation(Vector(0.f, m_offset.y));

  for (int i = 0; i < static_cast<int>(m_charlist.size()); i++)
  {
    context.color().draw_text(Resources::control_font, std::to_string(i + 1),
                              Vector(LINE_NUMBER_RECT_WIDTH / 2.f, i * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                              ALIGN_CENTER, LAYER_GUI, Color::BLACK);
  }

  context.pop_transform();

  /** Suggestions */
  if (!m_has_focus || m_suggestions.empty())
    return;

  context.color().draw_filled_rect(m_suggestions_rect, Color(0.9f, 0.9f, 0.9f, 1.f), LAYER_GUI);
  m_suggestions_scrollbar->draw(context);

  context.push_transform();
  context.set_viewport(m_suggestions_rect.to_rect());
  context.set_translation(Vector(0.f, m_suggestions_offset));

  int i = 0;
  for (auto suggestion = m_suggestions.begin(); suggestion != m_suggestions.end(); suggestion++)
  {
    context.color().draw_text(Resources::control_font, suggestion->first,
                              Vector(TEXT_X_OFFSET, i * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                              ALIGN_LEFT, LAYER_GUI + 1, Color::BLACK);

    if (i == static_cast<int>(m_selected_suggestion))
    {
      context.color().draw_filled_rect(Rectf(Vector(0.f, i * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                                             Sizef(context.get_width(), Resources::control_font->get_height())),
                                       Color::WHITE, LAYER_GUI);
    }

    i++;
  }

  context.pop_transform();

  if (!m_suggestion_description.empty())
  {
    const Rectf desc_rect(Vector(m_suggestions_rect.get_right(), m_suggestions_rect.get_top()
                                  + static_cast<float>(m_selected_suggestion) * Resources::control_font->get_height() + TEXT_Y_OFFSET
                                  - m_suggestions_offset),
                          Sizef(Resources::control_font->get_text_width(m_suggestion_description) + 30.f,
                                Resources::control_font->get_text_height(m_suggestion_description) + 30.f));

    context.color().draw_filled_rect(desc_rect, Color(0.9f, 0.9f, 0.9f, 1.f), LAYER_GUI);
    context.color().draw_text(Resources::control_font, m_suggestion_description,
                              desc_rect.p1() + 15.f, ALIGN_LEFT, LAYER_GUI + 1, Color::BLACK);
  }
}

void
ControlScriptbox::update(float dt_sec)
{
  ControlTextbox::update(dt_sec);

  // Do not show autocomplete suggestions when selecting
  if (m_caret.pos != m_secondary_caret.pos)
    m_suggestions.clear();

  m_suggestions_scrollbar->set_total_region(static_cast<int>(m_suggestions.size()) * Resources::control_font->get_height() + TEXT_Y_OFFSET);
}

bool
ControlScriptbox::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  if (!m_suggestions.empty() && m_suggestions_scrollbar->on_mouse_button_up(button))
    return true;

  return ControlTextbox::on_mouse_button_up(button);
}

bool
ControlScriptbox::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (m_suggestions.empty())
    return ControlTextbox::on_mouse_button_down(button);

  if (m_suggestions_scrollbar->on_mouse_button_down(button))
    return true;

  const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(button.x, button.y);
  if (!m_suggestions_rect.contains(mouse_pos))
    return ControlTextbox::on_mouse_button_down(button);

  autocomplete();
  return true;
}

bool
ControlScriptbox::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  if (m_suggestions.empty())
    return ControlTextbox::on_mouse_motion(motion);

  if (m_suggestions_scrollbar->on_mouse_motion(motion))
    return true;

  if (ControlTextbox::on_mouse_motion(motion))
    return true;

  const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  if (!m_suggestions_rect.contains(mouse_pos))
    return false;

  m_selected_suggestion = std::min(static_cast<size_t>((mouse_pos.y - m_suggestions_rect.get_top() + m_suggestions_offset) /
                                                       Resources::control_font->get_height()),
                                   m_suggestions.size() - 1);
  update_description();
  return true;
}

bool
ControlScriptbox::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (!m_has_focus)
    return false;

  if (m_suggestions.empty())
    return ControlTextbox::on_mouse_wheel(wheel);

  return m_suggestions_scrollbar->on_mouse_wheel(wheel);
}

bool
ControlScriptbox::on_key_down(const SDL_KeyboardEvent& key)
{
  if (m_suggestions.empty())
    return ControlTextbox::on_key_down(key);

  switch (key.keysym.sym)
  {
    case SDLK_UP:
    {
      if (m_selected_suggestion > 0)
      {
        m_selected_suggestion--;
        update_description();
        return true;
      }
      else
      {
        m_suggestions.clear();
      }
      break;
    }

    case SDLK_DOWN:
    {
      if (m_selected_suggestion < m_suggestions.size() - 1)
      {
        m_selected_suggestion++;
        update_description();
        return true;
      }
      else
      {
        m_suggestions.clear();
      }
      break;
    }

    case SDLK_TAB:
    case SDLK_RETURN: // ENTER
    {
      autocomplete();
      return true;
    }
  }

  return ControlTextbox::on_key_down(key);
}

void
ControlScriptbox::update_description()
{
  m_suggestion_description.clear();
  if (m_suggestions.empty())
    return;

  auto it = m_suggestions.begin();
  advance(it, m_selected_suggestion);
  const squirrel::ScriptingObject* object = it->second;
  if (!object) return;

  using namespace squirrel;

  std::stringstream out;
  switch (object->get_type())
  {
    case ScriptingObject::Type::CONSTANT:
    {
      const auto& con = static_cast<const ScriptingConstant&>(*object);

      out << con.type << " " << con.name << "\n \n" << con.description;
    }
    break;

    case ScriptingObject::Type::FUNCTION:
    {
      const auto& func = static_cast<const ScriptingFunction&>(*object);

      out << func.type << " " << func.name << "(";
      for (const auto& param : func.parameters)
      {
        out << param.type << " " << param.name;
      }
      out << ")\n \n" << func.description;
    }
    break;

    case ScriptingObject::Type::CLASS:
    {
      const auto& cl = static_cast<const ScriptingClass&>(*object);

      out << cl.summary << "\n \n" << cl.instances;
    }
    break;
  }
  m_suggestion_description = out.str();
}

void
ControlScriptbox::autocomplete()
{
  assert(!m_suggestions.empty());

  std::string line_content = get_contents(m_caret.line);
  const size_t begin = line_content.find_first_not_of(" \t"); // Skip beginning spaces
  line_content = line_content.substr(begin);

  auto it = m_suggestions.begin();
  advance(it, m_selected_suggestion);
  const std::string& suggestion = it->first;

  size_t line_pos = line_content.find_last_of('.');
  if (line_pos == std::string::npos)
    line_pos = 0;
  else
    line_pos++;

  put_text(suggestion.substr(line_content.length() - line_pos));
  parse_value();
}

void
ControlScriptbox::on_caret_move()
{
  // To show suggestions, the caret must be at the end of a line
  if (m_caret.line_pos < static_cast<int>(m_charlist[m_caret.line].size()))
  {
    m_suggestions.clear();
    return;
  }

  std::string line_content = get_contents(m_caret.line);
  const size_t begin = line_content.find_first_not_of(" \t"); // Skip beginning spaces
  if (begin == std::string::npos)
  {
    m_suggestions.clear();
    return;
  }
  line_content = line_content.substr(begin);

  m_selected_suggestion = 0;
  m_suggestions = squirrel::autocomplete(line_content, true);
  update_description();

  // Re-calculate suggestions rect
  float max_suggestion_width = 0.f;
  for (const auto& suggestion : m_suggestions)
    max_suggestion_width = std::max(max_suggestion_width, Resources::control_font->get_text_width(suggestion.first));

  const float caret_lgt = Resources::control_font->get_text_width(get_first_chars(m_caret.line, m_caret.line_pos));
  m_suggestions_rect = Rectf(m_rect.p1() + Vector(caret_lgt + TEXT_X_OFFSET,
                                                  (m_caret.line + 1) * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                             Sizef(max_suggestion_width + TEXT_X_OFFSET * 2, SUGGESTION_RECT_HEIGHT));

  m_suggestions_scrollbar->set_rect(Rectf(Vector(m_suggestions_rect.get_right() - 5.f, m_suggestions_rect.get_top()),
                                          m_suggestions_rect.p2()));
}

/* EOF */
