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

#include "gui/item_scriptfield.hpp"

#include "supertux/globals.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

ItemScriptField::ItemScriptField(std::unique_ptr<ControlTextbox> control, const Sizef& relative_size) :
  MenuItem(""),
  m_control(std::move(control)),
  m_relative_size(relative_size)
{
  on_window_resize();
}

void
ItemScriptField::draw(DrawingContext& context, const Vector& pos, int, bool)
{
  m_control->set_pos(Vector(pos.x + 12.f,
                            pos.y - m_control->get_rect().get_height() / 2));
  m_control->draw(context);
}

void
ItemScriptField::update(float dt_sec)
{
  m_control->update(dt_sec);
}

void
ItemScriptField::process_action(const MenuAction& action)
{
  switch (action)
  {
    case MenuAction::SELECT:
      m_control->set_focus(true);
      break;
    case MenuAction::UNSELECT:
      m_control->set_focus(false);
      break;
  }
}

void
ItemScriptField::event(const SDL_Event& ev)
{
  m_control->event(ev);
}

void
ItemScriptField::on_window_resize()
{
  m_control->set_size(Sizef(static_cast<float>(SCREEN_WIDTH) * m_relative_size.width,
                            static_cast<float>(SCREEN_HEIGHT) * m_relative_size.height));
}

int
ItemScriptField::get_width() const
{
  return static_cast<int>(m_control->get_rect().get_width());
}

int
ItemScriptField::get_height() const
{
  return static_cast<int>(m_control->get_rect().get_height());
}

/* EOF */
