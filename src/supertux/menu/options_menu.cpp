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

#include <algorithm>
#include <sstream>
#include <stdio.h>

enum OptionsMenuIDs {
  MNID_FULLSCREEN,
  MNID_FULLSCREEN_RESOLUTION,
  MNID_MAGNIFICATION,
  MNID_ASPECTRATIO,
  MNID_SOUND,
  MNID_MUSIC
};

OptionsMenu::OptionsMenu(bool complete)
{
  add_label(_("Options"));
  add_hl();

  if (complete)
  {
    // Language and profile changes are only be possible in the
    // main menu, since elsewhere it might not always work fully
    add_submenu(_("Select Language"), MenuStorage::LANGUAGE_MENU)
      ->set_help(_("Select a different language to display text in"));

    add_submenu(_("Select Profile"), MenuStorage::PROFILE_MENU)
      ->set_help(_("Select a profile to play with"));
  }

  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), g_config->use_fullscreen)
    ->set_help(_("Fill the entire screen"));

  MenuItem* fullscreen_res = add_string_select(MNID_FULLSCREEN_RESOLUTION, _("Resolution"));
  fullscreen_res->set_help(_("Determine the resolution used in fullscreen mode (you must toggle fullscreen to complete the change)"));

  MenuItem* magnification = add_string_select(MNID_MAGNIFICATION, _("Magnification"));
  magnification->set_help(_("Change the magnification of the game area"));

  // These values go from screen:640/projection:1600 to
  // screen:1600/projection:640 (i.e. 640, 800, 1024, 1280, 1600)
  magnification->list.push_back(_("auto"));
  magnification->list.push_back("40%");
  magnification->list.push_back("50%");
  magnification->list.push_back("62.5%");
  magnification->list.push_back("80%");
  magnification->list.push_back("100%");
  magnification->list.push_back("125%");
  magnification->list.push_back("160%");
  magnification->list.push_back("200%");
  magnification->list.push_back("250%");
  if (g_config->magnification != 0.0f) //auto
  {
    std::ostringstream out;
    out << (g_config->magnification*100) << "%";
    std::string magn = out.str();
    size_t count = 0;
    for (std::vector<std::string>::iterator i = magnification->list.begin(); i != magnification->list.end(); ++i)
    {
      if (*i == magn)
      {
	magnification->selected = count;
	magn.clear();
	break;
      }

      ++count;
    }
    if (!magn.empty()) //magnification not in our list but accept anyway
    {
      magnification->selected = magnification->list.size();
      magnification->list.push_back(magn);
    }
  }

  int display_mode_count = SDL_GetNumDisplayModes(0);
  std::string last_display_mode;
  for(int i = 0; i < display_mode_count; ++i)
  {
    SDL_DisplayMode mode;
    int ret = SDL_GetDisplayMode(0, i, &mode);
    if (ret != 0)
    {
      log_warning << "failed to get display mode: " << SDL_GetError() << std::endl;
    }
    else
    {
      std::ostringstream out;
      out << mode.w << "x" << mode.h << "@" << mode.refresh_rate;
      if(last_display_mode == out.str())
        continue;
      last_display_mode = out.str();
      fullscreen_res->list.push_back(out.str());
    }
  }
  fullscreen_res->list.push_back("Desktop");

  std::ostringstream out;
  std::string fullscreen_size_str = "Desktop";
  if (g_config->fullscreen_size != Size(0, 0))
  {
    out << g_config->fullscreen_size.width << "x" << g_config->fullscreen_size.height << "@" << g_config->fullscreen_refresh_rate;
    fullscreen_size_str = out.str();
  }
  size_t cnt = 0;
  for (std::vector<std::string>::iterator i = fullscreen_res->list.begin(); i != fullscreen_res->list.end(); ++i)
  {
    if (*i == fullscreen_size_str)
    {
      fullscreen_size_str.clear();
      fullscreen_res->selected = cnt;
      break;
    }
    ++cnt;
  }
  if (!fullscreen_size_str.empty())
  {
    fullscreen_res->selected = fullscreen_res->list.size();
    fullscreen_res->list.push_back(fullscreen_size_str);
  }

  MenuItem* aspect = add_string_select(MNID_ASPECTRATIO, _("Aspect Ratio"));
  aspect->set_help(_("Adjust the aspect ratio"));

  aspect->list.push_back(_("auto"));
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
    size_t cnt = 0;
    for(std::vector<std::string>::iterator i = aspect->list.begin(); i != aspect->list.end(); ++i)
    {
      if(*i == aspect_ratio)
      {
        aspect_ratio.clear();
	aspect->selected = cnt;
        break;
      }
      ++cnt;
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

  add_submenu(_("Setup Keyboard"), MenuStorage::KEYBOARD_MENU)
    ->set_help(_("Configure key-action mappings"));

  add_submenu(_("Setup Joystick"), MenuStorage::JOYSTICK_MENU)
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
        if (item->list[item->selected] == _("auto"))
        {
          g_config->aspect_size = Size(0, 0); // Magic values
          Renderer::instance()->apply_config();
          MenuManager::instance().on_window_resize();
        }
        else if (sscanf(item->list[item->selected].c_str(), "%d:%d",
                        &g_config->aspect_size.width, &g_config->aspect_size.height) == 2)
        {
          Renderer::instance()->apply_config();
          MenuManager::instance().on_window_resize();
        }
        else
        {
          assert(!"This must not be reached");
        }
      }
      break;

    case MNID_MAGNIFICATION:
      if (item->list[item->selected] == _("auto"))
      {
        g_config->magnification = 0.0f; // Magic value
      }
      else if(sscanf(item->list[item->selected].c_str(), "%f", &g_config->magnification) == 1)
      {
        g_config->magnification /= 100.0f;
      }
      Renderer::instance()->apply_config();
      MenuManager::instance().on_window_resize();
      break;

    case MNID_FULLSCREEN_RESOLUTION:
      {
        int width;
        int height;
        int refresh_rate;
        if (item->list[item->selected] == "Desktop")
        {
          g_config->fullscreen_size.width = 0;
          g_config->fullscreen_size.height = 0;
          g_config->fullscreen_refresh_rate = 0;
        }
        else if(sscanf(item->list[item->selected].c_str(), "%dx%d@%d",
                  &width, &height, &refresh_rate) == 3)
        {
          // do nothing, changes are only applied when toggling fullscreen mode
          g_config->fullscreen_size.width = width;
          g_config->fullscreen_size.height = height;
          g_config->fullscreen_refresh_rate = refresh_rate;
        }
      }
      break;

    case MNID_FULLSCREEN:
      if(g_config->use_fullscreen != is_toggled(MNID_FULLSCREEN)) {
        g_config->use_fullscreen = !g_config->use_fullscreen;
        Renderer::instance()->apply_config();
        MenuManager::instance().on_window_resize();
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
