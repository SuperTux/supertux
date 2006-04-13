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
#include <config.h>

#include "mainloop.hpp"

#include <stdlib.h>
#include <SDL.h>
#include "video/drawing_context.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu.hpp"
#include "audio/sound_manager.hpp"
#include "gameconfig.hpp"
#include "main.hpp"
#include "resources.hpp"
#include "script_manager.hpp"
#include "screen.hpp"
#include "timer.hpp"

// the engine will be run with a logical framerate of 64fps.
// We chose 64fps here because it is a power of 2, so 1/64 gives an "even"
// binary fraction...
static const float LOGICAL_FPS = 64.0;

MainLoop* main_loop = NULL;

MainLoop::MainLoop()
  : speed(1.0)
{
}

MainLoop::~MainLoop()
{
  for(std::vector<Screen*>::iterator i = screen_stack.begin();
      i != screen_stack.end(); ++i) {
    delete *i;
  }
}

void
MainLoop::push_screen(Screen* screen)
{
  this->next_screen.reset(screen);
  nextpush = true;
  speed = 1.0;
}

void
MainLoop::exit_screen()
{
  if (screen_stack.size() < 1) {
    quit();
    return;
  }
  next_screen.reset(screen_stack.back());
  nextpush = false;
  screen_stack.pop_back();
  speed = 1.0;
}

void
MainLoop::quit()
{
  running = false;
}

void
MainLoop::set_speed(float speed)
{
  this->speed = speed;
}

void
MainLoop::run()
{
  DrawingContext context; 
  
  unsigned int frame_count;
  float fps_fps;
  Uint32 fps_ticks = SDL_GetTicks();
  Uint32 fps_nextframe_ticks = SDL_GetTicks();
  Uint32 ticks;
  bool skipdraw = false;
  
  running = true;
  while(running) {
    if(next_screen.get() != NULL) {
      if(nextpush && current_screen.get() != NULL) {
        current_screen->leave();
        screen_stack.push_back(current_screen.release());
      }
      
      next_screen->setup();
      ScriptManager::instance->fire_wakeup_event(ScriptManager::SCREEN_SWITCHED);
      current_screen.reset(next_screen.release());
      next_screen.reset(NULL);
      nextpush = false;
    }

    if(current_screen.get() == NULL)
        break;
      
    float elapsed_time = 1.0 / LOGICAL_FPS;
    ticks = SDL_GetTicks();
    if(ticks > fps_nextframe_ticks) {
      if(skipdraw == true) {
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
        if(diff > 15) {
          SDL_Delay(diff - 10);
        }
        ticks = SDL_GetTicks();
      }
    }
    fps_nextframe_ticks = ticks + (Uint32) (1000.0 / LOGICAL_FPS);

    if(!skipdraw) {
      current_screen->draw(context);
      if(Menu::current() != NULL)
          Menu::current()->draw(context);
      Console::instance->draw(context);

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

    elapsed_time *= speed;

    game_time += elapsed_time;
    ScriptManager::instance->update();
    current_screen->update(elapsed_time);
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
  }
}

