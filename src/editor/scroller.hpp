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

class Editor;
class DrawingContext;
union SDL_Event;

class EditorScroller final
{
public:
  static bool rendered;

public:
  EditorScroller(Editor& editor);

  void draw(DrawingContext&);
  void update(float dt_sec);
  bool event(const SDL_Event& ev);

public:
  void draw_arrow(DrawingContext&, const Vector& pos);
  bool can_scroll() const;

private:
  Editor& m_editor;
  bool m_scrolling;
  Vector m_scrolling_vec;
  Vector m_mouse_pos;

private:
  EditorScroller(const EditorScroller&) = delete;
  EditorScroller& operator=(const EditorScroller&) = delete;
};

#endif

/* EOF */
