//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmail.com>
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
#include "supertux/screen.hpp"
#include "util/currenton.hpp"

class Console;
class DrawingContext;
class MenuManager;
class MenuStorage;
class Screen;
class ScreenFade;

/**
 * Manages, updates and draws all Screens, Controllers, Menus and the Console.
 */
class ScreenManager : public Currenton<ScreenManager>
{
public:
  ScreenManager();
  ~ScreenManager();

  void run(DrawingContext &context);
  void quit(std::unique_ptr<ScreenFade> fade = {});
  void set_speed(float speed);
  float get_speed() const;
  bool has_pending_fadeout() const;

  /**
   * requests that a screenshot be taken after the next frame has been rendered
   */
  void take_screenshot();

  // push new screen on screen_stack
  void push_screen(std::unique_ptr<Screen> screen, std::unique_ptr<ScreenFade> fade = {});
  void pop_screen(std::unique_ptr<ScreenFade> fade = {});
  void set_screen_fade(std::unique_ptr<ScreenFade> fade);

  /// threads that wait for a screenswitch
  scripting::ThreadQueue m_waiting_threads;

private:
  void draw_fps(DrawingContext& context, float fps);
  void draw_player_pos(DrawingContext& context);
  void draw(DrawingContext& context);
  void update_gamelogic(float elapsed_time);
  void process_events();
  void handle_screen_switch();

private:
  std::unique_ptr<MenuStorage> m_menu_storage;
  std::unique_ptr<MenuManager> m_menu_manager;

  float m_speed;
  struct Action
  {
    enum Type { PUSH_ACTION, POP_ACTION, QUIT_ACTION };
    Type type;
    std::unique_ptr<Screen> screen;

    Action(Type type_,
           std::unique_ptr<Screen> screen_ = {}) :
      type(type_),
      screen(std::move(screen_))
    {}
#ifdef WIN32
    Action(Action &a) :
      type(a.type),
      screen(std::move(a.screen))
    {}
#endif
    Action(Action &&a) :
      type(a.type),
      screen(std::move(a.screen))
    {}

#ifdef WIN32
    Action& operator=(Action &a)
    {
      type = a.type;
      screen = std::move(a.screen);
      return *this;
    }
#endif
    Action& operator=(Action &&a)
    {
      type = a.type;
      screen = std::move(a.screen);
      return *this;
    }
  };

  std::vector<Action> m_actions;

  /// measured fps
  float m_fps;
  std::unique_ptr<ScreenFade> m_screen_fade;
  std::vector<std::unique_ptr<Screen> > m_screen_stack;
  bool m_screenshot_requested; /**< true if a screenshot should be taken after the next frame has been rendered */
};

#endif

/* EOF */
