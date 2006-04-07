//  $Id: worldmap.hpp 2800 2005-10-02 22:57:31Z matzebraun $
// 
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
#ifndef __MAINLOOP_HPP__
#define __MAINLOOP_HPP__

#include <memory>
#include <vector>

class Screen;
class Console;

class MainLoop
{
public:
  MainLoop();
  ~MainLoop();
  
  void run();
  void exit_screen();
  void quit();
  void set_speed(float speed);

  // push new screen on screen_stack
  void push_screen(Screen* screen);

private:
  bool running;
  float speed;
  bool nextpush;
  std::auto_ptr<Screen> next_screen;
  std::auto_ptr<Screen> current_screen;
  std::auto_ptr<Console> console;
  std::vector<Screen*> screen_stack;
};

extern MainLoop* main_loop;

#endif

