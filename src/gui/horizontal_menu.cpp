//  SuperTux
//  Copyright (C) 2022-2023 Vankata453
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

#include "gui/horizontal_menu.hpp"

#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "math/util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/viewport.hpp"

float HorizontalMenu::s_max_width = 0.0f; // Dynamically set
const float HorizontalMenu::s_height = 100.0f;

const float HorizontalMenu::s_width_offset = 20.0f;
const float HorizontalMenu::s_item_spacing = 30.0f;

const float HorizontalMenu::s_icon_y = 10.0f;
const float HorizontalMenu::s_text_y = s_height - 30.0f;

HorizontalMenu::HorizontalMenu() :
  m_items(),
  m_selected_item(0),
  m_item_range_begin(0),
  m_item_range_end(0),
  m_width(0.f),
  m_rect()
{
  on_window_resize();
  calculate_width();
}

void
HorizontalMenu::add_item(const std::string& text, const std::string& description,
                         const std::string& icon_file, int id)
{
  // Wrap description
  const float desc_max_width = Resources::normal_font->get_text_width(text) + 300;
  std::string overflow;
  std::string desc_result = Resources::normal_font->wrap_to_width(description, desc_max_width, &overflow);
  while (!overflow.empty())
    desc_result += "\n" + Resources::normal_font->wrap_to_width(overflow, desc_max_width, &overflow);

  m_items.push_back({ id, text, desc_result, Surface::from_file(icon_file) });
  calculate_width();
}

void
HorizontalMenu::calculate_width(int begin)
{
  const float leftover_spacing = std::fabs(10.0f - s_item_spacing);

  const int total = static_cast<int>(m_items.size());
  int end = total - 1;

  float width = 10.0f;
  for (int i = begin; i < total; i++)
  {
    float item_width = Resources::normal_font->get_text_width(m_items[i].text) + s_item_spacing;
    width += item_width;
    if (width > s_max_width)
    {
      // Try removing the leftover spacing after the last item.
      width -= leftover_spacing;
      item_width -= leftover_spacing;

      if (width > s_max_width) // The width of the item is still bigger.
      {
        end = i - 1;
        width -= item_width - leftover_spacing; // Remove the leftover spacing after the last item.
      }
      else // The item can fit.
      {
        end = i;
      }
      break;
    }
  }

  m_item_range_begin = begin;
  m_item_range_end = end;
  m_width = width;

  calculate_rect();
}

void
HorizontalMenu::calculate_rect()
{
  m_rect = Rectf(Vector(s_width_offset + (s_max_width - m_width) / 2, get_y()),
                 Sizef(m_width, s_height));
}

void
HorizontalMenu::on_window_resize()
{
  s_max_width = static_cast<float>(SCREEN_WIDTH) - s_width_offset * 2;
  calculate_width();
}

void
HorizontalMenu::draw(DrawingContext& context)
{
  // Draw background rect.
  context.color().draw_filled_rect(m_rect.grown(8.0f),
                                   Color(g_config->menufrontcolor.red, g_config->menufrontcolor.green,
                                         g_config->menufrontcolor.blue, std::max(0.f, g_config->menufrontcolor.alpha - 0.3f)),
                                         g_config->menuroundness,
                                   LAYER_GUI - 10);

  // Draw items.
  float pos_x = m_rect.get_left() + 22.0f;
  for (int i = m_item_range_begin; i <= m_item_range_end; i++)
  {
    const float text_width = Resources::normal_font->get_text_width(m_items[i].text);
    draw_item(context, i, pos_x, text_width);

    pos_x += text_width + s_item_spacing;
  }

  // Draw arrows.
  if (m_item_range_begin > 0)
    context.color().draw_text(Resources::big_font, "<",
                              Vector(m_rect.get_left() + 5.0f, get_y() + s_height / 2.5f),
                              ALIGN_CENTER, LAYER_GUI);
  if (m_item_range_end < static_cast<int>(m_items.size()) - 1)
    context.color().draw_text(Resources::big_font, ">",
                              Vector(m_rect.get_right() - 5.0f, get_y() + s_height / 2.5f),
                              ALIGN_CENTER, LAYER_GUI);
}

