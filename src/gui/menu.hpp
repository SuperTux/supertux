//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_GUI_MENU_HPP
#define HEADER_SUPERTUX_GUI_MENU_HPP

#include <list>
#include <memory>
#include <SDL.h>

#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;
class MenuItem;

class Menu
{
private:
  /* Action done on the menu */
  enum MenuAction {
    MENU_ACTION_NONE = -1,
    MENU_ACTION_UP,
    MENU_ACTION_DOWN,
    MENU_ACTION_LEFT,
    MENU_ACTION_RIGHT,
    MENU_ACTION_HIT,
    MENU_ACTION_INPUT,
    MENU_ACTION_REMOVE,
    MENU_ACTION_BACK
  };

public:
  Menu();
  virtual ~Menu();

  MenuItem* add_hl();
  MenuItem* add_label(const std::string& text);
  MenuItem* add_entry(int id, const std::string& text);
  MenuItem* add_toggle(int id, const std::string& text, bool toggled = false);
  MenuItem* add_inactive(int id, const std::string& text);
  MenuItem* add_back(const std::string& text);
  MenuItem* add_submenu(const std::string& text, int submenu);
  MenuItem* add_controlfield(int id, const std::string& text,
                             const std::string& mapping = "");
  MenuItem* add_string_select(int id, const std::string& text);

  virtual void menu_action(MenuItem* item) = 0;

  void process_input();

  /** Perform actions to bring the menu up to date with configuration changes */
  virtual void refresh() {}

  /** Remove all entries from the menu */
  void clear();

  MenuItem& get_item(int index)
  {
    return *(items[index]);
  }

  MenuItem& get_item_by_id(int id);
  const MenuItem& get_item_by_id(int id) const;

  int get_active_item_id();
  void set_active_item(int id);

  void draw(DrawingContext& context);
  Vector get_center_pos() const { return pos; }
  void set_center_pos(float x, float y);

  void event(const SDL_Event& event);

  bool is_toggled(int id) const;
  void set_toggled(int id, bool toggled);

  float get_width() const;
  float get_height() const;

  virtual void on_window_resize();

protected:
  MenuItem* add_item(std::unique_ptr<MenuItem> menu_item);

private:
  void process_action(MenuAction menuaction);
  void check_controlfield_change_event(const SDL_Event& event);
  void draw_item(DrawingContext& context, int index);

private:
  // position of the menu (ie. center of the menu, not top/left)
  Vector pos;

  /* input implementation variables */
  int   delete_character;
  char  mn_input_char;
  float menu_repeat_time;

public:
  std::vector<std::unique_ptr<MenuItem> > items;

private:
  int arrange_left;
  int active_item;
};

#endif

/* EOF */
