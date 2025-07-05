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

#include "editor/button_widget.hpp"

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/viewport.hpp"
#include "video/video_system.hpp"

ButtonWidget::ButtonWidget(SpritePtr sprite, const Vector& pos,
                           std::function<void()> sig_click) :
  m_sprite(std::move(sprite)),
  m_rect(pos, Sizef(static_cast<float>(m_sprite->get_width()),
                    static_cast<float>(m_sprite->get_height()))),
  m_grab(false),
  m_hover(false),
  m_sig_click(std::move(sig_click)),
  m_mouse_pos(),
  m_help_text()
{
}

void
ButtonWidget::draw(DrawingContext& context)
{
  context.color().draw_filled_rect(m_rect, Color(0.0f, 0.0f, 0.0f, 0.6f), 4.0f,
                                   LAYER_GUI-5);

  if (m_sprite) {
    m_sprite->draw_scaled(context.color(), m_rect.grown(-4.0f), LAYER_GUI - 5);
  }

  if (m_grab) {
    context.color().draw_filled_rect(m_rect, g_config->editorgrabcolor, 4.0f,
                                     LAYER_GUI-5);
  } else if (m_hover) {
    context.color().draw_filled_rect(m_rect, g_config->editorhovercolor, 4.0f,
                                     LAYER_GUI-5);
  }

  if (m_hover && !m_help_text.empty())
  {
    const auto& font = Resources::control_font;
    const auto text_height = font->get_text_height(m_help_text);
    const auto text_width = font->get_text_width(m_help_text);
    const auto text_rect = Rectf(m_mouse_pos + Vector(32, 32), m_mouse_pos + Vector(32, 32) + Vector(text_width, text_height));
    context.color().draw_filled_rect(text_rect, Color::BLACK, INT_MAX);
    context.color().draw_text(font, m_help_text, m_mouse_pos + Vector(32, 32), FontAlignment::ALIGN_LEFT, INT_MAX);
  }
}

void
ButtonWidget::update(float dt_sec)
{
}

void
ButtonWidget::setup()
{
}

void
ButtonWidget::on_window_resize()
{
}

bool
ButtonWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  if (button.button != SDL_BUTTON_LEFT) return false;

  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(button.x, button.y);

  if (m_grab) {
    if (m_rect.contains(m_mouse_pos)) {
      if (m_sig_click) {
        m_sig_click();
      }
    }
    m_grab = false;
    return true;
  } else {
    m_hover = false;
    return false;
  }
}

bool
ButtonWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (button.button != SDL_BUTTON_LEFT) return false;

  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(button.x, button.y);

  if (m_rect.contains(m_mouse_pos)) {
    m_hover = true;
    m_grab = true;
    return true;
  } else {
    m_hover = false;
    return false;
  }
}

bool
ButtonWidget::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);

  if (m_grab) {
    m_hover = m_rect.contains(m_mouse_pos);
    return true;
  } else if (m_rect.contains(m_mouse_pos)) {
    m_hover = true;
    return false;
  } else {
    m_hover = false;
    return false;
  }
}

void
ButtonWidget::set_sprite(SpritePtr sprite)
{
  m_sprite = std::move(sprite);
  m_rect.set_size(sprite->get_width() * 1.0f, sprite->get_height() * 1.0f);
}

void
ButtonWidget::set_sprite(const std::string& path)
{
  set_sprite(SpriteManager::current()->create(path));
}

void
ButtonWidget::set_help_text(const std::string& help_text)
{
  m_help_text = help_text;
}
