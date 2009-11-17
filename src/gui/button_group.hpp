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

#ifndef HEADER_SUPERTUX_GUI_BUTTON_GROUP_HPP
#define HEADER_SUPERTUX_GUI_BUTTON_GROUP_HPP

#include <SDL.h>
#include <string>
#include <vector>

#include "math/vector.hpp"
#include "gui/button.hpp"

class DrawingContext;

class ButtonGroup
{
public:
  ButtonGroup(Vector pos_, Vector size_, Vector button_box_);
  ~ButtonGroup();

  void draw(DrawingContext& context);
  bool event(SDL_Event& event);

  void add_button(Button button, int id, bool select = false);
  void add_pair_of_buttons(Button button1, int id1, Button button2, int id2);

  int selected_id();
  void set_unselected();
  bool is_hover();

private:
  typedef std::vector <Button> Buttons;

  Vector pos;
  Vector buttons_size;
  Vector buttons_box;
  Buttons buttons;

  int button_selected;
  int row;
  bool mouse_hover;
  bool mouse_left_button;

  int buttons_pair_nb;

private:
  ButtonGroup(const ButtonGroup&);
  ButtonGroup& operator=(const ButtonGroup&);
};

#endif

/* EOF */
