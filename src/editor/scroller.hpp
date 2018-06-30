//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_SCROLLER_HPP
#define HEADER_SUPERTUX_EDITOR_SCROLLER_HPP

#include "math/vector.hpp"

class DrawingContext;
union SDL_Event;

class EditorScroller
{
  public:
    EditorScroller();

    void draw(DrawingContext&);
    void update(float elapsed_time);
    bool event(SDL_Event& ev);

    static bool rendered;

  private:
    bool scrolling;
    Vector scrolling_vec;
    Vector mouse_pos;

    void draw_arrow(DrawingContext&, const Vector& pos);
    bool can_scroll() const;

    EditorScroller(const EditorScroller&);
    EditorScroller& operator=(const EditorScroller&);
};

#endif // HEADER_SUPERTUX_EDITOR_SCROLLER_HPP

/* EOF */
