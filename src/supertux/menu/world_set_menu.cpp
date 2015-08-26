#include "supertux/menu/world_set_menu.hpp"

#include "audio/sound_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/addon_menu.hpp"
#include "supertux/menu/contrib_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/textscroller.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"

WorldSetMenu::WorldSetMenu()
{
   add_label(_("This is a menu."));
   add_hl();
   add_entry(WORLDSET_CAMPAIGN1, _("Campaign: Antartica"));
   add_entry(WORLDSET_CAMPAIGN2, _("Campaign: Ghost Forest"));
   add_entry(WORLDSET_CONTRIB, _("Contrib Levels"));
   add_hl();
   add_back(_("Back"));
 //add_entry(WORLDSET_ADDON, _("Addon Worlds"));
}

void WorldSetMenu::menu_action(MenuItem* item)
{
  switch(item->id)
  {
    case WORLDSET_CAMPAIGN1:
    {
      std::unique_ptr<World> world = World::load("levels/world1");
      GameManager::current()->start_worldmap(std::move(world));
      break;
    }
    case WORLDSET_CAMPAIGN2:
    {
      std::unique_ptr<World> world = World::load("levels/world2");
      GameManager::current()->start_worldmap(std::move(world));
      break;
    } 
    case WORLDSET_CONTRIB:
	MenuManager::instance().push_menu(MenuStorage::CONTRIB_MENU);    
	break;
	
    /*
    case WORLDSET_ADDON:
	// Add-ons Menu
	MenuManager::instance().push_menu(MenuStorage::ADDON_MENU);
	break;
    */
  }
}
