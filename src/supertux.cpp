//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <sys/types.h>
#include <ctype.h>
#include <iostream>

#include <exception>
#include "exceptions.h"

#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "intro.h"
#include "title.h"
#include "gameloop.h"
#include "leveleditor.h"
#include "screen/screen.h"
#include "worldmap.h"
#include "resources.h"
#include "screen/texture.h"
#include "tile.h"
#include "gettext.h"

int main(int argc, char * argv[])
{
#ifndef DEBUG
  try {
#endif
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    bind_textdomain_codeset(PACKAGE, "ISO-8859-1");
      
    st_directory_setup();
    parseargs(argc, argv);

    st_audio_setup();
    st_video_setup();
    st_joystick_setup();
    st_general_setup();
    st_menu();
    loadshared();

    if (launch_leveleditor_mode && level_startup_file)
    {
      // TODO
      // leveleditor(level_startup_file);
    }
    else if (level_startup_file)
    {
      GameSession session(level_startup_file, ST_GL_LOAD_LEVEL_FILE);
      session.run();
    }
    else
    {  
      title();
    }

    SDL_FillRect(screen, 0, 0);
    SDL_Flip(screen);

    unloadshared();
    st_general_free();
    TileManager::destroy_instance();
    #ifdef DEBUG
    Surface::debug_check();
    #endif
    st_shutdown();
#ifndef DEBUG  // we want to see the backtrace in gdb when in debug mode
  }
  catch (SuperTuxException &e)
  {
    std::cerr << "Unhandled SuperTux exception:\n  " << e.what_file() << ":" << e.what_line() << ": " << e.what() << std::endl;
  }
  catch (std::exception &e)
  {
    std:: cerr << "Unhandled exception: " << e.what() << std::endl;
  }
#endif

  return 0;
}
