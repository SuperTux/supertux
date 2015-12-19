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

#ifndef HEADER_SUPERTUX_GUI_ITEM_COLORDISPLAY_HPP
#define HEADER_SUPERTUX_GUI_ITEM_COLORDISPLAY_HPP

#include "gui/menu_item.hpp"

#include "supertux/timer.hpp"
#include "video/color.hpp"

//
class ItemColorDisplay : public MenuItem
{
  public:
    ItemColorDisplay(Color* color, int id_ = -1);

    /** Draws the menu item. */
    virtual void draw(DrawingContext&, Vector pos, int menu_width, bool active);

    /** Returns the minimum width of the menu item. */
    virtual int get_width() const;

    virtual bool skippable() const {
      return true;
    }

  private:
    Color old_color;
    Color* new_color;

    ItemColorDisplay(const ItemColorDisplay&);
    ItemColorDisplay& operator=(const ItemColorDisplay&);
};

#endif // HEADER_SUPERTUX_GUI_ITEM_COLORDISPLAY_HPP

/* EOF */
