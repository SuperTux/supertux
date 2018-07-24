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
#include "gui/menu_manager.hpp"
#include "scripting/scripting.hpp"
#include "scripting/time_scheduler.hpp"
#include "supertux/console.hpp"
#include "supertux/constants.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_session.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/sector.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"

#include <stdio.h>

/** ticks (as returned from SDL_GetTicks) per frame */
static const Uint32 TICKS_PER_FRAME = (Uint32) (1000.0 / LOGICAL_FPS);
/** don't skip more than every 2nd frame */
static const int MAX_FRAME_SKIP = 2;

ScreenManager::ScreenManager(VideoSystem& video_system) :
  m_waiting_threads(),
  m_video_system(video_system),
  m_menu_storage(new MenuStorage),
  m_menu_manager(new MenuManager),
  m_speed(1.0),
  m_actions(),
  m_fps(0),
  m_screen_fade(),
  m_screen_stack()
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
  log_debug << "ScreenManager::push_screen(): " << screen.get() << std::endl;
  assert(screen);
  if(g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
  m_actions.emplace_back(Action::PUSH_ACTION, std::move(screen));
}

void
ScreenManager::pop_screen(std::unique_ptr<ScreenFade> screen_fade)
{
  log_debug << "ScreenManager::pop_screen(): stack_size: " << m_screen_stack.size() << std::endl;
  if(g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
  m_actions.emplace_back(Action::POP_ACTION);
}

void
ScreenManager::set_screen_fade(std::unique_ptr<ScreenFade> screen_fade)
{
  if(g_config->transitions_enabled)
  {
    m_screen_fade = std::move(screen_fade);
  }
}

void
ScreenManager::quit(std::unique_ptr<ScreenFade> screen_fade)
{
  if(g_config->transitions_enabled)
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

void
ScreenManager::draw_fps(DrawingContext& context, float fps_fps)
{
  char str[60];
  snprintf(str, sizeof(str), "%3.1f", fps_fps);
  const char* fpstext = "FPS";
  context.color().draw_text(Resources::small_font, fpstext,
                    Vector(context.get_width() - Resources::small_font->get_text_width(fpstext) - Resources::small_font->get_text_width(" 99999") - BORDER_X,
                           BORDER_Y + 20), ALIGN_LEFT, LAYER_HUD);
  context.color().draw_text(Resources::small_font, str, Vector(context.get_width() - BORDER_X, BORDER_Y + 20), ALIGN_RIGHT, LAYER_HUD);
}

void
ScreenManager::draw_player_pos(DrawingContext& context)
{
  if (auto session = GameSession::current())
  {
    auto sector = session->get_current_sector();
    if (sector == NULL)
      return;
    auto pos = sector->get_players()[0]->get_pos();
    auto pos_text = "X:" + std::to_string(int(pos.x)) + " Y:" + std::to_string(int(pos.y));

    context.color().draw_text(Resources::small_font, pos_text,
                      Vector(context.get_width() - Resources::small_font->get_text_width("99999x99999") - BORDER_X,
                             BORDER_Y + 40), ALIGN_LEFT, LAYER_HUD);
  }
}

void
ScreenManager::draw(Compositor& compositor)
{
  assert(!m_screen_stack.empty());

  static Uint32 fps_ticks = SDL_GetTicks();

  // draw the actual screen
  m_screen_stack.back()->draw(compositor);

  // draw effects and hud
  auto& context = compositor.make_context();
  m_menu_manager->draw(context);

  if (m_screen_fade)
  {
    m_screen_fade->draw(context);
  }

  Console::current()->draw(context);

  if (g_config->show_fps)
  {
    draw_fps(context, m_fps);
  }

  if (g_config->show_player_pos)
  {
    draw_player_pos(context);
  }

  // render everything
  compositor.render();

  /* Calculate frames per second */
  if (g_config->show_fps)
  {
    static int frame_count = 0;
    ++frame_count;

    if (SDL_GetTicks() - fps_ticks >= 500)
    {
      m_fps = frame_count / .5f;
      frame_count = 0;
      fps_ticks = SDL_GetTicks();
    }
  }
}

void
ScreenManager::update_gamelogic(float elapsed_time)
{
  scripting::Scripting::current()->update_debugger();
  scripting::TimeScheduler::instance->update(game_time);

  if (!m_screen_stack.empty())
  {
    m_screen_stack.back()->update(elapsed_time);
  }

  m_menu_manager->process_input();

  if (m_screen_fade)
  {
    m_screen_fade->update(elapsed_time);
  }

  Console::current()->update(elapsed_time);
}

void
ScreenManager::process_events()
{
  InputManager::current()->update();
  SDL_Event event;
  auto session = GameSession::current();
  while (SDL_PollEvent(&event))
  {
    InputManager::current()->process_event(event);

    m_menu_manager->event(event);

    if (Editor::is_active()) {
      Editor::current()->event(event);
    }

    switch(event.type)
    {
      case SDL_QUIT:
        quit();
        break;

      case SDL_WINDOWEVENT:
        switch(event.window.event)
        {
          case SDL_WINDOWEVENT_RESIZED:
            m_video_system.resize(event.window.data1,
                                           event.window.data2);
            m_menu_manager->on_window_resize();
            if (Editor::is_active()) {
              Editor::current()->resize();
            }
            if(session != NULL)
            {
              session->on_window_resize();
            }
            break;

          case SDL_WINDOWEVENT_FOCUS_LOST:
            if(session != NULL && session->is_active())
            {
              session->toggle_pause();
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

      for(auto& action : actions)
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
          if (current_screen != nullptr)
          {
            current_screen->leave();
          }

          if (!m_screen_stack.empty())
          {
            m_screen_stack.back()->setup();
            m_speed = 1.0;
            m_waiting_threads.wakeup();
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

  handle_screen_switch();

  while (!m_screen_stack.empty())
  {
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

    if (!m_screen_stack.empty())
    {
      Compositor compositor(m_video_system);
      draw(compositor);
    }

    SoundManager::current()->update();

    handle_screen_switch();
  }
}

/* EOF */
