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
#include "supertux/globals.hpp"
#include "supertux/timer.hpp"

MenuManager* MenuManager::s_instance = 0;

MenuManager&
MenuManager::instance()
{
  assert(s_instance);
  return *s_instance;
}

MenuManager::MenuManager() :
  m_last_menus(),
  m_all_menus(),
  m_previous(),
  m_current()
{
  s_instance = this;
}

MenuManager::~MenuManager()
{
  s_instance = nullptr;
}

void
MenuManager::push_current(Menu* menu)
{
  m_previous = m_current;

  if (m_current)
  {
    m_last_menus.push_back(m_current);
  }

  m_current = menu;
  m_current->effect_start_time = real_time;
  m_current->effect_progress = 0.0f;
}

void
MenuManager::pop_current()
{
  m_previous = m_current;

  if (m_last_menus.size() >= 1)
  {
    m_current = m_last_menus.back();
    m_current->effect_start_time = real_time;
    m_current->effect_progress   = 0.0f;
    m_last_menus.pop_back();
  }
  else
  {
    set_current(nullptr);
  }
}

void
MenuManager::set_current(Menu* menu)
{
  if (m_current && m_current->close == true)
  {
    // do nothing
  }
  else
  {
    m_previous = m_current;

    if (menu)
    {
      menu->effect_start_time = real_time;
      menu->effect_progress = 0.0f;
      m_current = menu;
    }
    else if (m_current)
    {
      m_last_menus.clear();                         //NULL new menu pointer => close all menus
      m_current->effect_start_time = real_time;
      m_current->effect_progress = 0.0f;
      m_current->close = true;
    }

    // just to be sure...
    g_input_manager->reset();
  }
}

void
MenuManager::recalc_pos()
{
  if (m_current)
    m_current->set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

  for(auto i = m_all_menus.begin(); i != m_all_menus.end(); ++i)
  {
    // FIXME: This is of course not quite right, since it ignores any previous set_pos() calls
    (*i)->set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
  }
}

/* EOF */
