//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "editor/tip.hpp"

#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"

Tip::Tip(GameObject& object) :
  m_strings(),
  m_warnings(),
  m_header()
{
  auto os = object.get_settings();
  m_header = os.get_name();

  for (const auto& oo_ptr : os.get_options())
  {
    const auto& oo = *oo_ptr;

    if (dynamic_cast<LabelObjectOption*>(oo_ptr.get()))
    {
      m_strings.push_back(oo.get_text());
    }
    if (!(oo.get_flags() & OPTION_HIDDEN))
    {
      auto value = oo.to_string();
      if (!value.empty())
      {
        m_strings.push_back(oo.get_text() + ": " + value);
      }
    }
  }

  if (!object.is_up_to_date())
    m_warnings.push_back(_("This object's current functionality is deprecated.") + "\n" +
                         _("Updating to get its latest functionality is recommended."));
}

Tip::Tip(std::string text) :
  m_strings(),
  m_header(text)
{
}

Tip::Tip(std::string header, std::vector<std::string> text) :
  m_strings(text),
  m_header(header)
{
}

void
Tip::draw(DrawingContext& context, const Vector& pos, const bool align_right)
{
  auto position = pos;
  position.y += 35;
  context.color().draw_text(Resources::normal_font, m_header, position,
                            align_right ? ALIGN_RIGHT : ALIGN_LEFT, LAYER_GUI + 10, g_config->labeltextcolor);

  for (const auto& str : m_strings)
  {
    position.y += 22;
    context.color().draw_text(Resources::normal_font, str, position,
                              align_right ? ALIGN_RIGHT : ALIGN_LEFT, LAYER_GUI + 10, ColorScheme::Menu::default_color);
  }

  position.y += 35;
  for (const auto& str : m_warnings)
  {
    position.y += 22;
    context.color().draw_text(Resources::normal_font, str, position,
                              align_right ? ALIGN_RIGHT : ALIGN_LEFT, LAYER_GUI + 10, ColorScheme::Menu::warning_color);
  }
}

void
Tip::draw_up(DrawingContext& context, const Vector& pos, const bool align_right)
{
  auto position = Vector(pos.x, pos.y - (static_cast<float>(m_strings.size()) + 1.0f) * 22.0f - 35.0f);
  draw(context, position, align_right);
}

/* EOF */
