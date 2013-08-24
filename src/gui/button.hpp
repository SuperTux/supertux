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

#ifndef HEADER_SUPERTUX_GUI_BUTTON_HPP
#define HEADER_SUPERTUX_GUI_BUTTON_HPP

#include <SDL.h>
#include <string>

#include "math/vector.hpp"
#include "video/font_ptr.hpp"
#include "video/surface_ptr.hpp"

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
  Button(SurfacePtr image_, std::string info_, SDL_Keycode binding_);
  Button(const Button& rhs);
  ~Button();

  Button& operator=(const Button& rhs);

  void draw(DrawingContext& context, bool selected);
  int event(SDL_Event& event, int x_offset = 0, int y_offset = 0);

  static FontPtr info_font;

private:
  friend class ButtonGroup;

private:
  Vector pos;
  Vector size;

  SurfacePtr image;
  SDL_Keycode binding;

  int id;
  int state;
  std::string info;
};

#endif

/* EOF */
