//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include <SDL.h>

#include "supertux/main.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/video_system.hpp"
#endif

int main(int argc, char** argv)
{
#ifdef __EMSCRIPTEN__
  EM_ASM(
    if (window.supertux_onready)
      window.supertux_onready();
  );
#endif
  return (new Main())->run(argc, argv);
}

// Export functions for emscripten
#ifdef __EMSCRIPTEN__

extern "C" {

EMSCRIPTEN_KEEPALIVE // This is probably not useful, I just want ppl to know it exists
void set_resolution(int w, int h)
{
  VideoSystem::current()->on_resize(w, h);
  MenuManager::instance().on_window_resize();
}

EMSCRIPTEN_KEEPALIVE // Same as above
void save_config()
{
  g_config->save();
}

}

#endif

/* EOF */
