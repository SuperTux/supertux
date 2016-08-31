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

class Color;
class DrawingContext;
class MenuItem;

#include "gui/menu_action.hpp"

class Menu
{
public:
  Menu();
  virtual ~Menu();

  MenuItem* add_hl();
  MenuItem* add_label(const std::string& text);
  MenuItem* add_entry(int id, const std::string& text);
  MenuItem* add_toggle(int id, const std::string& text, bool* toggled);
  MenuItem* add_inactive(const std::string& text);
  MenuItem* add_back(const std::string& text, int id = -1);
  MenuItem* add_submenu(const std::string& text, int submenu, int id = -1);
  MenuItem* add_controlfield(int id, const std::string& text,
                             const std::string& mapping = "");
  MenuItem* add_string_select(int id, const std::string& text, int* selected, const std::vector<std::string>& strings);
  MenuItem* add_textfield(const std::string& text, std::string* input, int id = -1);
  MenuItem* add_script(const std::string& text, std::string* script, int id = -1);
  MenuItem* add_script_line(std::string* input, int id = -1);
  MenuItem* add_intfield(const std::string& text, int* input, int id = -1);
  MenuItem* add_numfield(const std::string& text, float* input, int id = -1);
  MenuItem* add_badguy_select(const std::string& text, std::vector<std::string>* badguys, int id = -1);
  MenuItem* add_file(const std::string& text, std::string* input, const std::vector<std::string>& extensions, int id = -1);

  MenuItem* add_color(const std::string& text, Color* color, int id = -1);
  MenuItem* add_colordisplay(Color* color, int id = -1);
  MenuItem* add_colorchannel(float* input, Color channel, int id = -1);

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

  int get_active_item_id() const;
  void set_active_item(int id);

  void draw(DrawingContext& context);
  Vector get_center_pos() const { return pos; }
  void set_center_pos(float x, float y);

  void event(const SDL_Event& event);

  float get_width() const;
  float get_height() const;

  virtual void on_window_resize();

protected:
  MenuItem* add_item(std::unique_ptr<MenuItem> menu_item);
  MenuItem* add_item(std::unique_ptr<MenuItem> menu_item, int pos_);
  void delete_item(int pos_);

  ///returns true when the text is more important than action
  virtual bool is_sensitive() const;

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

protected:
  int active_item;
};

#endif

/* EOF */