void
HorizontalMenu::draw_item(DrawingContext& context, const int& index,
                          const float& pos_x, const float& text_width)
{
  // Draw icon.
  context.color().draw_surface_scaled(m_items[index].icon, Rectf(Vector(pos_x + text_width / 2 - static_cast<float>(m_items[index].icon->get_width()) / 2,
                                                                        get_y() + s_icon_y),
                                                                 Sizef(s_height / 2, s_height / 2)), LAYER_GUI);

  // Draw text.
  context.color().draw_text(Resources::normal_font, m_items[index].text,
                            Vector(pos_x, get_y() + s_text_y),
                            ALIGN_LEFT, LAYER_GUI);

  // Draw selection border, if selected.
  if (index == m_selected_item)
  {
    const float blink = (sinf(g_real_time * math::PI * 1.0f) / 2.0f + 0.5f) * 0.5f + 0.25f;

    const float text_height = Resources::normal_font->get_text_height(m_items[index].text);
    const Rectf item_rect(Vector(pos_x - 7.5f, get_y() + s_icon_y - 5.f),
                          Sizef(text_width + 10.f, s_text_y + text_height));

    context.color().draw_filled_rect(item_rect,
                                     Color(1.0f, 1.0f, 1.0f, blink),
                                     std::max(0.f, g_config->menuroundness - 2.f),
                                     LAYER_GUI - 10);
    context.color().draw_filled_rect(item_rect.grown(3.0f),
                                     Color(1.0f, 1.0f, 1.0f, 0.5f),
                                     std::max(0.f, g_config->menuroundness - 4.f),
                                     LAYER_GUI - 10);

    // Draw description, if available.
    if (!m_items[index].description.empty())
    {
      const float desc_width = Resources::normal_font->get_text_width(m_items[index].description);
      const float desc_height = Resources::normal_font->get_text_height(m_items[index].description);
      const Rectf desc_rect(pos_x + text_width / 2 - desc_width / 1.5f,
                            m_rect.get_bottom() + 48.f - 4.f,
                            pos_x + text_width / 2 + desc_width / 1.5f,
                            m_rect.get_bottom() + 48.f + desc_height + 8.f);

      context.color().draw_filled_rect(desc_rect.grown(8.0f),
                                       g_config->menuhelpfrontcolor,
                                       g_config->menuroundness,
                                       LAYER_GUI);
      context.color().draw_filled_rect(desc_rect.grown(12.0f),
                                       g_config->menuhelpbackcolor,
                                       g_config->menuroundness + 4.f,
                                       LAYER_GUI);

      context.color().draw_text(Resources::big_font, m_items[index].description,
                                Vector(desc_rect.get_left() + desc_rect.get_width() / 2, desc_rect.get_top()),
                                ALIGN_CENTER, LAYER_GUI);
    }
  }
}

void
HorizontalMenu::process_action(const MenuAction& action)
{
  switch (action)
  {
    case MenuAction::LEFT:
      go_left();
      break;

    case MenuAction::RIGHT:
      go_right();
      break;

    case MenuAction::HIT:
      // Execute an action for the specific item.
      if (interactable()) menu_action(m_items[m_selected_item]);
      break;

    case MenuAction::BACK:
      MenuManager::instance().pop_menu();
      return;

    default:
      break;
  }
}

void
HorizontalMenu::event(const SDL_Event& ev)
{
  switch (ev.type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      if (!MenuManager::instance().is_active()) break;

      if (ev.button.button == SDL_BUTTON_LEFT)
      {
        const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);

        if (m_rect.contains(mouse_pos))
        {
          // Execute an action for the specific item.
          if (interactable()) menu_action(m_items[m_selected_item]);
        }
      }
      break;
    }
    case SDL_MOUSEMOTION:
    {
      if (!MenuManager::instance().is_active()) break;

      const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);

      if (m_rect.contains(mouse_pos))
      {
        float pos_x = m_rect.get_left() + 20.0f;
        for (int i = m_item_range_begin; i <= m_item_range_end; i++)
        {
          const float text_width = Resources::normal_font->get_text_width(m_items[i].text);
          const Rectf item_rect(Vector(pos_x - 2.0f, get_y() + s_icon_y),
                                Sizef(text_width, s_text_y + Resources::normal_font->get_text_height(m_items[i].text)));

          if (item_rect.contains(mouse_pos))
          {
            m_selected_item = i;
            break;
          }

          pos_x += text_width + s_item_spacing;
        }

        if (MouseCursor::current() && interactable())
          MouseCursor::current()->set_state(MouseCursorState::LINK);
      }
      else
      {
        if (MouseCursor::current())
          MouseCursor::current()->set_state(MouseCursorState::NORMAL);
      }
      break;
    }
  }
}

// Navigation functions

void
HorizontalMenu::go_left()
{
  // If the selected item is the first, go from the back.
  if (m_selected_item == 0)
  {
    m_selected_item = static_cast<int>(m_items.size()) - 1;
    calculate_width(m_selected_item - m_item_range_end);
    return;
  }

  m_selected_item--;
  if (m_selected_item < m_item_range_begin)
    calculate_width(m_selected_item);
}

void
HorizontalMenu::go_right()
{
  // If the selected item is last, go from the beginning.
  if (m_selected_item == static_cast<int>(m_items.size()) - 1)
  {
    m_selected_item = 0;
    calculate_width(0);
    return;
  }

  m_selected_item++;
  if (m_selected_item > m_item_range_end)
    calculate_width(m_item_range_begin + 1);
}

/* EOF */
