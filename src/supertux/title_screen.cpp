//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#include "supertux/title_screen.hpp"

#include "audio/sound_manager.hpp"
#include "gui/menu_manager.hpp"
#include "lisp/parser.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/menu/addon_menu.hpp"
#include "supertux/menu/contrib_world_menu.hpp"
#include "supertux/menu/contrib_menu.hpp"
#include "supertux/menu/main_menu.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"

#include <version.h>

TitleScreen::TitleScreen() :
  main_menu(new MainMenu()),
  frame(),
  controller(),
  titlesession()
{
  controller.reset(new CodeController());
  titlesession.reset(new GameSession("levels/misc/menu.stl"));

  Player* player = titlesession->get_current_sector()->player;
  player->set_controller(controller.get());
  player->set_speedlimit(230); //MAX_WALK_XM

  frame = Surface::create("images/engine/menu/frame.png");
}

std::string
TitleScreen::get_level_name(const std::string& filename)
{
  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(filename);

    const lisp::Lisp* level = root->get_lisp("supertux-level");
    if(!level)
      return "";

    std::string name;
    level->get("name", name);
    return name;
  } catch(std::exception& e) {
    log_warning << "Problem getting name of '" << filename << "': "
                << e.what() << std::endl;
    return "";
  }
}

void
TitleScreen::make_tux_jump()
{
  static bool jumpWasReleased = true;
  Sector* sector  = titlesession->get_current_sector();
  Player* tux = sector->player;

  controller->update();
  controller->press(Controller::RIGHT);

  // Check if we should press the jump button
  Rectf lookahead = tux->get_bbox();
  lookahead.p2.x += 96;
  bool pathBlocked = !sector->is_free_of_statics(lookahead);
  if ((pathBlocked && jumpWasReleased) || !tux->on_ground()) {
    controller->press(Controller::JUMP);
    jumpWasReleased = false;
  } else {
    jumpWasReleased = true;
  }

  // Wrap around at the end of the level back to the beginning
  if(sector->get_width() - 320 < tux->get_pos().x) {
    sector->activate("main");
    sector->camera->reset(tux->get_pos());
  }
}

TitleScreen::~TitleScreen()
{
}

void
TitleScreen::setup()
{
  player_status->reset();

  Sector* sector = titlesession->get_current_sector();
  if(Sector::current() != sector) {
    sector->play_music(LEVEL_MUSIC);
    sector->activate(sector->player->get_pos());
  }

  MenuManager::set_current(main_menu.get());
}

void
TitleScreen::leave()
{
  Sector* sector = titlesession->get_current_sector();
  sector->deactivate();
  MenuManager::set_current(NULL);
}

void
TitleScreen::draw(DrawingContext& context)
{
  Sector* sector  = titlesession->get_current_sector();
  sector->draw(context);

  // FIXME: Add something to scale the frame to the resolution of the screen
  context.draw_surface(frame, Vector(0,0),LAYER_FOREGROUND1);

  context.draw_text(Resources::small_font, "SuperTux " PACKAGE_VERSION "\n",
                    Vector(5, SCREEN_HEIGHT - 50), ALIGN_LEFT, LAYER_FOREGROUND1);
  context.draw_text(Resources::small_font,
                    _(
                      "Copyright (c) 2007 SuperTux Devel Team\n"
                      "This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
                      "redistribute it under certain conditions; see the file COPYING for details.\n"
                      ),
                    Vector(5, SCREEN_HEIGHT - 50 + Resources::small_font->get_height() + 5),
                    ALIGN_LEFT, LAYER_FOREGROUND1);
}

void
TitleScreen::update(float elapsed_time)
{
  g_screen_manager->set_speed(0.6f);
  Sector* sector  = titlesession->get_current_sector();
  sector->update(elapsed_time);

  make_tux_jump();

  if (Menu* menu = MenuManager::current())
  {
    menu->check_menu();
  }

  // reopen menu if user closed it (so that the app doesn't close when user
  // accidently hit ESC)
  if(MenuManager::current() == 0 && g_screen_manager->has_no_pending_fadeout()) 
  {
    MenuManager::set_current(main_menu.get());
  }
}

void
TitleScreen::start_game(World* world)
{
  MenuManager::set_current(NULL);

  std::string basename = world->get_basedir();
  basename = basename.substr(0, basename.length()-1);
  std::string worlddirname = FileSystem::basename(basename);
  std::ostringstream stream;
  stream << "profile" << g_config->profile << "/" << worlddirname << ".stsg";
  std::string slotfile = stream.str();

  try 
  {
    world->set_savegame_filename(slotfile);
    world->run();
  } 
  catch(std::exception& e) 
  {
    log_fatal << "Couldn't start world: " << e.what() << std::endl;
  }
}

/* EOF */
