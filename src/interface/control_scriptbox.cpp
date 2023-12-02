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

ControlScriptbox::ControlScriptbox() :
  ControlTextbox(true),
  m_suggestions()
{
}

void
ControlScriptbox::draw(DrawingContext& context)
{
  ControlTextbox::draw(context);

  if (!m_has_focus || m_suggestions.empty())
    return;

  float max_suggestion_width = 0.f;
  for (const auto& suggestion : m_suggestions)
    max_suggestion_width = std::max(max_suggestion_width, Resources::control_font->get_text_width(suggestion));

  const float caret_lgt = Resources::control_font->get_text_width(get_first_chars(m_caret.line, m_caret.line_pos));
  const Rectf suggestions_rect(m_rect.p1() + Vector(caret_lgt + TEXT_X_OFFSET,
                                                    (m_caret.line + 1) * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                               Sizef(max_suggestion_width + TEXT_X_OFFSET * 2, 100.f));

  context.color().draw_filled_rect(suggestions_rect, Color(0.9f, 0.9f, 0.9f, 1.f), LAYER_GUI + 1);

  context.push_transform();
  context.set_viewport(suggestions_rect.to_rect());

  for (size_t i = 0; i < m_suggestions.size(); i++)
    context.color().draw_text(Resources::control_font, m_suggestions[i],
                              Vector(TEXT_X_OFFSET, i * Resources::control_font->get_height() + TEXT_Y_OFFSET),
                              ALIGN_LEFT, LAYER_GUI + 2, Color::BLACK);

  context.pop_transform();
}

bool
ControlScriptbox::validate_value()
{
  if (!ControlTextbox::validate_value())
    return false;

  // Do not show autocomplete suggestions when selecting
  if (m_caret.pos != m_secondary_caret.pos)
  {
    m_suggestions.clear();
    return true;
  }

  std::string line_content = get_contents(m_caret.line);
  std::remove(line_content.begin(), line_content.end(), ' '); // Remove spaces

  if (line_content.empty()) // TODO: Temporary
    m_suggestions.clear();
  else
    m_suggestions = { "grow", "heh", "oooo" };

  return true;
}

/* EOF */
