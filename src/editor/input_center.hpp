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

#ifndef HEADER_SUPERTUX_EDITOR_INPUT_CENTER_HPP
#define HEADER_SUPERTUX_EDITOR_INPUT_CENTER_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "math/vector.hpp"
#include "supertux/screen.hpp"

class Rectf;
class MovingObject;
class Tip;

class EditorInputCenter
{
  public:
    EditorInputCenter();
    ~EditorInputCenter();

    void event(SDL_Event& ev);
    void draw(DrawingContext&);
    void update(float elapsed_time);

    void actualize_pos();
    void stop_scrolling();

  private:
    Vector hovered_tile;
    Vector sector_pos;
    Vector mouse_pos;

    bool dragging;
    Vector drag_start;
    MovingObject *dragged_object;
    std::unique_ptr<Tip> object_tip;
    Vector obj_mouse_desync;

    void input_tile(Vector pos);
    void put_tile();
    void draw_rectangle();
    void fill();
    void put_object();

    void rubber_object();
    void rubber_rect();

    void grab_object();
    void move_object();
    void hover_object();
    void set_object();

    void draw_tile_tip(DrawingContext&);

    void process_left_click();
    void process_right_click();

    // sp is sector pos, tp is pos on tilemap.
    Vector tp_to_sp(Vector tp);
    Vector sp_to_tp(Vector sp);

    // in sector position
    Rectf drag_rect();

    // scrolling the level on update
    enum HorizontalScrolling{
      HS_NONE,HS_LEFT,HS_RIGHT
    };

    enum VerticalScrolling{
      VS_NONE,VS_UP,VS_DOWN
    };

    HorizontalScrolling mouse_hs;
    VerticalScrolling mouse_vs;

    void actualize_scrolling();
    void update_scroll();

    EditorInputCenter(const EditorInputCenter&);
    EditorInputCenter& operator=(const EditorInputCenter&);
};

#endif // INPUT_CENTER_HPP

/* EOF */
