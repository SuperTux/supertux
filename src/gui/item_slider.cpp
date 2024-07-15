//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "gui/item_slider.hpp"

#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

static const float SLIDER_WIDTH = 100.f;

ItemSlider::ItemSlider(const std::string& text, int min_value, int max_value, int* value, const std::string& value_append, int id) :
  MenuItem(text, id),
  m_min_value(min_value),
  m_max_value(max_value),
  m_value(value),
  m_value_append(value_append),
  m_slider_x(-SLIDER_WIDTH), // Will be set in draw().
  m_sliding(false)
{
}

void
ItemSlider::draw(DrawingContext& context, const Vector& pos, int menu_width, bool active)
{
  assert(m_min_value < m_max_value);
  assert(*m_value >= m_min_value && *m_value <= m_max_value);

  const float value_text_width = Resources::normal_font->get_text_width(std::to_string(m_max_value) + m_value_append);

  context.color().draw_text(Resources::normal_font, get_text(),
                              Vector(pos.x + 16.f,
                                     pos.y - Resources::normal_font->get_height() / 2.f),
                              ALIGN_LEFT, LAYER_GUI, active ? g_config->activetextcolor : get_color());

  m_slider_x = pos.x + static_cast<float>(menu_width) - SLIDER_WIDTH - value_text_width - 32.f;
  context.color().draw_filled_rect(Rectf(Vector(m_slider_x, pos.y - 1.f),
                                         Vector(pos.x + static_cast<float>(menu_width) - value_text_width - 32.f, pos.y + 1.f)),
                                         active ? Color::BLACK : get_color(), LAYER_GUI);

  const float slider_indicator_x = m_slider_x + (static_cast<float>(*m_value - m_min_value) / static_cast<float>(m_max_value - m_min_value)) * SLIDER_WIDTH;
  context.color().draw_filled_rect(Rectf(Vector(slider_indicator_x - 2.f, pos.y - Resources::normal_font->get_height() / 2 + 1.f),
                                         Vector(slider_indicator_x + 2.f, pos.y + Resources::normal_font->get_height() / 2 - 1.f)),
                                   active ? Color::BLACK : get_color(), LAYER_GUI);

  context.color().draw_text(Resources::normal_font, std::to_string(*m_value) + m_value_append,
                            Vector(pos.x + static_cast<float>(menu_width) - 16.f,
                                   pos.y - Resources::normal_font->get_height() / 2.f),
                            ALIGN_RIGHT, LAYER_GUI, active ? g_config->activetextcolor : get_color());
}

int
ItemSlider::get_width() const
{
  const float value_text_width = Resources::normal_font->get_text_width(std::to_string(m_max_value) + m_value_append);
  return static_cast<int>(Resources::normal_font->get_text_width(get_text()) + SLIDER_WIDTH + value_text_width + 48.f);
}

void
ItemSlider::event(const SDL_Event& ev)
{
  switch (ev.type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      if (ev.button.button == SDL_BUTTON_LEFT)
      {
        const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);
        if (mouse_pos.x >= m_slider_x && mouse_pos.x <= m_slider_x + SLIDER_WIDTH)
        {
          *m_value = ((static_cast<float>(mouse_pos.x) - m_slider_x) / SLIDER_WIDTH) * (m_max_value - m_min_value) + m_min_value;
          m_sliding = true;

          MenuManager::instance().current_menu()->menu_action(*this);
        }
      }
      break;
    }

    case SDL_MOUSEBUTTONUP:
      if (ev.button.button == SDL_BUTTON_LEFT)
        m_sliding = false;
      break;

    case SDL_MOUSEMOTION:
      if (m_sliding)
      {
        const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);
        if (mouse_pos.x <= m_slider_x)
          *m_value = m_min_value;
        else if (mouse_pos.x >= m_slider_x + SLIDER_WIDTH)
          *m_value = m_max_value;
        else
          *m_value = ((static_cast<float>(mouse_pos.x) - m_slider_x) / SLIDER_WIDTH) * (m_max_value - m_min_value) + m_min_value;

        MenuManager::instance().current_menu()->menu_action(*this);
      }
      break;

    case SDL_KEYDOWN:
      if (ev.key.keysym.sym == SDLK_LEFT)
        *m_value = std::max(*m_value - 1, m_min_value);
      else if (ev.key.keysym.sym == SDLK_RIGHT)
        *m_value = std::min(*m_value + 1, m_max_value);

      MenuManager::instance().current_menu()->menu_action(*this);
      break;
  }
}

/* EOF */
