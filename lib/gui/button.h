/***************************************************************************
                          button.h  -  graphical buttons
                             -------------------
    begin                : June, 23 2004
    copyright            : (C) 2004 by Ricardo Cruz
    email                : rick2@aeiou.pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SUPERTUX_BUTTON_H
#define SUPERTUX_BUTTON_H

#include <vector>
#include <string>

#include "math/vector.h"
#include "video/drawing_context.h"

namespace SuperTux
  {
class Surface;
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

} //namespace SuperTux

#endif
