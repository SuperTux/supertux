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

class EditorInputCenter
{
  public:
    EditorInputCenter();
    ~EditorInputCenter();

    void event(SDL_Event& ev);
    void draw(DrawingContext&);

    void actualize_pos();

  private:
    Vector hovered_tile;
    Vector sector_pos;
    Vector mouse_pos;

    // sp is sector pos, tp is pos on tilemap.
    Vector tp_to_sp(Vector tp);
    Vector sp_to_tp(Vector sp);
};

#endif // INPUT_CENTER_HPP

/* EOF */
