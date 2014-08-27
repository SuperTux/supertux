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
#include "gui/menu_manager.hpp"
#include "gui/menu.hpp"
#include "gui/mousecursor.hpp"
#include "supertux/resources.hpp"
#include "supertux/colorscheme.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

Dialog::Dialog() :
  m_text(),
  m_buttons(),
  m_selected_button(),
  m_cancel_button(-1),
  m_text_size()
{
}

Dialog::~Dialog()
{
}

void
Dialog::set_text(const std::string& text)
{
  m_text = text;

  m_text_size = Sizef(Resources::normal_font->get_text_width(m_text),
                      Resources::normal_font->get_text_height(m_text));

}

void
Dialog::clear_buttons()
{
  m_buttons.clear();
  m_selected_button = 0;
  m_cancel_button = -1;
}

void
Dialog::add_default_button(const std::string& text, const std::function<void ()>& callback)
{
  add_button(text, callback);
  m_selected_button = m_buttons.size() - 1;
}

void
Dialog::add_cancel_button(const std::string& text, const std::function<void ()>& callback)
{
  add_button(text, callback);
  m_cancel_button = m_buttons.size() - 1;
}

void
Dialog::add_button(const std::string& text, const std::function<void ()>& callback)
{
  m_buttons.push_back({text, callback});
}

int
Dialog::get_button_at(const Vector& mouse_pos) const
{
  Rectf bg_rect(Vector(SCREEN_WIDTH/2 - m_text_size.width/2,
                       SCREEN_HEIGHT/2 - m_text_size.height/2),
                Sizef(m_text_size.width,
                      m_text_size.height + 44));

  for(int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
  {
    float segment_width = bg_rect.get_width() / m_buttons.size();
    float button_width = segment_width;
    float button_height = 24.0f;
    Vector pos(bg_rect.p1.x + segment_width/2.0f + i * segment_width,
               bg_rect.p2.y - 12);
    Rectf button_rect(Vector(pos.x - button_width/2, pos.y - button_height/2),
                      Vector(pos.x + button_width/2, pos.y + button_height/2));
    if (button_rect.contains(mouse_pos))
    {
      return i;
    }
  }
  return -1;
}

void
Dialog::event(const SDL_Event& ev)
{
  switch(ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    if(ev.button.button == SDL_BUTTON_LEFT)
    {
      Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      int new_button = get_button_at(mouse_pos);
      if (new_button != -1)
      {
        m_selected_button = new_button;
        on_button_click(m_selected_button);
      }
    }
    break;

    case SDL_MOUSEMOTION:
    {
      Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      int new_button = get_button_at(mouse_pos);
      if (new_button != -1)
      {
        m_selected_button = new_button;
        if(MouseCursor::current())
          MouseCursor::current()->set_state(MC_LINK);
      }
      else
      {
        if(MouseCursor::current())
          MouseCursor::current()->set_state(MC_NORMAL);
      }
    }
    break;

    default:
      break;
  }
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

  if (controller.pressed(Controller::ACTION) ||
      controller.pressed(Controller::MENU_SELECT))
  {
    on_button_click(m_selected_button);
  }

  if (m_cancel_button != -1 &&
      (controller.pressed(Controller::ESCAPE) ||
       controller.pressed(Controller::MENU_BACK)))
  {
    on_button_click(m_cancel_button);
  }
}

void
Dialog::draw(DrawingContext& ctx)
{
  Rectf bg_rect(Vector(SCREEN_WIDTH/2 - m_text_size.width/2,
                       SCREEN_HEIGHT/2 - m_text_size.height/2),
                Sizef(m_text_size.width,
                      m_text_size.height + 44));

  // draw background rect
  ctx.draw_filled_rect(bg_rect.grown(12.0f),
                       Color(0.2f, 0.3f, 0.4f, 0.8f),
                       16.0f,
                       LAYER_GUI-10);

  ctx.draw_filled_rect(bg_rect.grown(8.0f),
                       Color(0.6f, 0.7f, 0.8f, 0.5f),
                       16.0f,
                       LAYER_GUI-10);

  // draw text
  ctx.draw_text(Resources::normal_font, m_text,
                Vector(bg_rect.p1.x + bg_rect.get_width()/2.0f,
                       bg_rect.p1.y),
                ALIGN_CENTER, LAYER_GUI);

  // draw HL line
  ctx.draw_filled_rect(Vector(bg_rect.p1.x, bg_rect.p2.y - 35),
                       Vector(bg_rect.get_width(), 4),
                       Color(0.6f, 0.7f, 1.0f, 1.0f), LAYER_GUI);
  ctx.draw_filled_rect(Vector(bg_rect.p1.x, bg_rect.p2.y - 35),
                       Vector(bg_rect.get_width(), 2),
                       Color(1.0f, 1.0f, 1.0f, 1.0f), LAYER_GUI);

  // draw buttons
  for(int i = 0; i < static_cast<int>(m_buttons.size()); ++i)
  {
    float segment_width = bg_rect.get_width() / m_buttons.size();
    float button_width = segment_width;
    float button_height = 24.0f;
    Vector pos(bg_rect.p1.x + segment_width/2.0f + i * segment_width,
               bg_rect.p2.y - 12);

    if (i == m_selected_button)
    {
      float blink = (sinf(real_time * M_PI * 1.0f)/2.0f + 0.5f) * 0.5f + 0.25f;
      ctx.draw_filled_rect(Rectf(Vector(pos.x - button_width/2, pos.y - button_height/2),
                                 Vector(pos.x + button_width/2, pos.y + button_height/2)).grown(2.0f),
                           Color(1.0f, 1.0f, 1.0f, blink),
                           14.0f,
                           LAYER_GUI-10);
      ctx.draw_filled_rect(Rectf(Vector(pos.x - button_width/2, pos.y - button_height/2),
                                 Vector(pos.x + button_width/2, pos.y + button_height/2)),
                           Color(1.0f, 1.0f, 1.0f, 0.5f),
                           12.0f,
                           LAYER_GUI-10);
    }

    ctx.draw_text(Resources::normal_font, m_buttons[i].text,
                  Vector(pos.x, pos.y - int(Resources::normal_font->get_height()/2)),
                  ALIGN_CENTER, LAYER_GUI,
                  i == m_selected_button ? ColorScheme::Menu::active_color : ColorScheme::Menu::default_color);
  }
}

void
Dialog::on_button_click(int button) const
{
  if (m_buttons[button].callback)
  {
    m_buttons[button].callback();
  }
  MenuManager::instance().set_dialog({});
}

/* EOF */
