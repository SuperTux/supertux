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

#include "supertux/screen_manager.hpp"

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/time_scheduler.hpp"
#include "supertux/console.hpp"
#include "supertux/constants.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/main.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/player_status.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/timer.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"

#include <stdio.h>
/** ticks (as returned from SDL_GetTicks) per frame */
static const Uint32 TICKS_PER_FRAME = (Uint32) (1000.0 / LOGICAL_FPS);
/** don't skip more than every 2nd frame */
static const int MAX_FRAME_SKIP = 2;

ScreenManager::ScreenManager() :
  m_waiting_threads(),
  m_menu_storage(new MenuStorage),
  m_menu_manager(new MenuManager),
  m_running(),
  m_speed(1.0),
  m_nextpop(false),
  m_nextpush(false),
  m_fps(0),
  m_next_screen(),
  m_current_screen(),
  m_screen_fade(),
  m_screen_stack(),
  m_screenshot_requested(false)
{
  using namespace scripting;
  TimeScheduler::instance = new TimeScheduler();
}

ScreenManager::~ScreenManager()
{
  using namespace scripting;
  delete TimeScheduler::instance;
  TimeScheduler::instance = NULL;
}

void
ScreenManager::push_screen(std::unique_ptr<Screen> screen, std::unique_ptr<ScreenFade> screen_fade)
{
  assert(!m_next_screen);
  m_next_screen = std::move(screen);
  m_screen_fade = std::move(screen_fade);
  m_nextpush = !m_nextpop;
  m_nextpop = false;
  m_speed = 1.0f;
}

void
ScreenManager::exit_screen(std::unique_ptr<ScreenFade> screen_fade)
{
  m_next_screen.reset();
  m_screen_fade = std::move(screen_fade);
  m_nextpop = true;
  m_nextpush = false;
}

void
ScreenManager::set_screen_fade(std::unique_ptr<ScreenFade> screen_fade)
{
  m_screen_fade = std::move(screen_fade);
}

void
ScreenManager::quit(std::unique_ptr<ScreenFade> screen_fade)
{
  m_screen_stack.clear();
  exit_screen(std::move(screen_fade));
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

bool
ScreenManager::has_no_pending_fadeout() const
{
  return !m_screen_fade || m_screen_fade->done();
}

void
ScreenManager::draw_fps(DrawingContext& context, float fps_fps)
{
  char str[60];
  snprintf(str, sizeof(str), "%3.1f", fps_fps);
  const char* fpstext = "FPS";
  context.draw_text(Resources::small_font, fpstext,
                    Vector(SCREEN_WIDTH - Resources::small_font->get_text_width(fpstext) - Resources::small_font->get_text_width(" 99999") - BORDER_X,
                           BORDER_Y + 20), ALIGN_LEFT, LAYER_HUD);
  context.draw_text(Resources::small_font, str, Vector(SCREEN_WIDTH - BORDER_X, BORDER_Y + 20), ALIGN_RIGHT, LAYER_HUD);
}

void
ScreenManager::draw(DrawingContext& context)
{
  static Uint32 fps_ticks = SDL_GetTicks();
  static int frame_count = 0;

  m_current_screen->draw(context);
  m_menu_manager->draw(context);
  if (m_screen_fade)
  {
    m_screen_fade->draw(context);
  }
  Console::instance->draw(context);

  if (g_config->show_fps)
  {
    draw_fps(context, m_fps);
  }

  // if a screenshot was requested, pass request on to drawing_context
  if (m_screenshot_requested)
  {
    context.take_screenshot();
    m_screenshot_requested = false;
  }
  context.do_drawing();

  /* Calculate frames per second */
  if (g_config->show_fps)
  {
    ++frame_count;

    if (SDL_GetTicks() - fps_ticks >= 500)
    {
      m_fps = (float) frame_count / .5;
      frame_count = 0;
      fps_ticks = SDL_GetTicks();
    }
  }
}

void
ScreenManager::update_gamelogic(float elapsed_time)
{
  scripting::update_debugger();
  scripting::TimeScheduler::instance->update(game_time);
  m_current_screen->update(elapsed_time);
  m_menu_manager->process_input();
  if (m_screen_fade)
  {
    m_screen_fade->update(elapsed_time);
  }
  Console::instance->update(elapsed_time);
}

void
ScreenManager::process_events()
{
  g_input_manager->update();
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    g_input_manager->process_event(event);

    m_menu_manager->event(event);

    switch(event.type)
    {
      case SDL_QUIT:
        quit();
        break;

      case SDL_WINDOWEVENT:
        switch(event.window.event)
        {
          case SDL_WINDOWEVENT_RESIZED:
            Renderer::instance()->resize(event.window.data1,
                                         event.window.data2);
            m_menu_manager->on_window_resize();
            break;
        }
        break;

      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_F10)
        {
          g_config->show_fps = !g_config->show_fps;
        }
        else if (event.key.keysym.sym == SDLK_F11)
        {
          g_config->use_fullscreen = !g_config->use_fullscreen;
          Renderer::instance()->apply_config();
          m_menu_manager->on_window_resize();
        }
        else if (event.key.keysym.sym == SDLK_PRINTSCREEN ||
                 event.key.keysym.sym == SDLK_F12)
        {
          take_screenshot();
        }
        else if (event.key.keysym.sym == SDLK_F1 &&
                 event.key.keysym.mod & KMOD_CTRL)
        {
          Console::instance->toggle();
          g_config->console_enabled = true;
          g_config->save();
        }
        break;
    }
  }
}

