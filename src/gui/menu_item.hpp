//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_GUI_MENU_ITEM_HPP
#define HEADER_SUPERTUX_GUI_MENU_ITEM_HPP

#include <list>
#include <memory>
#include <SDL.h>

#include "gui/menu.hpp"

class Color;

class MenuItem
{
  public:
    MenuItem(const std::string& text_, int id = -1);
    virtual ~MenuItem();

    virtual void set_help(const std::string& help_text);
    virtual void change_text (const std::string& text);

    /** Draws the menu item. */
    virtual void draw(DrawingContext&, Vector pos, int menu_width, bool active);

    /** Returns true when the menu item has no action and therefore can be skipped.
        Useful for labels and horizontal lines.*/
    virtual bool skippable() const {
      return false;
    }

    /** Returns the minimum width of the menu item. */
    virtual int get_width() const;

    /** Processes the menu action. */
    virtual void process_action(MenuAction action) { }

    /** Processes the given event. */
    virtual void event(const SDL_Event& ev) { }

    virtual Color get_color() const;

    /** Returns true when the memu manager shouldn't do anything else. */
    virtual bool no_other_action() const {
      return false;
    }

    int id; //item ID
    std::string text;
    std::string help;

  private:
    MenuItem(const MenuItem&);
    MenuItem& operator=(const MenuItem&);
};

#endif

#ifdef INCLUDE_MENU_ITEMS
  #include "gui/item_action.hpp"
  #include "gui/item_back.hpp"
  #include "gui/item_badguy_select.hpp"
  #include "gui/item_color.hpp"
  #include "gui/item_colorchannel.hpp"
  #include "gui/item_colordisplay.hpp"
  #include "gui/item_controlfield.hpp"
  #include "gui/item_file.hpp"
  #include "gui/item_goto.hpp"
  #include "gui/item_hl.hpp"
  #include "gui/item_inactive.hpp"
  #include "gui/item_intfield.hpp"
  #include "gui/item_label.hpp"
  #include "gui/item_numfield.hpp"
  #include "gui/item_script.hpp"
  #include "gui/item_script_line.hpp"
  #include "gui/item_stringselect.hpp"
  #include "gui/item_textfield.hpp"
  #include "gui/item_toggle.hpp"
#endif
/* EOF */
