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

#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu.hpp"
#include "supertux/globals.hpp"
#include "supertux/timer.hpp"

std::vector<Menu*> MenuManager2::last_menus;
std::list<Menu*> MenuManager2::all_menus;
Menu* MenuManager2::current_ = 0;
Menu* MenuManager2::previous = 0;

void
MenuManager2::push_current(Menu* pmenu)
{
  previous = current_;

  if (current_)
    last_menus.push_back(current_);

  current_ = pmenu;
  current_->effect_start_time = real_time;
  current_->effect_progress   = 0.0f;
}

void
MenuManager2::pop_current()
{
  previous = current_;

  if (last_menus.size() >= 1) {
    current_ = last_menus.back();
    current_->effect_start_time = real_time;
    current_->effect_progress   = 0.0f;
    last_menus.pop_back();
  } else {
    set_current(NULL);
  }
}

void
MenuManager2::set_current(Menu* menu)
{
  if (current_ && current_->close == true)
    return;

  previous = current_;

  if (menu) {
    menu->effect_start_time = real_time;
    menu->effect_progress = 0.0f;
    current_ = menu;
  }
  else if (current_) {
    last_menus.clear();                         //NULL new menu pointer => close all menus
    current_->effect_start_time = real_time;
    current_->effect_progress = 0.0f;
    current_->close = true;
  }

  // just to be sure...
  g_main_controller->reset();
}

void
MenuManager2::recalc_pos()
{
  if (current_)
    current_->set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

  for(std::list<Menu*>::iterator i = all_menus.begin(); i != all_menus.end(); ++i)
  {
    // FIXME: This is of course not quite right, since it ignores any previous set_pos() calls
    (*i)->set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
  }
}

/* EOF */
