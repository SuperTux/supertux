//  SuperTux
//  Copyright (C) 2004 Tobas Glaesser <tobi.web@gmx.de>
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

#include "supertux/menu/options_menu.hpp"

#include <algorithm>

#include "audio/sound_manager.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "supertux/menu/language_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/profile_menu.hpp"
#include "util/string_util.hpp"
#include "video/renderer.hpp"

enum OptionsMenuIDs {
  MNID_FULLSCREEN,
  MNID_FULLSCREEN_RESOLUTION,
  MNID_MAGNIFICATION,
  MNID_ASPECTRATIO,
  MNID_PROFILES,
  MNID_SOUND,
  MNID_MUSIC
};

OptionsMenu::OptionsMenu() :
  language_menu()
{
  language_menu.reset(new LanguageMenu());

  add_label(_("Options"));
  add_hl();

  // Language change should only be possible in the main menu, since elsewhere it might not always work fully
  // FIXME: Implement me: if (get_parent() == main_menu)
  add_submenu(_("Select Language"), language_menu.get())
    ->set_help(_("Select a different language to display text in"));

  add_submenu(_("Select Profile"), MenuStorage::get_profile_menu())
    ->set_help(_("Select a profile to play with"));

  add_toggle(MNID_PROFILES, _("Profile on Startup"), g_config->sound_enabled)
    ->set_help(_("Select your profile immediately after start-up"));
  
  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), g_config->use_fullscreen)
    ->set_help(_("Fill the entire screen"));

  MenuItem* fullscreen_res = add_string_select(MNID_FULLSCREEN_RESOLUTION, _("Resolution"));
  fullscreen_res->set_help(_("Determine the resolution used in fullscreen mode (you must toggle fullscreen to complete the change)"));

  MenuItem* magnification = add_string_select(MNID_MAGNIFICATION, _("Magnification"));
  magnification->set_help(_("Change the magnification of the game area"));

  // These values go from screen:640/projection:1600 to
  // screen:1600/projection:640 (i.e. 640, 800, 1024, 1280, 1600)
  magnification->list.push_back("auto");
  magnification->list.push_back("40%");
  magnification->list.push_back("50%");
  magnification->list.push_back("62.5%");
  magnification->list.push_back("80%");
  magnification->list.push_back("100%");
  magnification->list.push_back("125%");
  magnification->list.push_back("160%");
  magnification->list.push_back("200%");
  magnification->list.push_back("250%");

  SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);

  if (modes == (SDL_Rect **)0) 
  { // No resolutions at all available, bad

  }
  else if(modes == (SDL_Rect **)-1) 
  { // All resolutions should work, so we fall back to hardcoded defaults
    fullscreen_res->list.push_back("640x480");
    fullscreen_res->list.push_back("800x600");
    fullscreen_res->list.push_back("1024x768");
    fullscreen_res->list.push_back("1152x864");
    fullscreen_res->list.push_back("1280x960");
    fullscreen_res->list.push_back("1280x1024");
    fullscreen_res->list.push_back("1440x900");
    fullscreen_res->list.push_back("1680x1050");
    fullscreen_res->list.push_back("1600x1200");
    fullscreen_res->list.push_back("1920x1080");
    fullscreen_res->list.push_back("1920x1200");
  }
  else 
  {
    for(int i = 0; modes[i]; ++i)
    {
      std::ostringstream out;          
      out << modes[i]->w << "x" << modes[i]->h;
      fullscreen_res->list.push_back(out.str());
    }

    // On Ubuntu/Linux resolutions are returned from highest to
    // lowest, so reverse them
    std::sort(fullscreen_res->list.begin(), fullscreen_res->list.end(), StringUtil::numeric_less);
  }

  MenuItem* aspect = add_string_select(MNID_ASPECTRATIO, _("Aspect Ratio"));
  aspect->set_help(_("Adjust the aspect ratio"));
  
  aspect->list.push_back("auto");
  aspect->list.push_back("5:4");
  aspect->list.push_back("4:3");
  aspect->list.push_back("16:10");
  aspect->list.push_back("16:9");
  aspect->list.push_back("1368:768");

  if (g_config->aspect_size != Size(0, 0))
  {
    std::ostringstream out;
    out << g_config->aspect_size.width << ":" << g_config->aspect_size.height;
    std::string aspect_ratio = out.str();
    for(std::vector<std::string>::iterator i = aspect->list.begin(); i != aspect->list.end(); ++i)
    {
      if(*i == aspect_ratio)
      {
        aspect_ratio.clear();
        break;
      }
    }

    if (!aspect_ratio.empty())
    {
      aspect->selected = aspect->list.size();
      aspect->list.push_back(aspect_ratio);
    }
  }
  
  if (sound_manager->is_audio_enabled()) {
    add_toggle(MNID_SOUND, _("Sound"), g_config->sound_enabled)
      ->set_help(_("Disable all sound effects"));
    add_toggle(MNID_MUSIC, _("Music"), g_config->music_enabled)
      ->set_help(_("Disable all music"));
  } else {
    add_inactive(MNID_SOUND, _("Sound (disabled)"));
    add_inactive(MNID_MUSIC, _("Music (disabled)"));
  }
  
  add_submenu(_("Setup Keyboard"), MenuStorage::get_key_options_menu())
    ->set_help(_("Configure key-action mappings"));

  add_submenu(_("Setup Joystick"), MenuStorage::get_joystick_options_menu())
    ->set_help(_("Configure joystick control-action mappings"));
  add_hl();
  add_back(_("Back"));
}

