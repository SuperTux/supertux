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

#include "control/input_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/menu.hpp"
#include "gui/mousecursor.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"

MenuManager* MenuManager::s_instance = nullptr;

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

class MenuTransition final
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

    m_effect_start_time = g_real_time;
    m_effect_progress = 0.0f;

    m_is_active = true;
  }

  void set(const Rectf& rect)
  {
    m_to_rect = m_from_rect = rect;
  }

  void update()
  {
    if (!g_config->transitions_enabled && m_is_active)
    {
      m_effect_progress = 1.0f;
      m_is_active = false;
      return;
    }
    if (m_is_active)
    {
      m_effect_progress = (g_real_time - m_effect_start_time) * 6.0f;

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
      rect = Rectf((m_to_rect.get_left() * p) + (m_from_rect.get_left() * (1.0f - p)),
                   (m_to_rect.get_top() * p) + (m_from_rect.get_top() * (1.0f - p)),
                   (m_to_rect.get_right() * p) + (m_from_rect.get_right() * (1.0f - p)),
                   (m_to_rect.get_bottom() * p) + (m_from_rect.get_bottom() * (1.0f - p)));
    }

    // draw menu background rectangles
    context.color().draw_filled_rect(Rectf(rect.get_left() - 4, rect.get_top() - 10-4,
                                             rect.get_right() + 4, rect.get_bottom() + 10 + 4),
                                       Color(g_config->menubackcolor),
                                       g_config->menuroundness + 4.f,
                                       LAYER_GUI-10);

    context.color().draw_filled_rect(Rectf(rect.get_left(), rect.get_top() - 10,
                                             rect.get_right(), rect.get_bottom() + 10),
                                       Color(g_config->menufrontcolor),
                                       g_config->menuroundness,
                                       LAYER_GUI-10);
  }

  bool is_active() const
  {
    return m_is_active;
  }
};

MenuManager::MenuManager() :
  m_dialog(),
  m_has_next_dialog(false),
  m_next_dialog(),
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
  for (const auto& menu : m_menu_stack)
  {
    menu->refresh();
  }
}

void
MenuManager::process_input(const Controller& controller)
{
  if (m_dialog && !m_dialog->is_passive())
  {
    m_dialog->process_input(controller);
  }
  else if (current_menu())
  {
    current_menu()->process_input(controller);
  }
}

void
MenuManager::event(const SDL_Event& ev)
{
  if (!m_transition->is_active())
  {
    if (m_dialog && !m_dialog->is_passive())
    {
      m_dialog->event(ev);
    }
    else if (current_menu())
    {
      // only pass events when the menu is fully visible and not in a
      // transition animation
      current_menu()->event(ev);
    }
  }
}

void
MenuManager::draw(DrawingContext& context)
{
  if (m_has_next_dialog)
  {
    m_dialog = std::move(m_next_dialog);
    m_has_next_dialog = false;
  }

  if (m_transition->is_active())
  {
    m_transition->update();
    m_transition->draw(context);
  }
  else
  {
    if (m_dialog)
    {
      m_dialog->update();
      m_dialog->draw(context);
    }
    if (current_menu() && (!m_dialog || m_dialog->is_passive()))
    {
      // brute force the transition into the right shape in case the
      // menu has changed sizes
      m_transition->set(menu2rect(*current_menu()));
      m_transition->draw(context);

      current_menu()->draw(context);
    }
  }

  if ((m_dialog || current_menu()) && MouseCursor::current())
  {
    MouseCursor::current()->draw(context);
  }
}

void
MenuManager::set_dialog(std::unique_ptr<Dialog> dialog)
{
  // delay reseting m_dialog to a later point, as otherwise the Dialog
  // can't unset itself without ending up with "delete this" problems
  m_next_dialog = std::move(dialog);
  m_has_next_dialog = true;
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
  InputManager::current()->reset();
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
  for (const auto& menu : m_menu_stack)
  {
    menu->on_window_resize();
  }
}

Menu*
MenuManager::current_menu() const
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
