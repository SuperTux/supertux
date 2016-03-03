//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_GUI_ITEM_STRINGSELECT_HPP
#define HEADER_SUPERTUX_GUI_ITEM_STRINGSELECT_HPP

#include <list>
#include <memory>
#include <SDL.h>

#include "gui/menu_item.hpp"

#include "gui/menu.hpp"

class Color;

class ItemStringSelect : public MenuItem
{
  public:
    ItemStringSelect(const std::string& text_, std::vector<std::string> list_, int* selected_, int _id = -1);

    /** Draws the menu item. */
    virtual void draw(DrawingContext&, Vector pos, int menu_width, bool active);

    /** Returns the minimum width of the menu item. */
    virtual int get_width() const;

    /** Processes the menu action. */
    virtual void process_action(MenuAction action);

    std::vector<std::string> list; // list of values for a STRINGSELECT item
    int* selected; // currently selected item

  private:
    ItemStringSelect(const ItemStringSelect&);
    ItemStringSelect& operator=(const ItemStringSelect&);
};

#endif

/* EOF */