OptionsMenu::~OptionsMenu()
{
}

void
OptionsMenu::menu_action(MenuItem* item)
{
  switch (item->id) {
    case MNID_ASPECTRATIO:
    { 
      if (item->list[item->selected] == "auto")
      {
        g_config->aspect_size = Size(0, 0); // Magic values
        Renderer::instance()->apply_config();
        MenuManager::recalc_pos();
      }
      else if (sscanf(item->list[item->selected].c_str(), "%d:%d", 
                      &g_config->aspect_size.width, &g_config->aspect_size.height) == 2)
      {
        Renderer::instance()->apply_config();
        MenuManager::recalc_pos();
      }
      else
      {
        assert(!"This must not be reached");
      }
    }
    break;

    case MNID_MAGNIFICATION:
      if (item->list[item->selected] == "auto")
      {
        g_config->magnification = 0.0f; // Magic value 
      }
      else if(sscanf(item->list[item->selected].c_str(), "%f", &g_config->magnification) == 1)
      {
        g_config->magnification /= 100.0f;
      }
      Renderer::instance()->apply_config();
      MenuManager::recalc_pos();
      break;

    case MNID_FULLSCREEN_RESOLUTION:
      if(sscanf(item->list[item->selected].c_str(), "%dx%d", 
                &g_config->fullscreen_size.width, &g_config->fullscreen_size.height) == 2)
      {
        // do nothing, changes are only applied when toggling fullscreen mode
      }      
      break;

    case MNID_FULLSCREEN:
      if(g_config->use_fullscreen != is_toggled(MNID_FULLSCREEN)) {
        g_config->use_fullscreen = !g_config->use_fullscreen;
        Renderer::instance()->apply_config();
        MenuManager::recalc_pos();
        g_config->save();
      }
      break;

    case MNID_SOUND:
      if(g_config->sound_enabled != is_toggled(MNID_SOUND)) {
        g_config->sound_enabled = !g_config->sound_enabled;
        sound_manager->enable_sound(g_config->sound_enabled);
        g_config->save();
      }
      break;

    case MNID_MUSIC:
      if(g_config->music_enabled != is_toggled(MNID_MUSIC)) {
        g_config->music_enabled = !g_config->music_enabled;
        sound_manager->enable_music(g_config->music_enabled);
        g_config->save();
      }
      break;

    default:
      break;
  }
}

void
OptionsMenu::check_menu()
{
}

/* EOF */
