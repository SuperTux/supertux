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

#ifndef SUPERTUX_BUTTON_H
#define SUPERTUX_BUTTON_H

#include <vector>
#include <string>

#include "SDL.h"

#include "math/vector.hpp"

class Surface;
class DrawingContext;
class Font;
class ButtonGroup;

enum {
  BT_NONE,
  BT_HOVER,
  BT_SELECTED,
  BT_SHOW_INFO
  };

class Button
{
public:
  Button(Surface* image_, std::string info_, SDLKey binding_);
  ~Button();

  void draw(DrawingContext& context, bool selected);
  int event(SDL_Event& event, int x_offset = 0, int y_offset = 0);

  static Font* info_font;

private:
  friend class ButtonGroup;

  Vector pos, size;

  Surface* image;
  SDLKey binding;

  int id;
  int state;
  std::string info;
};

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
  Vector pos, buttons_size, buttons_box;
  typedef std::vector <Button> Buttons;
  Buttons buttons;

  int button_selected, row;
  bool mouse_hover, mouse_left_button;

  int buttons_pair_nb;
};

#endif
