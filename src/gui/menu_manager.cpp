//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/timer.hpp"
#include "video/drawing_context.hpp"

MenuManager* MenuManager::s_instance = 0;

MenuManager&
MenuManager::instance()
{
  assert(s_instance);
  return *s_instance;
}

MenuManager::MenuManager() :
  m_menu_stack()
{
  s_instance = this;
}

MenuManager::~MenuManager()
{
  s_instance = nullptr;
}

void
MenuManager::draw(DrawingContext& context)
{
  if (current())
  {
    Vector pos = current()->get_pos();
    float menu_width = current()->get_width();
    float menu_height = current()->get_height();

    // draw menu background rectangles
    context.draw_filled_rect(Rectf(Vector(pos.x - menu_width/2-4, pos.y - menu_height/2 - 10-4),
                                   Vector(pos.x + menu_width/2+4, pos.y - menu_height/2 + 10 + menu_height+4)),
                             Color(0.2f, 0.3f, 0.4f, 0.8f),
                             20.0f,
                             LAYER_GUI-10);

    context.draw_filled_rect(Rectf(Vector(pos.x - menu_width/2, pos.y - menu_height/2 - 10),
                                   Vector(pos.x + menu_width/2, pos.y - menu_height/2 + 10 + menu_height)),
                             Color(0.6f, 0.7f, 0.8f, 0.5f),
                             16.0f,
                             LAYER_GUI-10);

    current()->draw(context);

    if (MouseCursor::current())
    {
      MouseCursor::current()->draw(context);
    }
  }

#ifdef GRUMBEL
  if (effect_progress != 1.0f)
  {
    if (close)
    {
      menu_width *= 1.0f - effect_progress;
      menu_height *= 1.0f - effect_progress;
    }
    else if (MenuManager::instance().m_previous)
    {
      menu_width  = (menu_width  * effect_progress) + (MenuManager::instance().m_previous->get_width()  * (1.0f - effect_progress));
      menu_height = (menu_height * effect_progress) + (MenuManager::instance().m_previous->get_height() * (1.0f - effect_progress));
      //std::cout << effect_progress << " " << this << " " << last_menus.back() << std::endl;
    }
    else
    {
      menu_width  *= effect_progress;
      menu_height *= effect_progress;
    }
  }

  //update
  effect_progress = (real_time - effect_start_time) * 6.0f;

  if(effect_progress >= 1.0f) {
    effect_progress = 1.0f;

    if (close) {
      MenuManager::instance().m_current = 0;
      close = false;
    }
  }
  else if (effect_progress <= 0.0f) {
    effect_progress = 0.0f;
  }

  // only pass events in non-anim states
  if(effect_progress != 1.0f)
    return;

#endif
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
MenuManager::push_menu(std::unique_ptr<Menu> menu)
{
  m_menu_stack.push_back(std::move(menu));

  //current()->effect_start_time = real_time;
  //current()->effect_progress = 0.0f;
}

void
MenuManager::pop_menu()
{
  if (!m_menu_stack.empty())
  {
    m_menu_stack.pop_back();
    //current()->effect_start_time = real_time;
    //current()->effect_progress   = 0.0f;
  }
  else
  {
    set_menu(MenuStorage::NO_MENU);
  }
}

void
MenuManager::clear_menu_stack()
{
  set_menu(MenuStorage::NO_MENU);
}

void
MenuManager::set_menu(int id)
{
  set_menu(MenuStorage::instance().create(static_cast<MenuStorage::MenuId>(id)));
}

void
MenuManager::set_menu(std::unique_ptr<Menu> menu)
{
  if (menu)
  {
    m_menu_stack.push_back(std::move(menu));
    //current()->effect_start_time = real_time;
    //current()->effect_progress = 0.0f;
  }
  else
  {
    m_menu_stack.clear();
    //current()->effect_start_time = real_time;
    //current()->effect_progress = 0.0f;
    //current()->close = true;
  }

  // just to be sure...
  g_input_manager->reset();
}

void
MenuManager::recalc_pos()
{
  for(auto i = m_menu_stack.begin(); i != m_menu_stack.end(); ++i)
  {
    // FIXME: This is of course not quite right, since it ignores any previous set_pos() calls
    (*i)->set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
  }
}

/* EOF */
