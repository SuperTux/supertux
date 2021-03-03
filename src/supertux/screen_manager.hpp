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

#include "squirrel/squirrel_thread_queue.hpp"
#include "supertux/screen.hpp"
#include "util/currenton.hpp"

#include <assert.h>
#include <SDL2/SDL.h>
#include <chrono>

class Compositor;
class ControllerHUD;
class DrawingContext;
class InputManager;
class MenuManager;
class MenuStorage;
class ScreenFade;
class VideoSystem;

/**
 * Manages, updates and draws all Screens, Controllers, Menus and the Console.
 */
class ScreenManager final : public Currenton<ScreenManager>
{
public:
  ScreenManager(VideoSystem& video_system, InputManager& input_manager);
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
	struct FPS_Stats
	{
	  FPS_Stats():
	    measurements_cnt(0),
	    acc_us(0),
	    min_us(1000000),
	    max_us(0),
	    last_fps(0),
	    last_fps_min(0),
	    last_fps_max(0),
	    // Use chrono instead of SDL_GetTicks for more precise FPS measurement
	    time_prev(std::chrono::steady_clock::now())
	  {
	  }

	  void report_frame()
	  {
	    auto time_now = std::chrono::steady_clock::now();
	    int dtime_us = static_cast<int>(std::chrono::duration_cast<
	      std::chrono::microseconds>(time_now - time_prev).count());
	    assert(dtime_us >= 0);  // Steady clock.
	    if (dtime_us == 0)
	      return;
	    time_prev = time_now;

	    acc_us += dtime_us;
	    ++measurements_cnt;
	    if (min_us > dtime_us)
	      min_us = dtime_us;
	    if (max_us < dtime_us)
	      max_us = dtime_us;

	    float expired_seconds = static_cast<float>(acc_us) / 1000000.0f;
	    if (expired_seconds < 0.5f)
	      return;
	    // Update values to be printed every 0.5 s
	    assert(measurements_cnt > 0);  // ++measurements_cnt above.
	    last_fps = static_cast<float>(measurements_cnt) / expired_seconds;
	    assert(last_fps > 0);  // measurements_cnt > 0 and expired_seconds >= 0.5f.
	    assert(max_us > 0);  // dtime_us > 0.
	    last_fps_min = 1000000.0f / static_cast<float>(max_us);
	    assert(last_fps_min > 0);  // max_us > 0.
	    assert(min_us > 0);  // initialization to 1000000 and dtime_us > 0.
	    last_fps_max = 1000000.0f / static_cast<float>(min_us);
	    assert(last_fps_max > 0);  // min_us > 0.
	    measurements_cnt = 0;
	    acc_us = 0;
	    min_us = 1000000;
	    max_us = 0;
	  }

	  float get_fps() const { return last_fps; }
	  float get_fps_min() const { return last_fps_min; }
	  float get_fps_max() const { return last_fps_max; }

	  // This returns the highest measured delay between two frames from the
	  // previous and current 0.5 s measuring intervals
	  float get_highest_max_ms() const
	  {
	    float previous_max_ms = 1000.0f / last_fps_min;
	    if (measurements_cnt > 0) {
	      float current_max_ms = static_cast<float>(max_us) / 1000.0f;
	      return std::max<float>(previous_max_ms, current_max_ms);
	    }
	    return previous_max_ms;
	  }

	private:
	  int measurements_cnt;
	  int acc_us;
	  int min_us;
	  int max_us;
	  float last_fps;
	  float last_fps_min;
	  float last_fps_max;
	  std::chrono::steady_clock::time_point time_prev;
	};

  void draw_fps(DrawingContext& context, FPS_Stats& fps_statistics);
  void draw_player_pos(DrawingContext& context);
  void draw(Compositor& compositor, FPS_Stats& fps_statistics);
  void update_gamelogic(float dt_sec);
  void process_events();
  void handle_screen_switch();

private:
  VideoSystem& m_video_system;
  InputManager& m_input_manager;
  std::unique_ptr<MenuStorage> m_menu_storage;
  std::unique_ptr<MenuManager> m_menu_manager;
  std::unique_ptr<ControllerHUD> m_controller_hud;

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
