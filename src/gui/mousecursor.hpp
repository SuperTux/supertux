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

#ifndef HEADER_SUPERTUX_GUI_MOUSECURSOR_HPP
#define HEADER_SUPERTUX_GUI_MOUSECURSOR_HPP

#include <string>
#include <vector>

#include "video/surface_ptr.hpp"

#define MC_STATES_NB 3

enum MouseCursorState
{
  MC_NORMAL = 0,
  MC_CLICK,
  MC_LINK,
  MC_HIDE
};

class DrawingContext;

/// Mouse cursor.
/** Used to create mouse cursors.
    The mouse cursors can be animated
    and can be used in four different states.
    (MC_NORMAL, MC_CLICK, MC_LINK or MC_HIDE) */
class MouseCursor
{
public:
  MouseCursor(const std::string& cursor_file,
              const std::string& cursor_click_file,
              const std::string& cursor_link_file);
  ~MouseCursor();

  /// Set MouseCursor state.
  /** (MC_NORMAL, MC_CLICK, MC_LINK or MC_HIDE) */
  void set_state(MouseCursorState nstate);

  /// Define the middle of a MouseCursor.
  /** Useful for cross mouse cursor images in example. */
  void set_mid(int x, int y);

  /// Draw MouseCursor on screen.
  void draw(DrawingContext& context);

  /// Return the current cursor.
  static MouseCursor* current()
  {        return current_;      };
  /// Set current cursor.
  static void set_current(MouseCursor* pcursor)
  {        current_ = pcursor;      };

private:
  int m_mid_x;
  int m_mid_y;
  MouseCursorState m_state;
  std::vector<SurfacePtr> m_cursor;

private:
  static MouseCursor* current_;

private:
  MouseCursor(const MouseCursor&);
  MouseCursor& operator=(const MouseCursor&);
};

#endif /*SUPERTUX_MOUSECURSOR_H*/

/* EOF */
