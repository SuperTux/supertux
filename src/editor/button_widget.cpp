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

EditorToolbarButtonWidget::EditorToolbarButtonWidget(SpritePtr sprite, const Vector& pos,
                           std::function<void()> sig_click, std::optional<Sizef> sprite_size) :
  EditorToolbarBaseWidget(),
  m_sprite(std::move(sprite)),
  m_sig_click(std::move(sig_click)),
  m_flat(false)
{
  m_rect = Rectf(pos, sprite_size ? *sprite_size : m_sprite->get_size());
  set_position(pos);
}

EditorToolbarButtonWidget::EditorToolbarButtonWidget(const std::string& path, const Vector& pos, std::function<void()> callback, std::optional<Sizef> sprite_size) :
  EditorToolbarButtonWidget(SpriteManager::current()->create(path), std::move(pos), std::move(callback), std::move(sprite_size))
{
}

EditorToolbarButtonWidget::EditorToolbarButtonWidget(const std::string& path, std::function<void()> callback, const std::string& help_text, std::optional<Sizef> sprite_size) :
  EditorToolbarButtonWidget(SpriteManager::current()->create(path), Vector(0,0), std::move(callback), std::move(sprite_size))
{
  set_help_text(help_text);
}

void
EditorToolbarButtonWidget::draw(DrawingContext& context)
{
  if (!get_visible())
    return;

  if (!m_flat)
    context.color().draw_filled_rect(m_rect, Color(0.0f, 0.0f, 0.0f, 0.6f), 4.0f,
                                     LAYER_GUI-5);

  if (m_sprite)
  {
    if (m_disabled) context.set_alpha(0.4);
    m_sprite->draw_scaled(context.color(), m_rect.grown(-4.0f), LAYER_GUI - 5);
    if (m_disabled) context.set_alpha(1.0);
  }

  EditorToolbarBaseWidget::draw(context);
}

void
EditorToolbarButtonWidget::set_sprite(SpritePtr sprite)
{
  m_sprite = std::move(sprite);
  m_rect.set_size(sprite->get_width() * 1.0f, sprite->get_height() * 1.0f);
}

void
EditorToolbarButtonWidget::set_sprite(const std::string& path)
{
  set_sprite(SpriteManager::current()->create(path));
}

bool
EditorToolbarButtonWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  auto handled = EditorToolbarBaseWidget::on_mouse_button_up(button);

  if (m_hover)
  {
    if (m_sig_click)
    {
      m_sig_click();
      return true;
    }
  }

  return handled;
}