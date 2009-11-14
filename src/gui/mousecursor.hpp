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

#ifndef SUPERTUX_MOUSECURSOR_H
#define SUPERTUX_MOUSECURSOR_H

#include <string>

class Surface;

#define MC_STATES_NB 3

enum {
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
  /// Constructor of MouseCursor.
  /** Expects an imagefile for the cursor and  the number of animation frames it contains. */
  MouseCursor(std::string cursor_file);
  ~MouseCursor();
  /// Get MouseCursor state.
  /** (MC_NORMAL, MC_CLICK, MC_LINK or MC_HIDE) */
  int state();
  /// Set MouseCursor state.
  /** (MC_NORMAL, MC_CLICK, MC_LINK or MC_HIDE) */
  void set_state(int nstate);
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
  int mid_x, mid_y;
  static MouseCursor* current_;
  int state_before_click;
  int cur_state;
  Surface* cursor;
};

#endif /*SUPERTUX_MOUSECURSOR_H*/
