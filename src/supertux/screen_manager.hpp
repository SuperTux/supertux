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

#ifndef HEADER_SUPERTUX_SUPERTUX_MAINLOOP_HPP
#define HEADER_SUPERTUX_SUPERTUX_MAINLOOP_HPP

#include <memory>
#include <cstddef>

#include "scripting/thread_queue.hpp"

class Console;
class DrawingContext;
class MenuManager;
class MenuStorage;
class Screen;
class ScreenFade;

/**
 * Manages, updates and draws all Screens, Controllers, Menus and the Console.
 */
class ScreenManager
{
public:
  ScreenManager();
  ~ScreenManager();

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
  scripting::ThreadQueue waiting_threads;

private:
  void draw_fps(DrawingContext& context, float fps);
  void draw(DrawingContext& context);
  void update_gamelogic(float elapsed_time);
  void process_events();
  void handle_screen_switch();

private:
  std::unique_ptr<MenuStorage> m_menu_storage;
  std::unique_ptr<MenuManager> m_menu_manager;
  bool running;
  float speed;
  bool nextpop;
  bool nextpush;
  /// measured fps
  float fps;
  std::unique_ptr<Screen> next_screen;
  std::unique_ptr<Screen> current_screen;
  std::unique_ptr<Console> console;
  std::unique_ptr<ScreenFade> screen_fade;
  std::vector<Screen*> screen_stack;
  bool screenshot_requested; /**< true if a screenshot should be taken after the next frame has been rendered */
};

#endif

/* EOF */
