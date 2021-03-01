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

#include "supertux/screen_manager.hpp"

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "editor/particle_editor.hpp"
#include "gui/menu_manager.hpp"
#include "object/player.hpp"
#include "sdk/integration.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "supertux/console.hpp"
#include "supertux/constants.hpp"
#include "supertux/controller_hud.hpp"
#include "supertux/debug.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"

#include <stdio.h>
#include <chrono>


ScreenManager::ScreenManager(VideoSystem& video_system, InputManager& input_manager) :
  m_video_system(video_system),
  m_input_manager(input_manager),
  m_menu_storage(new MenuStorage),
  m_menu_manager(new MenuManager),
  m_controller_hud(new ControllerHUD),
  m_speed(1.0),
  m_actions(),
  m_screen_fade(),
  m_screen_stack()
{
}

ScreenManager::~ScreenManager()
{
}

void
ScreenManager::push_screen(std::unique_ptr<Screen> screen, std::unique_ptr<ScreenFade> screen_fade)
{
  log_debug << "ScreenManager::push_screen(): " << screen.get() << std::endl;
  assert(screen);
  if (g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
  m_actions.emplace_back(Action::PUSH_ACTION, std::move(screen));
}

void
ScreenManager::pop_screen(std::unique_ptr<ScreenFade> screen_fade)
{
  log_debug << "ScreenManager::pop_screen(): stack_size: " << m_screen_stack.size() << std::endl;
  if (g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
  m_actions.emplace_back(Action::POP_ACTION);
}

void
ScreenManager::set_screen_fade(std::unique_ptr<ScreenFade> screen_fade)
{
  if (g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
}

void
ScreenManager::quit(std::unique_ptr<ScreenFade> screen_fade)
{
  if (g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
  m_actions.emplace_back(Action::QUIT_ACTION);
}

void
ScreenManager::set_speed(float speed)
{
  m_speed = speed;
}

float
ScreenManager::get_speed() const
{
  return m_speed;
}

struct ScreenManager::FPS_Stats
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

void
ScreenManager::draw_fps(DrawingContext& context, FPS_Stats& fps_statistics)
{
  // The fonts are not monospace, so the numbers need to be drawn separately
  Vector pos(static_cast<float>(context.get_width()) - BORDER_X, BORDER_Y + 50);
  context.color().draw_text(Resources::small_font, "FPS  min / avg / max",
    pos, ALIGN_RIGHT, LAYER_HUD);
  static const float w2 = Resources::small_font->get_text_width("999.9 /");
  static const float w3 = Resources::small_font->get_text_width("999.9");
  char str1[60];
  char str2[60];
  char str3[60];
  int str_length = sizeof(str1);
  snprintf(str1, str_length, "%3.1f /",
    static_cast<double>(fps_statistics.get_fps_min()));
  snprintf(str2, str_length, "%3.1f /",
    static_cast<double>(fps_statistics.get_fps()));
  snprintf(str3, str_length, "%3.1f",
    static_cast<double>(fps_statistics.get_fps_max()));
  pos.y += 15;
  context.color().draw_text(Resources::small_font, str3,
    pos, ALIGN_RIGHT, LAYER_HUD);
  pos.x -= w3;
  context.color().draw_text(Resources::small_font, str2,
    pos, ALIGN_RIGHT, LAYER_HUD);
  pos.x -= w2;
  context.color().draw_text(Resources::small_font, str1,
    pos, ALIGN_RIGHT, LAYER_HUD);
}

void
ScreenManager::draw_player_pos(DrawingContext& context)
{
  if (auto session = GameSession::current())
  {
    Sector& sector = session->get_current_sector();
    auto pos = sector.get_player().get_pos();
    auto pos_text = "X:" + std::to_string(int(pos.x)) + " Y:" + std::to_string(int(pos.y));

    context.color().draw_text(
      Resources::small_font, pos_text,
      Vector(static_cast<float>(context.get_width()) - Resources::small_font->get_text_width("99999x99999") - BORDER_X,
             BORDER_Y + 60), ALIGN_LEFT, LAYER_HUD);
  }
}

void
ScreenManager::draw(Compositor& compositor, FPS_Stats& fps_statistics)
{
  assert(!m_screen_stack.empty());

  // draw the actual screen
  m_screen_stack.back()->draw(compositor);

  // draw effects and hud
  auto& context = compositor.make_context(true);
  m_menu_manager->draw(context);

  if (m_screen_fade) {
    m_screen_fade->draw(context);
  }

  Console::current()->draw(context);

  if (g_config->show_fps)
    draw_fps(context, fps_statistics);

  if (g_config->show_controller) {
    m_controller_hud->draw(context);
  }

  if (g_config->show_player_pos) {
    draw_player_pos(context);
  }

  // render everything
  compositor.render();
}

void
ScreenManager::update_gamelogic(float dt_sec)
{
  const Controller& controller = m_input_manager.get_controller();

  SquirrelVirtualMachine::current()->update(g_game_time);

  if (!m_screen_stack.empty())
  {
    m_screen_stack.back()->update(dt_sec, controller);
  }

  m_menu_manager->process_input(controller);

  if (m_screen_fade)
  {
    m_screen_fade->update(dt_sec);
  }

  Console::current()->update(dt_sec);
}

void
ScreenManager::process_events()
{
  m_input_manager.update();
  SDL_Event event;
  auto session = GameSession::current();
  while (SDL_PollEvent(&event))
  {
    m_input_manager.process_event(event);

    m_menu_manager->event(event);

    if (Editor::is_active()) {
      Editor::current()->event(event);
    }

    if (ParticleEditor::is_active()) {
      ParticleEditor::current()->event(event);
    }

    switch (event.type)
    {
      case SDL_QUIT:
        quit();
        break;

      case SDL_WINDOWEVENT:
        switch (event.window.event)
        {
          case SDL_WINDOWEVENT_RESIZED:
            m_video_system.on_resize(event.window.data1, event.window.data2);
            m_menu_manager->on_window_resize();
            if (Editor::is_active()) {
              Editor::current()->resize();
            }
            break;

          case SDL_WINDOWEVENT_FOCUS_LOST:
            if (g_config->pause_on_focusloss)
            {
              if (session != nullptr && session->is_active() && !Level::current()->m_suppress_pause_menu)
              {
                session->toggle_pause();
              }
            }
            break;
        }
        break;

      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_F10)
        {
          g_config->show_fps = !g_config->show_fps;
        }
        else if (event.key.keysym.sym == SDLK_F11 ||
                 ((event.key.keysym.mod & KMOD_LALT || event.key.keysym.mod & KMOD_RALT) &&
                 (event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN)))
        {
          g_config->use_fullscreen = !g_config->use_fullscreen;
          m_video_system.apply_config();
          m_menu_manager->on_window_resize();
        }
        else if (event.key.keysym.sym == SDLK_PRINTSCREEN ||
                 event.key.keysym.sym == SDLK_F12)
        {
          m_video_system.do_take_screenshot();
        }
        else if (event.key.keysym.sym == SDLK_F2 &&
                 event.key.keysym.mod & KMOD_CTRL)
        {
          g_config->developer_mode = !g_config->developer_mode;
          log_info << "developer mode: " << g_config->developer_mode << std::endl;
          
          // Keep that line disabled; changing dev mode during a session
          // shouldn't change that setting.
          // It should only take what value it had when the game was launched
          // and keep it until the user changes it manually.
          // If you uncomment, add this in the includes :
          //     #include "editor/overlay_widget.hpp"
          
          // EditorOverlayWidget::autotile_help = !developer_mode;
        }
        break;
    }
  }
}

bool
ScreenManager::has_pending_fadeout() const
{
  return m_screen_fade && !m_screen_fade->done();
}

void
ScreenManager::handle_screen_switch()
{
  if (has_pending_fadeout())
  {
    // wait till the fadeout is completed before switching screens
  }
  else
  {
    m_screen_fade.reset();

    // Screen::setup() might push more screens, so loop till everything is done
    while (!m_actions.empty())
    {
      // keep track of the current screen, as only that needs a call to Screen::leave()
      auto current_screen = m_screen_stack.empty() ? nullptr : m_screen_stack.back().get();

      // move actions to a new vector since setup() might modify it
      auto actions = std::move(m_actions);
      bool quit_action_triggered = false;

      for (auto& action : actions)
      {
        switch (action.type)
        {
          case Action::POP_ACTION:
            assert(!m_screen_stack.empty());
            if (current_screen == m_screen_stack.back().get())
            {
              m_screen_stack.back()->leave();
              current_screen = nullptr;
            }
            m_screen_stack.pop_back();
            break;

          case Action::PUSH_ACTION:
            assert(action.screen);
            m_screen_stack.push_back(std::move(action.screen));
            break;

          case Action::QUIT_ACTION:
            m_screen_stack.clear();
            current_screen = nullptr;
            quit_action_triggered = true;
            break;
        }
      }

      if (!quit_action_triggered)
      {
        if (current_screen != m_screen_stack.back().get())
        {
          g_debug.set_game_speed_multiplier(1.f);

          if (current_screen != nullptr)
          {
            current_screen->leave();
          }

          if (!m_screen_stack.empty())
          {
            m_screen_stack.back()->setup();
            m_speed = 1.0;
            SquirrelVirtualMachine::current()->wakeup_screenswitch();
          }
        }
      }
    }
  }
}

void
ScreenManager::run()
{
  Uint32 last_ticks = 0;
  Uint32 elapsed_ticks = 0;
  const Uint32 ms_per_step = static_cast<Uint32>(1000.0f / LOGICAL_FPS);
  const float seconds_per_step = static_cast<float>(ms_per_step) / 1000.0f;
  FPS_Stats fps_statistics;

  Integration::init_all();

  handle_screen_switch();
  while (!m_screen_stack.empty()) {

    // Useful if screens edit their status without switching screens
    Integration::update_status_all(m_screen_stack.back()->get_status());
    Integration::update_all();

    Uint32 ticks = SDL_GetTicks();
    elapsed_ticks += ticks - last_ticks;
    last_ticks = ticks;

    if (elapsed_ticks > ms_per_step * 8) {
      // when the game loads up or levels are switched the
      // elapsed_ticks grows extremely large, so we just ignore those
      // large time jumps
      elapsed_ticks = 0;
    }

    if (elapsed_ticks < ms_per_step && !g_debug.draw_redundant_frames) {
      // Sleep a bit because not enough time has passed since the previous
      // logical game step
      SDL_Delay(ms_per_step - elapsed_ticks);
      continue;
    }

    g_real_time = static_cast<float>(ticks) / 1000.0f;

    float speed_multiplier = g_debug.get_game_speed_multiplier();
    int steps = elapsed_ticks / ms_per_step;

    // Do not calculate more than a few steps at once
    // The maximum number of steps executed before drawing a frame is
    // adjusted to the current average frame rate
    float fps = fps_statistics.get_fps();
    if (fps != 0) {
      // Skip if fps not ready yet (during first 0.5 seconds of startup).
      float seconds_per_frame = 1.0f / fps_statistics.get_fps();
      int max_steps_per_frame = static_cast<int>(
        ceilf(seconds_per_frame / seconds_per_step));
      if (max_steps_per_frame < 2)
        // max_steps_per_frame is very negative when the fps value is zero
        // Furthermore, the game should always be able to execute
        // up to two steps before drawing a frame
        max_steps_per_frame = 2;
      if (max_steps_per_frame > 4)
        // When the game is very laggy, it should slow down instead of
        // calculating lots of steps at once so that the player can still
        // control Tux reasonably;
        // four steps per frame approximately corresponds to a 16 FPS gameplay
        max_steps_per_frame = 4;
      steps = std::min<int>(steps, max_steps_per_frame);
    }

    for (int i = 0; i < steps; ++i) {
      // Perform a logical game step; seconds_per_step is set to a fixed value
      // so that the game is deterministic.
      // In cases which don't affect regular gameplay, such as the
      // end sequence and debugging, dtime can be changed.
      float dtime = seconds_per_step * m_speed * speed_multiplier;
      g_game_time += dtime;
      process_events();
      update_gamelogic(dtime);
      elapsed_ticks -= ms_per_step;
    }

    if ((steps > 0 && !m_screen_stack.empty())
        || g_debug.draw_redundant_frames) {
      // Draw a frame
      Compositor compositor(m_video_system);
      draw(compositor, fps_statistics);
      fps_statistics.report_frame();
    }

    SoundManager::current()->update();

    handle_screen_switch();
  }

  Integration::close_all();
}

/* EOF */
