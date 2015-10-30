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

#ifndef HEADER_SUPERTUX_GUI_ITEM_COLORCHANEL_HPP
#define HEADER_SUPERTUX_GUI_ITEM_COLORCHANEL_HPP

#include "gui/menu_item.hpp"

#include "supertux/timer.hpp"
#include "video/color.hpp"

//
class ItemColorChanel : public MenuItem
{
  public:
    ItemColorChanel(float* input_, Color chanel_, int id_ = -1);

    /** Draws the menu item. */
    virtual void draw(DrawingContext&, Vector pos, int menu_width, bool active);

    /** Returns the minimum width of the menu item. */
    virtual int get_width() const;

    /** Processes the menu action. */
    virtual void process_action(MenuAction action);

    float* number;

    void change_input(const std::string& input_) {
      text = input_;
    }

    /** Processes the given event. */
    virtual void event(const SDL_Event& ev);

    virtual Color get_color() const;

  private:

    int flickw;
    bool has_coma;
    Color chanel;

    void add_char(char c);
    void remove_char();

    ItemColorChanel(const ItemColorChanel&);
    ItemColorChanel& operator=(const ItemColorChanel&);
};

#endif // HEADER_SUPERTUX_GUI_ITEM_COLORCHANEL_HPP

/* EOF */
