//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "gui/menu_manager.hpp"

#include <assert.h>

#include "control/input_manager.hpp"
#include "gui/menu.hpp"
#include "gui/mousecursor.hpp"
#include "math/sizef.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/timer.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"

MenuManager* MenuManager::s_instance = 0;

MenuManager&
MenuManager::instance()
{
  assert(s_instance);
  return *s_instance;
}

namespace {

Rectf menu2rect(const Menu& menu)
{
  return Rectf(menu.get_center_pos().x - menu.get_width() / 2,
               menu.get_center_pos().y - menu.get_height() / 2,
               menu.get_center_pos().x + menu.get_width() / 2,
               menu.get_center_pos().y + menu.get_height() / 2);
}

} // namespace

class MenuTransition
{
private:
  Rectf m_from_rect;
  Rectf m_to_rect;

  float m_effect_progress;
  float m_effect_start_time;
  bool m_is_active;

public:
  MenuTransition() :
    m_from_rect(),
    m_to_rect(),
    m_effect_progress(1.0f),
    m_effect_start_time(),
    m_is_active(false)
  {
  }

  void start(const Rectf& from_rect,
             const Rectf& to_rect)
  {
    m_from_rect = from_rect;
    m_to_rect = to_rect;

    m_effect_start_time = real_time;
    m_effect_progress = 0.0f;

    m_is_active = true;
  }

  void set(const Rectf& rect)
  {
    m_to_rect = m_from_rect = rect;
  }

  void update()
  {
    if (m_is_active)
    {
      m_effect_progress = (real_time - m_effect_start_time) * 6.0f;

      if (m_effect_progress > 1.0f)
      {
        m_effect_progress = 1.0f;
        m_is_active = false;
      }
    }
  }

  void draw(DrawingContext& context)
  {
    float p = m_effect_progress;

    Rectf rect = m_to_rect;
    if (m_is_active)
    {
      rect.p1.x = (m_to_rect.p1.x * p) + (m_from_rect.p1.x * (1.0f - p));
      rect.p1.y = (m_to_rect.p1.y * p) + (m_from_rect.p1.y * (1.0f - p));
      rect.p2.x = (m_to_rect.p2.x * p) + (m_from_rect.p2.x * (1.0f - p));
      rect.p2.y = (m_to_rect.p2.y * p) + (m_from_rect.p2.y * (1.0f - p));
    }

    // draw menu background rectangles
    context.draw_filled_rect(Rectf(rect.p1.x - 4, rect.p1.y - 10-4,
                                   rect.p2.x + 4, rect.p2.y + 10 + 4),
                             Color(0.2f, 0.3f, 0.4f, 0.8f),
                             20.0f,
                             LAYER_GUI-10);

    context.draw_filled_rect(Rectf(rect.p1.x, rect.p1.y - 10,
                                   rect.p2.x, rect.p2.y + 10),
                             Color(0.6f, 0.7f, 0.8f, 0.5f),
                             16.0f,
                             LAYER_GUI-10);
  }

  bool is_active()
  {
    return m_is_active;
  }
};

MenuManager::MenuManager() :
  m_menu_stack(),
  m_transition(new MenuTransition)
{
  s_instance = this;
}

MenuManager::~MenuManager()
{
  s_instance = nullptr;
}

void
MenuManager::refresh()
{
  for(auto i = m_menu_stack.begin(); i != m_menu_stack.end(); ++i)
  {
    (*i)->refresh();
  }
}

void
MenuManager::process_input()
{
  if (current())
  {
    current()->process_input();
  }
}

void
MenuManager::event(const SDL_Event& event)
{
  if (current() && !m_transition->is_active())
  {
    // only pass events when the menu is fully visible and not in a
    // transition animation
    current()->event(event);
  }
}

void
MenuManager::draw(DrawingContext& context)
{
  if (m_transition->is_active())
  {
    m_transition->update();
    m_transition->draw(context);
  }
  else
  {
    if (current())
    {
      // brute force the transition into the right shape in case the
      // menu has changed sizes
      m_transition->set(menu2rect(*current()));
      m_transition->draw(context);

      current()->draw(context);
    }
  }

  if (current() && MouseCursor::current())
  {
    MouseCursor::current()->draw(context);
  }
}

bool
MenuManager::check_menu()
{
  if (current())
  {
    current()->check_menu();
    return true;
  }
  else
  {
    return false;
  }
}

void
MenuManager::push_menu(int id)
{
  push_menu(MenuStorage::instance().create(static_cast<MenuStorage::MenuId>(id)));
}

void
MenuManager::set_menu(int id)
{
  set_menu(MenuStorage::instance().create(static_cast<MenuStorage::MenuId>(id)));
}

void
MenuManager::push_menu(std::unique_ptr<Menu> menu)
{
  assert(menu);
  transition(m_menu_stack.empty() ? nullptr : m_menu_stack.back().get(),
             menu.get());
  m_menu_stack.push_back(std::move(menu));
}

void
MenuManager::pop_menu()
{
  if (m_menu_stack.empty())
  {
    log_warning << "trying to pop on an empty menu_stack" << std::endl;
  }
  else
  {
    transition(m_menu_stack.back().get(),
               (m_menu_stack.size() >= 2)
               ? m_menu_stack[m_menu_stack.size() - 2].get()
               : nullptr);

    m_menu_stack.pop_back();
  }
}

void
MenuManager::set_menu(std::unique_ptr<Menu> menu)
{
  if (menu)
  {
    transition(m_menu_stack.empty() ? nullptr : m_menu_stack.back().get(),
               menu.get());
    m_menu_stack.clear();
    m_menu_stack.push_back(std::move(menu));
  }
  else
  {
    transition(m_menu_stack.empty() ? nullptr : m_menu_stack.back().get(),
               nullptr);
    m_menu_stack.clear();
  }

  // just to be sure...
  g_input_manager->reset();
}

void
MenuManager::clear_menu_stack()
{
  transition(m_menu_stack.empty() ? nullptr : m_menu_stack.back().get(),
             nullptr);
  m_menu_stack.clear();
}

void
MenuManager::on_window_resize()
{
  for(auto i = m_menu_stack.begin(); i != m_menu_stack.end(); ++i)
  {
    (*i)->on_window_resize();
  }
}

Menu*
MenuManager::current() const
{
  if (m_menu_stack.empty())
  {
    return nullptr;
  }
  else
  {
    return m_menu_stack.back().get();
  }
}

void
MenuManager::transition(Menu* from, Menu* to)
{
  if (!from && !to)
  {
    return;
  }
  else
  {
    Rectf from_rect;
    if (from)
    {
      from_rect = menu2rect(*from);
    }
    else
    {
      from_rect = Rectf(to->get_center_pos(), Sizef(0, 0));
    }

    Rectf to_rect;
    if (to)
    {
      to_rect = menu2rect(*to);
    }
    else
    {
      to_rect = Rectf(from->get_center_pos(), Sizef(0, 0));
    }

    m_transition->start(from_rect, to_rect);
  }
}

/* EOF */
