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
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "lisp/parser.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"

#include <sstream>
#include <version.h>

TitleScreen::TitleScreen(PlayerStatus* player_status) :
  frame(),
  controller(),
  titlesession(),
  copyright_text()
{
  controller.reset(new CodeController());
  titlesession.reset(new GameSession("levels/misc/menu.stl", player_status));

  Player* player = titlesession->get_current_sector()->player;
  player->set_controller(controller.get());
  player->set_speedlimit(230); //MAX_WALK_XM

  frame = Surface::create("images/engine/menu/frame.png");
  copyright_text = "SuperTux " PACKAGE_VERSION "\n" +
    _("Copyright") + " (c) 2003-2014 SuperTux Devel Team\n" +
    _("This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
      "redistribute it under certain conditions; see the file COPYING for details.\n"
   );
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
  Sector* sector = titlesession->get_current_sector();
  if(Sector::current() != sector) {
    sector->play_music(LEVEL_MUSIC);
    sector->activate(sector->player->get_pos());
  }

  MenuManager::instance().set_menu(MenuStorage::MAIN_MENU);
}

void
TitleScreen::leave()
{
  Sector* sector = titlesession->get_current_sector();
  sector->deactivate();
  MenuManager::instance().clear_menu_stack();
}

void
TitleScreen::draw(DrawingContext& context)
{
  Sector* sector  = titlesession->get_current_sector();
  sector->draw(context);

  // FIXME: Add something to scale the frame to the resolution of the screen
  //context.draw_surface(frame, Vector(0,0),LAYER_FOREGROUND1);

  context.draw_text(Resources::small_font,
                    copyright_text,
                    Vector(5, SCREEN_HEIGHT - 50),
                    ALIGN_LEFT, LAYER_FOREGROUND1);
}

void
TitleScreen::update(float elapsed_time)
{
  g_screen_manager->set_speed(0.6f);
  Sector* sector  = titlesession->get_current_sector();
  sector->update(elapsed_time);

  make_tux_jump();

  MenuManager::instance().check_menu();

  // reopen menu if user closed it (so that the app doesn't close when user
  // accidently hit ESC)
  if(!MenuManager::instance().is_active() && g_screen_manager->has_no_pending_fadeout())
  {
    MenuManager::instance().set_menu(MenuStorage::MAIN_MENU);
  }
}

/* EOF */
