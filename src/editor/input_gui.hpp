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

#ifndef HEADER_SUPERTUX_EDITOR_INPUT_GUI_HPP
#define HEADER_SUPERTUX_EDITOR_INPUT_GUI_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "supertux/screen.hpp"

class SDL_event;
class TileSet;
class Vector;

class EditorInputGui
{
  public:
    EditorInputGui();
    ~EditorInputGui();

    void draw(DrawingContext&);
    void update(float elapsed_time);
    void event(SDL_Event& ev);
    void setup();

    int tile;

    typedef enum {
      IP_NONE, IP_TILE, IP_OBJECT
    }InputType;
    InputType input_type;

    std::vector<int> active_tilegroup;

  private:
    typedef enum {
      HI_NONE, HI_TILEGROUP, HI_OBJECTS, HI_TILE
    }HoveredItem;

    HoveredItem hovered_item;
    int hovered_tile;

    int Xpos;
    const int Ypos = 44;

    Vector get_tile_coords(const int pos);
    int get_tile_pos(const Vector coords);

    EditorInputGui(const EditorInputGui&);
    EditorInputGui& operator=(const EditorInputGui&);
};

#endif // HEADER_SUPERTUX_EDITOR_INPUT_GUI_HPP
