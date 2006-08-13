//  $Id$
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
#include <config.h>

#include "mainloop.hpp"

#include <stdlib.h>
#include <SDL.h>
#include "video/drawing_context.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu.hpp"
#include "audio/sound_manager.hpp"
#include "scripting/time_scheduler.hpp"
#include "scripting/squirrel_util.hpp"
#include "gameconfig.hpp"
#include "main.hpp"
#include "resources.hpp"
#include "screen.hpp"
#include "screen_fade.hpp"
#include "timer.hpp"
#include "player_status.hpp"
#include "random_generator.hpp"

// the engine will be run with a logical framerate of 64fps.
// We chose 64fps here because it is a power of 2, so 1/64 gives an "even"
// binary fraction...
static const float LOGICAL_FPS = 64.0;

MainLoop* main_loop = NULL;

MainLoop::MainLoop()
  : speed(1.0), nextpop(false), nextpush(false)
{
  using namespace Scripting;
  TimeScheduler::instance = new TimeScheduler();
}

MainLoop::~MainLoop()
{
  using namespace Scripting;
  delete TimeScheduler::instance;
  TimeScheduler::instance = NULL;

  for(std::vector<Screen*>::iterator i = screen_stack.begin();
      i != screen_stack.end(); ++i) {
    delete *i;
  }
}

void
MainLoop::push_screen(Screen* screen, ScreenFade* screen_fade)
{
  this->next_screen.reset(screen);
  this->screen_fade.reset(screen_fade);
  nextpush = !nextpop;
  nextpop = false;
  speed = 1.0;
}

void
MainLoop::exit_screen(ScreenFade* screen_fade)
{
  next_screen.reset(NULL);
  this->screen_fade.reset(screen_fade);
  nextpop = true;
  nextpush = false;
}

void
MainLoop::set_screen_fade(ScreenFade* screen_fade)
{
  this->screen_fade.reset(screen_fade);
}

void
MainLoop::quit(ScreenFade* screen_fade)
{
  for(std::vector<Screen*>::iterator i = screen_stack.begin();
          i != screen_stack.end(); ++i)
    delete *i;
  screen_stack.clear();

  exit_screen(screen_fade);
}

void
MainLoop::set_speed(float speed)
{
  this->speed = speed;
}

void
MainLoop::draw_fps(DrawingContext& context, float fps_fps)
{
  char str[60];
  snprintf(str, sizeof(str), "%3.1f", fps_fps);
  const char* fpstext = "FPS";
  context.draw_text(white_text, fpstext, Vector(SCREEN_WIDTH - white_text->get_text_width(fpstext) - gold_text->get_text_width(" 99999") - BORDER_X, BORDER_Y + 20), LEFT_ALLIGN, LAYER_HUD);
  context.draw_text(gold_text, str, Vector(SCREEN_WIDTH - BORDER_X, BORDER_Y + 20), RIGHT_ALLIGN, LAYER_HUD);
}

void
MainLoop::run()
{
  DrawingContext context;

  unsigned int frame_count = 0;
  float fps_fps = 0;
  Uint32 fps_ticks = SDL_GetTicks();
  Uint32 fps_nextframe_ticks = SDL_GetTicks();
  Uint32 ticks;
  bool skipdraw = false;

  running = true;
  while(running) {
    while( (next_screen.get() != NULL || nextpop) &&
            (screen_fade.get() == NULL || screen_fade->done())) {
      if(current_screen.get() != NULL) {
        current_screen->leave();
      }

      if(nextpop) {
        if(screen_stack.empty()) {
          running = false;
          break;
        }
        next_screen.reset(screen_stack.back());
        screen_stack.pop_back();
      }
      if(nextpush && current_screen.get() != NULL) {
        screen_stack.push_back(current_screen.release());
      }

      nextpush = false;
      nextpop = false;
      speed = 1.0;
      if(next_screen.get() != NULL)
        next_screen->setup();
      current_screen.reset(next_screen.release());
      screen_fade.reset(NULL);

      waiting_threads.wakeup();
    }

    if(!running || current_screen.get() == NULL)
      break;

    float elapsed_time = 1.0 / LOGICAL_FPS;
    ticks = SDL_GetTicks();
    if(ticks > fps_nextframe_ticks) {
      if(skipdraw) {
        // already skipped last frame? we have to slow down the game then...
        skipdraw = false;
        fps_nextframe_ticks -= (Uint32) (1000.0 / LOGICAL_FPS);
      } else {
        // don't draw all frames when we're getting too slow
        skipdraw = true;
      }
    } else {
      skipdraw = false;
      while(fps_nextframe_ticks > ticks) {
        /* just wait */
        // If we really have to wait long, then do an imprecise SDL_Delay()
        Uint32 diff = fps_nextframe_ticks - ticks;
        if(diff > 10) {
          SDL_Delay(diff - 2);
        }
        ticks = SDL_GetTicks();
      }
    }
    fps_nextframe_ticks = ticks + (Uint32) (1000.0 / LOGICAL_FPS);

    if(!skipdraw) {
      current_screen->draw(context);
      if(Menu::current() != NULL)
        Menu::current()->draw(context);
      if(screen_fade.get() != NULL)
        screen_fade->draw(context);
      Console::instance->draw(context);

      if(config->show_fps)
        draw_fps(context, fps_fps);

      context.do_drawing();

      /* Calculate frames per second */
      if(config->show_fps)
      {
        ++frame_count;

        if(SDL_GetTicks() - fps_ticks >= 500)
        {
          fps_fps = (float) frame_count / .5;
          frame_count = 0;
          fps_ticks = SDL_GetTicks();
        }
      }
    }

    real_time += elapsed_time;
    elapsed_time *= speed;
    game_time += elapsed_time;

    Scripting::update_debugger();
    Scripting::TimeScheduler::instance->update(game_time);
    current_screen->update(elapsed_time);
    if(screen_fade.get() != NULL)
      screen_fade->update(elapsed_time);
    Console::instance->update(elapsed_time);

    main_controller->update();
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      main_controller->process_event(event);
      if(Menu::current() != NULL)
        Menu::current()->event(event);
      if(event.type == SDL_QUIT)
        quit();
    }

    sound_manager->update();

    //log_info << "== periodic rand() = " << systemRandom.rand() << std::endl;
  }
}
