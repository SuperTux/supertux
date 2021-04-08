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

#ifndef HEADER_SUPERTUX_SUPERTUX_SCREEN_MANAGER_HPP
#define HEADER_SUPERTUX_SUPERTUX_SCREEN_MANAGER_HPP

#include <memory>

#include "config.h"

#include "control/input_manager.hpp"
#include "control/mobile_controller.hpp"
#include "squirrel/squirrel_thread_queue.hpp"
#include "supertux/screen.hpp"
#include "util/currenton.hpp"
#include "video/video_system.hpp"

#include <assert.h>
#include <SDL2/SDL.h>
#include <chrono>

class Compositor;
class ControllerHUD;
class DrawingContext;
class MenuManager;
class MenuStorage;
class ScreenFade;

/**
 * Manages, updates and draws all Screens, Controllers, Menus and the Console.
 */
class ScreenManager final : public Currenton<ScreenManager>
{
public:
  ScreenManager(std::unique_ptr<VideoSystem> video_system, InputManager& input_manager);
  ~ScreenManager();

  void run();
  void quit(std::unique_ptr<ScreenFade> fade = {});
  void set_speed(float speed);
  float get_speed() const;
  bool has_pending_fadeout() const;

  // push new screen on screen_stack
  void push_screen(std::unique_ptr<Screen> screen, std::unique_ptr<ScreenFade> fade = {});
  void pop_screen(std::unique_ptr<ScreenFade> fade = {});
  void set_screen_fade(std::unique_ptr<ScreenFade> fade);

  void loop_iter();

  std::vector<std::unique_ptr<Screen> > m_screen_stack;

private:
struct FPS_Stats;
  void draw_fps(DrawingContext& context, FPS_Stats& fps_statistics);
  void draw_player_pos(DrawingContext& context);
  void draw(Compositor& compositor, FPS_Stats& fps_statistics);
  void update_gamelogic(float dt_sec);
  void process_events();
  void handle_screen_switch();

private:
  std::unique_ptr<VideoSystem> m_video_system;
  InputManager m_input_manager;
  std::unique_ptr<MenuStorage> m_menu_storage;
  std::unique_ptr<MenuManager> m_menu_manager;
  std::unique_ptr<ControllerHUD> m_controller_hud;
#ifdef ENABLE_TOUCHSCREEN_SUPPORT
  MobileController m_mobile_controller;
#endif

  Uint32 last_ticks;
  Uint32 elapsed_ticks;
  const Uint32 ms_per_step;
  const float seconds_per_step;
  FPS_Stats fps_statistics;

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
  };

  std::vector<Action> m_actions;

  std::unique_ptr<ScreenFade> m_screen_fade;
};

#endif

/* EOF */
