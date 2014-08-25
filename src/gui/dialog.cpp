//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "gui/dialog.hpp"

#include "control/controller.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

Dialog::Dialog() :
  m_text("<no-text>"),
  m_buttons(),
  m_selected_button()
{
}

void
Dialog::add_button(const std::string& text)
{
  m_buttons.push_back(text);
}

void
Dialog::process_input(const Controller& controller)
{
  if (controller.pressed(Controller::LEFT))
  {
    m_selected_button -= 1;
    m_selected_button = std::max(m_selected_button, 0);
  }

  if (controller.pressed(Controller::RIGHT))
  {
    m_selected_button += 1;
    m_selected_button = std::min(m_selected_button, static_cast<int>(m_buttons.size()) - 1);
  }     
}

void
Dialog::draw(DrawingContext& ctx)
{
  for(int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
  {
    if (i == m_selected_button)
    {
      // highlight
    }
    ctx.draw_text(Resources::normal_font, m_buttons[i],
                  Vector(100, 100),
                  ALIGN_CENTER, LAYER_GUI);
  }
}

/* EOF */
