//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "editor/toolbar_base_widget.hpp"

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/viewport.hpp"
#include "video/video_system.hpp"

void
EditorToolbarBaseWidget::set_position(const Vector& pos)
{
  float w = m_rect.get_width(),
        h = m_rect.get_height();
  
  m_rect.set_left(pos.x);
  m_rect.set_width(w);
  m_rect.set_top(pos.y);
  m_rect.set_height(h);

  m_menu_list->set_position(pos + Vector(0, m_rect.get_height()));
}

void
EditorToolbarBaseWidget::draw(DrawingContext& context)
{
  if (!get_visible())
    return;

  Widget::draw(context);

  if (m_grab)
  {
    context.color().draw_filled_rect(m_rect, g_config->editorgrabcolor, 4.0f,
                                     LAYER_GUI-5);
  }
  else if (m_hover && !m_disabled)
  {
    context.color().draw_filled_rect(m_rect, g_config->editorhovercolor, 4.0f,
                                     LAYER_GUI-5);
  }

  if (m_hover && !m_help_text.empty())
    context.color().draw_text(Resources::normal_font, m_help_text, m_mouse_pos + Vector(32, 32), FontAlignment::ALIGN_LEFT, INT_MAX);

  
  if (get_show_menu())
  {
    m_menu_list->draw(context);
  }
}

void
EditorToolbarBaseWidget::update(float dt_sec)
{
  if (!get_visible())
      return;

  if (get_show_menu())
  {
    m_menu_list->update(dt_sec);
  }
}

void
EditorToolbarBaseWidget::setup()
{
}

void
EditorToolbarBaseWidget::on_window_resize()
{
}

bool
EditorToolbarBaseWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  if (!get_visible())
    return false;

  if (button.button != SDL_BUTTON_LEFT)
    return false;
  // XXX: these shouldn't pass through
  if (m_disabled) return false;
      
  if (get_show_menu())
  {
    return m_menu_list->on_mouse_button_up(button);
  }

  if (m_grab)
  {
    m_grab = false;
    return true;
  }
  
  m_hover = false;
  return false;
}

bool
EditorToolbarBaseWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (!get_visible() || m_disabled || button.button != SDL_BUTTON_LEFT)
    return false;

  if (get_show_menu())
  {
    return m_menu_list->on_mouse_button_down(button);
  }

  if (m_rect.contains(m_mouse_pos))
  {
    m_hover = true;
    m_grab = true;
    return true;
  }

  m_hover = false;
  return false;
}

bool
EditorToolbarBaseWidget::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);

  if (!get_visible())
    return false;

  if (m_hover)
  {
    m_show_menu = m_menu_list != nullptr && m_menu_list->get_item_count() > 0;
  }
  else if (!m_menu_list->has_mouse_focus())
  {
    m_show_menu = false;
  }

  if (m_grab)
  {
    m_hover = m_rect.contains(m_mouse_pos);
    return true;
  } 
  
  if (m_rect.contains(m_mouse_pos))
  {
    m_hover = true;
    return false;
  }

  m_hover = false;
  return false;
}

void
EditorToolbarBaseWidget::set_help_text(const std::string& help_text)
{
  m_help_text = help_text;
}