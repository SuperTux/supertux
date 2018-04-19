//  SuperTux
//  Copyright (C) 2018 Christian Hagemeier <christian@hagemeier.ch>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_CONFIRM_DIALOG_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_CONFIRM_DIALOG_HPP

#include "gui/menu.hpp"
#include <functional>

class ConfirmDialog : public Menu
{
private:
  std::string m_title;
  std::function<void(void)> m_callback;
public:
  ConfirmDialog(const std::string& title, std::function<void(void)> callback);
  ConfirmDialog(const std::string& title, std::function<void(void)> callback, std::string& yes, std::string& no);
  void menu_action(MenuItem* item) override;
  
private:
  
};

#endif

/* EOF */