void
ScreenManager::handle_screen_switch()
{
  while ((m_next_screen || m_nextpop) &&
        has_no_pending_fadeout())
  {
    if (m_current_screen) {
      m_current_screen->leave();
    }

    if (m_nextpop) {
      if (m_screen_stack.empty()) {
        m_running = false;
        break;
      }
      m_next_screen = std::move(m_screen_stack.back());
      m_screen_stack.pop_back();
    }
    if (m_nextpush && m_current_screen) {
      m_screen_stack.push_back(std::move(m_current_screen));
    }

    m_nextpush = false;
    m_nextpop = false;
    m_speed = 1.0;
    m_current_screen = std::move(m_next_screen);
    if (m_current_screen)
    {
      m_current_screen->setup();
    }
    m_screen_fade.reset();

    m_waiting_threads.wakeup();
  }
}

void
ScreenManager::run(DrawingContext &context)
{
  Uint32 last_ticks = 0;
  Uint32 elapsed_ticks = 0;

  m_running = true;
  while (m_running)
  {
    handle_screen_switch();
    if (!m_running || !m_current_screen)
    {
      break;
    }

    Uint32 ticks = SDL_GetTicks();
    elapsed_ticks += ticks - last_ticks;
    last_ticks = ticks;

    Uint32 ticks_per_frame = (Uint32) (TICKS_PER_FRAME * g_game_speed);

    if (elapsed_ticks > ticks_per_frame*4)
    {
      // when the game loads up or levels are switched the
      // elapsed_ticks grows extremely large, so we just ignore those
      // large time jumps
      elapsed_ticks = 0;
    }

    if (elapsed_ticks < ticks_per_frame)
    {
      Uint32 delay_ticks = ticks_per_frame - elapsed_ticks;
      SDL_Delay(delay_ticks);
      last_ticks += delay_ticks;
      elapsed_ticks += delay_ticks;
    }

    int frames = 0;

    while (elapsed_ticks >= ticks_per_frame && frames < MAX_FRAME_SKIP)
    {
      elapsed_ticks -= ticks_per_frame;
      float timestep = 1.0 / LOGICAL_FPS;
      real_time += timestep;
      timestep *= m_speed;
      game_time += timestep;

      process_events();
      update_gamelogic(timestep);
      frames += 1;
    }

    draw(context);

    sound_manager->update();
  }
}

void
ScreenManager::take_screenshot()
{
  m_screenshot_requested = true;
}

/* EOF */
