//  SuperTux
//  Copyright (C) 2017 christ2go <christian@hagemeier.ch>
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

#ifndef HEADER_SUPERTUX_GUI_ITEM_KEYVALUEFIELD_HPP
#define HEADER_SUPERTUX_GUI_ITEM_KEYVALUEFIELD_HPP

#include "gui/menu_item.hpp"
#include "supertux/colorscheme.hpp"

class KeyValueLabel : public MenuItem {
private:
  const std::string value;

  KeyValueLabel(const KeyValueLabel &);
  KeyValueLabel &operator=(const KeyValueLabel &);

public:
  KeyValueLabel(const std::string &key_, const std::string &value_)
      : MenuItem(key_), value(value_) {}

  /** Draws the menu item. */
  virtual void draw(DrawingContext &, const Vector & pos, int menu_width, bool active);

  /** Returns the minimum width of the menu item. */
  virtual int get_width() const;

  /** Processes the menu action. */
  virtual bool changes_width() const { return true; }
  virtual bool skippable() const { return true; }
  virtual Color get_color() const { return ColorScheme::Menu::label_color; }
  virtual void event(const SDL_Event &ev);
  virtual void process_action(const MenuAction &action) {}

  ~KeyValueLabel() {}
};

#endif // HEADER_SUPERTUX_GUI_ITEM_INTFIELD_HPP

/* EOF */
