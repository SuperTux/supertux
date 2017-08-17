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
#include "math/vector.hpp"
#include "supertux/screen.hpp"

class SDL_event;
class TileSet;
class ObjectInput;
class TileSelection;
class ToolIcon;
class Rectf;

class EditorInputGui
{
  public:
    EditorInputGui();
    ~EditorInputGui();

    void draw(DrawingContext&);
    void update(float elapsed_time);
    bool event(SDL_Event& ev);
    void setup();
    void resize();
    void reset_pos();
    void update_mouse_icon();

    std::unique_ptr<TileSelection> tiles;
    std::string object;

    typedef enum {
      IP_NONE, IP_TILE, IP_OBJECT
    } InputType;
    InputType input_type;

    std::vector<int> active_tilegroup;
    int active_objectgroup;
    std::unique_ptr<ObjectInput> object_input;

    std::unique_ptr<ToolIcon> rubber;
    std::unique_ptr<ToolIcon> select_mode;
    std::unique_ptr<ToolIcon> move_mode;
    std::unique_ptr<ToolIcon> settings_mode;

  private:

    typedef enum {
      HI_NONE, HI_TILEGROUP, HI_OBJECTS, HI_TILE, HI_TOOL
    } HoveredItem;

    typedef enum {
      TS_NONE, TS_UP, TS_DOWN
    } TileScrolling;

    HoveredItem hovered_item;
    int hovered_tile;
    TileScrolling tile_scrolling;
    bool using_scroll_wheel;
    int wheel_scroll_amount;
    int starting_tile;
    bool dragging;
    Vector drag_start;

    int Xpos;
    const int Ypos = 60;

    Vector get_tile_coords(const int pos) const;
    int get_tile_pos(const Vector& coords) const;
    Vector get_tool_coords(const int pos) const;
    int get_tool_pos(const Vector& coords) const;

    void update_selection();
    Rectf normalize_selection() const;
    Rectf selection_draw_rect() const;

    void draw_tilegroup(DrawingContext&);
    void draw_objectgroup(DrawingContext&);

    EditorInputGui(const EditorInputGui&);
    EditorInputGui& operator=(const EditorInputGui&);
};

#endif // HEADER_SUPERTUX_EDITOR_INPUT_GUI_HPP

/* EOF */
