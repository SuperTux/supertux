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
#include <cctype>
#include <iostream>
#include <exception>

#include "utils/exceptions.h"
#include "defines.h"
#include "app/globals.h"
#include "app/setup.h"
#include "intro.h"
#include "title.h"
#include "gameloop.h"
#include "leveleditor.h"
#include "video/screen.h"
#include "worldmap.h"
#include "resources.h"
#include "video/surface.h"
#include "tile_manager.h"
#include "app/gettext.h"
#include "player.h"
#include "misc.h"
#include "utils/configfile.h"

int main(int argc, char * argv[])
{
#ifndef DEBUG
  try {
#endif
    config = new MyConfig;
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
    bind_textdomain_codeset(PACKAGE, "ISO-8859-1");
    
    Setup::info(PACKAGE_NAME,PACKAGE,PACKAGE_VERSION);  
    
    Setup::directories();
    Setup::parseargs(argc, argv);

    Setup::audio();
    Setup::video(SCREEN_W,SCREEN_H);
    Setup::joystick();
    Setup::general();
    st_menu();
    loadshared();

    if (launch_leveleditor_mode)
    {
      LevelEditor leveleditor;

      if(level_startup_file)
        leveleditor.run(level_startup_file);
      else
        leveleditor.run();
    }
    else if (launch_worldmap_mode && level_startup_file)
    {
      // hack to make it possible for someone to give an absolute path
      std::string str(level_startup_file);
      unsigned int i = str.find_last_of("/", str.size());
      if(i != std::string::npos)
        str.erase(0, i+1);

      WorldMapNS::WorldMap worldmap;
      worldmap.loadmap(str);
      worldmap.display();
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
    //SDL_Flip(screen);

    unloadshared();
    Setup::general_free();
    st_menu_free();
    TileManager::destroy_instance();
    #ifdef DEBUG
    Surface::debug_check();
    #endif
    Termination::shutdown();
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
