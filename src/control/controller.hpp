//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

class Controller
{
public:
  static const char* controlNames[];

  enum Control {
    LEFT = 0,
    RIGHT,
    UP,
    DOWN,

    JUMP,
    ACTION,

    PAUSE_MENU,
    MENU_SELECT,

    CONSOLE,

    PEEK_LEFT,
    PEEK_RIGHT,
    PEEK_UP,
    PEEK_DOWN,

    CONTROLCOUNT
  };

  Controller();
  virtual ~Controller();

  /** returns true if the control is pressed down */
  bool hold(Control control);
  /** returns true if the control has just been pressed down this frame */
  bool pressed(Control control);
  /** returns true if the control has just been released this frame */
  bool released(Control control);

  virtual void reset();
  virtual void update();

protected:
  /** current control status */
  bool controls[CONTROLCOUNT];
  /** control status at last frame */
  bool oldControls[CONTROLCOUNT];
};

#endif
