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

#include "supertux/title.hpp"

#include "addon/addon_manager.hpp"
#include "audio/sound_manager.hpp"
#include "gui/menu.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/main.hpp"
#include "supertux/mainloop.hpp"
#include "supertux/options_menu.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"

#include <algorithm>
#include <physfs.h>

enum MainMenuIDs {
  MNID_STARTGAME,
  MNID_LEVELS_CONTRIB,
  MNID_ADDONS,
  MNID_OPTIONMENU,
  MNID_LEVELEDITOR,
  MNID_CREDITS,
  MNID_QUITMAINMENU
};

void
TitleScreen::update_load_game_menu()
{
}

void
TitleScreen::free_contrib_menu()
{
  for(std::vector<World*>::iterator i = contrib_worlds.begin();
      i != contrib_worlds.end(); ++i)
    delete *i;

  contrib_worlds.clear();
}

void
TitleScreen::generate_contrib_menu()
{
  /** Generating contrib levels list by making use of Level Subset  */
  std::vector<std::string> level_worlds;
  char** files = PHYSFS_enumerateFiles("levels/");
  for(const char* const* filename = files; *filename != 0; ++filename) {
    std::string filepath = std::string("levels/") + *filename;
    if(PHYSFS_isDirectory(filepath.c_str()))
      level_worlds.push_back(filepath);
  }
  PHYSFS_freeList(files);

  free_contrib_menu();
  contrib_menu.reset(new Menu());

  contrib_menu->add_label(_("Contrib Levels"));
  contrib_menu->add_hl();

  int i = 0;
  for (std::vector<std::string>::iterator it = level_worlds.begin();
      it != level_worlds.end(); ++it) {
    try {
      std::auto_ptr<World> world (new World());
      world->load(*it + "/info");
      if(world->hide_from_contribs) {
        continue;
      }
      contrib_menu->add_entry(i++, world->title);
      contrib_worlds.push_back(world.release());
    } catch(std::exception& e) {
      log_warning << "Couldn't parse levelset info for '" << *it << "': " << e.what() << std::endl;
    }
  }

  contrib_menu->add_hl();
  contrib_menu->add_back(_("Back"));
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
TitleScreen::check_levels_contrib_menu()
{
  int index = contrib_menu->check();
  if (index == -1)
    return;

  current_world = contrib_worlds[index];

  if(!current_world->is_levelset) {
    start_game();
  } else {
    contrib_world_menu.reset(new Menu());

    contrib_world_menu->add_label(current_world->title);
    contrib_world_menu->add_hl();

    for (unsigned int i = 0; i < current_world->get_num_levels(); ++i)
    {
      /** get level's title */
      std::string filename = current_world->get_level_filename(i);
      std::string title = get_level_name(filename);
      contrib_world_menu->add_entry(i, title);
    }

    contrib_world_menu->add_hl();
    contrib_world_menu->add_back(_("Back"));

    Menu::push_current(contrib_world_menu.get());
  }
}

void
TitleScreen::check_contrib_world_menu()
{
  int index = contrib_world_menu->check();
  if (index != -1) {
    if (contrib_world_menu->get_item_by_id(index).kind == MN_ACTION) {
      sound_manager->stop_music();
      GameSession* session =
        new GameSession(current_world->get_level_filename(index));
      main_loop->push_screen(session);
    }
  }
}

namespace {
  bool generate_addons_menu_sorter(const Addon* a1, const Addon* a2)
  {
    return a1->title < a2->title;
  }

  const int ADDON_LIST_START_ID = 10;
}

void
TitleScreen::generate_addons_menu()
{
  AddonManager& adm = AddonManager::get_instance();

  // refresh list of addons
  addons = adm.get_addons();
  
  // sort list
  std::sort(addons.begin(), addons.end(), generate_addons_menu_sorter);

  // (re)generate menu
  free_addons_menu();
  addons_menu.reset(new Menu());

  addons_menu->add_label(_("Add-ons"));
  addons_menu->add_hl();
  
#ifdef HAVE_LIBCURL
  addons_menu->add_entry(0, std::string(_("Check Online")));
#else
  addons_menu->add_inactive(0, std::string(_("Check Online (disabled)")));
#endif

  //addons_menu->add_hl();

  for (unsigned int i = 0; i < addons.size(); i++) {
    const Addon& addon = *addons[i];
    std::string text = "";
    if (addon.kind != "") text += addon.kind + " ";
    text += std::string("\"") + addon.title + "\"";
    if (addon.author != "") text += " by \"" + addon.author + "\"";
    addons_menu->add_toggle(ADDON_LIST_START_ID + i, text, addon.loaded);
  }

  addons_menu->add_hl();
  addons_menu->add_back(_("Back"));
}

void
TitleScreen::check_addons_menu()
{
  int index = addons_menu->check();
  if (index == -1) return;

  // check if "Check Online" was chosen
  if (index == 0) {
    try {
      AddonManager::get_instance().check_online();
      generate_addons_menu();
      Menu::set_current(addons_menu.get());
      addons_menu->set_active_item(index);
    } 
    catch (std::runtime_error e) {
      log_warning << "Check for available Add-ons failed: " << e.what() << std::endl;
    }
    return;
  }

  // if one of the Addons listed was chosen, take appropriate action
  if ((index >= ADDON_LIST_START_ID) && (index < ADDON_LIST_START_ID) + addons.size()) {
    Addon& addon = *addons[index - ADDON_LIST_START_ID];
    if (!addon.installed) {
      try {
        AddonManager::get_instance().install(&addon);
      } 
      catch (std::runtime_error e) {
        log_warning << "Installing Add-on failed: " << e.what() << std::endl;
      }
      addons_menu->set_toggled(index, addon.loaded);
    } else if (!addon.loaded) {
      try {
        AddonManager::get_instance().enable(&addon);
      } 
      catch (std::runtime_error e) {
        log_warning << "Enabling Add-on failed: " << e.what() << std::endl;
      }
      addons_menu->set_toggled(index, addon.loaded);
    } else {
      try {
        AddonManager::get_instance().disable(&addon);
      } 
      catch (std::runtime_error e) {
        log_warning << "Disabling Add-on failed: " << e.what() << std::endl;
      }
      addons_menu->set_toggled(index, addon.loaded);
    }
  }
}

void
TitleScreen::free_addons_menu()
{
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
  Rect lookahead = tux->get_bbox();
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

TitleScreen::TitleScreen()
{
  controller.reset(new CodeController());
  titlesession.reset(new GameSession("levels/misc/menu.stl"));

  Player* player = titlesession->get_current_sector()->player;
  player->set_controller(controller.get());
  player->set_speedlimit(230); //MAX_WALK_XM

  generate_main_menu();

  frame = std::auto_ptr<Surface>(new Surface("images/engine/menu/frame.png"));
}

void
TitleScreen::generate_main_menu()
{
  main_menu.reset(new Menu());
  main_menu->set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 35);
  main_menu->add_entry(MNID_STARTGAME, _("Start Game"));
  main_menu->add_entry(MNID_LEVELS_CONTRIB, _("Contrib Levels"));
  main_menu->add_entry(MNID_ADDONS, _("Add-ons"));
  main_menu->add_submenu(_("Options"), get_options_menu());
  main_menu->add_entry(MNID_CREDITS, _("Credits"));
  main_menu->add_entry(MNID_QUITMAINMENU, _("Quit"));
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

  Menu::set_current(main_menu.get());
}

void
TitleScreen::leave()
{
  Sector* sector = titlesession->get_current_sector();
  sector->deactivate();
  Menu::set_current(NULL);
}

void
TitleScreen::draw(DrawingContext& context)
{
  Sector* sector  = titlesession->get_current_sector();
  sector->draw(context);

  // FIXME: Add something to scale the frame to the resolution of the screen
  context.draw_surface(frame.get(), Vector(0,0),LAYER_FOREGROUND1);

  context.draw_text(small_font, "SuperTux " PACKAGE_VERSION "\n",
      Vector(5, SCREEN_HEIGHT - 50), ALIGN_LEFT, LAYER_FOREGROUND1);
  context.draw_text(small_font,
      _(
"Copyright (c) 2007 SuperTux Devel Team\n"
"This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
"redistribute it under certain conditions; see the file COPYING for details.\n"
),
      Vector(5, SCREEN_HEIGHT - 50 + small_font->get_height() + 5),
      ALIGN_LEFT, LAYER_FOREGROUND1);
}

void
TitleScreen::update(float elapsed_time)
{
  main_loop->set_speed(0.6f);
  Sector* sector  = titlesession->get_current_sector();
  sector->update(elapsed_time);

  make_tux_jump();

  Menu* menu = Menu::current();
  if(menu) {
    if(menu == main_menu.get()) {
      switch (main_menu->check()) {
        case MNID_STARTGAME:
          // Start Game, ie. goto the slots menu
          if(main_world.get() == NULL) {
            main_world.reset(new World());
            main_world->load("levels/world1/info");
          }
          current_world = main_world.get();
          start_game();
          break;

        case MNID_LEVELS_CONTRIB:
          // Contrib Menu
          generate_contrib_menu();
          Menu::push_current(contrib_menu.get());
          break;

        case MNID_ADDONS:
          // Add-ons Menu
          generate_addons_menu();
          Menu::push_current(addons_menu.get());
          break;

        case MNID_CREDITS:
          Menu::set_current(NULL);
          main_loop->push_screen(new TextScroller("credits.txt"),
                                 new FadeOut(0.5));
          break;

        case MNID_QUITMAINMENU:
          main_loop->quit(new FadeOut(0.25));
          sound_manager->stop_music(0.25);
          break;
      }
    } else if(menu == contrib_menu.get()) {
      check_levels_contrib_menu();
    } else if(menu == addons_menu.get()) {
      check_addons_menu();
    } else if (menu == contrib_world_menu.get()) {
      check_contrib_world_menu();
    }
  }

  // reopen menu if user closed it (so that the app doesn't close when user
  // accidently hit ESC)
  if(Menu::current() == 0 && main_loop->has_no_pending_fadeout()) {
    generate_main_menu();
    Menu::set_current(main_menu.get());
  }
}

void
TitleScreen::start_game()
{
  Menu::set_current(NULL);
  std::string basename = current_world->get_basedir();
  basename = basename.substr(0, basename.length()-1);
  std::string worlddirname = FileSystem::basename(basename);
  std::ostringstream stream;
  stream << "profile" << config->profile << "/" << worlddirname << ".stsg";
  std::string slotfile = stream.str();

  try {
    current_world->set_savegame_filename(slotfile);
    current_world->run();
  } catch(std::exception& e) {
    log_fatal << "Couldn't start world: " << e.what() << std::endl;
  }
}

/* EOF */
