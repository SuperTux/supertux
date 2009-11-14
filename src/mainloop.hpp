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
#include "scripting/thread_queue.hpp"

class Screen;
class Console;
class ScreenFade;
class DrawingContext;

/**
 * Manages, updates and draws all Screens, Controllers, Menus and the Console.
 */
class MainLoop
{
public:
  MainLoop();
  ~MainLoop();

  void run(DrawingContext &context);
  void exit_screen(ScreenFade* fade = NULL);
  void quit(ScreenFade* fade = NULL);
  void set_speed(float speed);
  float get_speed() const;
  bool has_no_pending_fadeout() const;

  /**
   * requests that a screenshot be taken after the next frame has been rendered
   */
  void take_screenshot();

  // push new screen on screen_stack
  void push_screen(Screen* screen, ScreenFade* fade = NULL);
  void set_screen_fade(ScreenFade* fade);

  /// threads that wait for a screenswitch
  Scripting::ThreadQueue waiting_threads;

private:
  void draw_fps(DrawingContext& context, float fps);
  void draw(DrawingContext& context);
  void update_gamelogic(float elapsed_time);
  void process_events();
  void handle_screen_switch();

  bool running;
  float speed;
  bool nextpop;
  bool nextpush;
  /// measured fps
  float fps;
  std::auto_ptr<Screen> next_screen;
  std::auto_ptr<Screen> current_screen;
  std::auto_ptr<Console> console;
  std::auto_ptr<ScreenFade> screen_fade;
  std::vector<Screen*> screen_stack;
  bool screenshot_requested; /**< true if a screenshot should be taken after the next frame has been rendered */
};

extern MainLoop* main_loop;

#endif
