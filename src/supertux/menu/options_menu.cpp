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
  MNID_MUSIC,
  MNID_DEVELOPER_MODE,
  MNID_CHRISTMAS_MODE
};

OptionsMenu::OptionsMenu(bool complete) :
  next_magnification(0),
  next_aspect_ratio(0),
  next_resolution(0),
  magnifications(),
  aspect_ratios(),
  resolutions()
{
  add_label(_("Options"));
  add_hl();

  magnifications.clear();
  // These values go from screen:640/projection:1600 to
  // screen:1600/projection:640 (i.e. 640, 800, 1024, 1280, 1600)
  magnifications.push_back(_("auto"));
  magnifications.push_back("40%");
  magnifications.push_back("50%");
  magnifications.push_back("62.5%");
  magnifications.push_back("80%");
  magnifications.push_back("100%");
  magnifications.push_back("125%");
  magnifications.push_back("160%");
  magnifications.push_back("200%");
  magnifications.push_back("250%");
  // Gets the actual magnification:
  if (g_config->magnification != 0.0f) //auto
  {
    std::ostringstream out;
    out << (g_config->magnification*100) << "%";
    std::string magn = out.str();
    size_t count = 0;
    for (std::vector<std::string>::iterator i = magnifications.begin(); i != magnifications.end(); ++i)
    {
      if (*i == magn)
      {
        next_magnification = count;
        magn.clear();
        break;
      }

      ++count;
    }
    if (!magn.empty()) //magnification not in our list but accept anyway
    {
      next_magnification = magnifications.size();
      magnifications.push_back(magn);
    }
  }

  aspect_ratios.clear();
  aspect_ratios.push_back(_("auto"));
  aspect_ratios.push_back("5:4");
  aspect_ratios.push_back("4:3");
  aspect_ratios.push_back("16:10");
  aspect_ratios.push_back("16:9");
  aspect_ratios.push_back("1368:768");
  // Gets the actual aspect ratio:
  if (g_config->aspect_size != Size(0, 0)) //auto
  {
    std::ostringstream out;
    out << g_config->aspect_size.width << ":" << g_config->aspect_size.height;
    std::string aspect_ratio = out.str();
    size_t cnt_ = 0;
    for(std::vector<std::string>::iterator i = aspect_ratios.begin(); i != aspect_ratios.end(); ++i)
    {
      if(*i == aspect_ratio)
      {
        aspect_ratio.clear();
        next_aspect_ratio = cnt_;
        break;
      }
      ++cnt_;
    }

    if (!aspect_ratio.empty())
    {
      next_aspect_ratio = aspect_ratios.size();
      aspect_ratios.push_back(aspect_ratio);
    }
  }

  resolutions.clear();
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
      out << mode.w << "x" << mode.h;
      if(mode.refresh_rate)
        out << "@" << mode.refresh_rate;
      if(last_display_mode == out.str())
        continue;
      last_display_mode = out.str();
      resolutions.insert(resolutions.begin(), out.str());
    }
  }
  resolutions.push_back("Desktop");

  std::string fullscreen_size_str = "Desktop";
  {
    std::ostringstream out;
    if (g_config->fullscreen_size != Size(0, 0))
    {
      out << g_config->fullscreen_size.width << "x" << g_config->fullscreen_size.height;
      if (g_config->fullscreen_refresh_rate)
         out << "@" << g_config->fullscreen_refresh_rate;
      fullscreen_size_str = out.str();
    }
  }

  size_t cnt = 0;
  for (std::vector<std::string>::iterator i = resolutions.begin(); i != resolutions.end(); ++i)
  {
    if (*i == fullscreen_size_str)
    {
      fullscreen_size_str.clear();
      next_resolution = cnt;
      break;
    }
    ++cnt;
  }
  if (!fullscreen_size_str.empty())
  {
    next_resolution = resolutions.size();
    resolutions.push_back(fullscreen_size_str);
  }

  if (complete)
  {
    // Language and profile changes are only be possible in the
    // main menu, since elsewhere it might not always work fully
    add_submenu(_("Select Language"), MenuStorage::LANGUAGE_MENU)
      ->set_help(_("Select a different language to display text in"));

    add_submenu(_("Select Profile"), MenuStorage::PROFILE_MENU)
      ->set_help(_("Select a profile to play with"));
  }

  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), &g_config->use_fullscreen)
    ->set_help(_("Fill the entire screen"));

  MenuItem* fullscreen_res = add_string_select(MNID_FULLSCREEN_RESOLUTION, _("Resolution"), &next_resolution, resolutions);
  fullscreen_res->set_help(_("Determine the resolution used in fullscreen mode (you must toggle fullscreen to complete the change)"));

  MenuItem* magnification = add_string_select(MNID_MAGNIFICATION, _("Magnification"), &next_magnification, magnifications);
  magnification->set_help(_("Change the magnification of the game area"));

  MenuItem* aspect = add_string_select(MNID_ASPECTRATIO, _("Aspect Ratio"), &next_aspect_ratio, aspect_ratios);
  aspect->set_help(_("Adjust the aspect ratio"));

  if (SoundManager::current()->is_audio_enabled()) {
    add_toggle(MNID_SOUND, _("Sound"), &g_config->sound_enabled)
      ->set_help(_("Disable all sound effects"));
    add_toggle(MNID_MUSIC, _("Music"), &g_config->music_enabled)
      ->set_help(_("Disable all music"));
  } else {
    add_inactive( _("Sound (disabled)"));
    add_inactive( _("Music (disabled)"));
  }

  add_submenu(_("Setup Keyboard"), MenuStorage::KEYBOARD_MENU)
    ->set_help(_("Configure key-action mappings"));

  add_submenu(_("Setup Joystick"), MenuStorage::JOYSTICK_MENU)
    ->set_help(_("Configure joystick control-action mappings"));

  if (g_config->developer_mode)
  {
    add_toggle(MNID_DEVELOPER_MODE, _("Developer Mode"), &g_config->developer_mode);
  }

  if (g_config->is_christmas() || g_config->christmas_mode)
  {
    add_toggle(MNID_CHRISTMAS_MODE, _("Christmas Mode"), &g_config->christmas_mode);
  }

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
        if (aspect_ratios[next_aspect_ratio] == _("auto"))
        {
          g_config->aspect_size = Size(0, 0); // Magic values
          VideoSystem::current()->get_renderer().apply_config();
          MenuManager::instance().on_window_resize();
        }
        else if (sscanf(aspect_ratios[next_aspect_ratio].c_str(), "%d:%d",
                        &g_config->aspect_size.width, &g_config->aspect_size.height) == 2)
        {
          VideoSystem::current()->get_renderer().apply_config();
          MenuManager::instance().on_window_resize();
        }
        else
        {
          assert(!"This must not be reached");
        }
      }
      break;

    case MNID_MAGNIFICATION:
      if (magnifications[next_magnification] == _("auto"))
      {
        g_config->magnification = 0.0f; // Magic value
      }
      else if(sscanf(magnifications[next_magnification].c_str(), "%f", &g_config->magnification) == 1)
      {
        g_config->magnification /= 100.0f;
      }
      VideoSystem::current()->get_renderer().apply_config();
      MenuManager::instance().on_window_resize();
      break;

    case MNID_FULLSCREEN_RESOLUTION:
      {
        int width;
        int height;
        int refresh_rate;
        if (resolutions[next_resolution] == "Desktop")
        {
          g_config->fullscreen_size.width = 0;
          g_config->fullscreen_size.height = 0;
          g_config->fullscreen_refresh_rate = 0;
        }
        else if(sscanf(resolutions[next_resolution].c_str(), "%dx%d@%d",
                  &width, &height, &refresh_rate) == 3)
        {
          // do nothing, changes are only applied when toggling fullscreen mode
          g_config->fullscreen_size.width = width;
          g_config->fullscreen_size.height = height;
          g_config->fullscreen_refresh_rate = refresh_rate;
        }
        else if(sscanf(resolutions[next_resolution].c_str(), "%dx%d",
                       &width, &height) == 2)
        {
            g_config->fullscreen_size.width = width;
            g_config->fullscreen_size.height = height;
            g_config->fullscreen_refresh_rate = 0;
        }
      }
      break;

    case MNID_FULLSCREEN:
      VideoSystem::current()->get_renderer().apply_config();
      MenuManager::instance().on_window_resize();
      g_config->save();
      break;

    case MNID_SOUND:
      SoundManager::current()->enable_sound(g_config->sound_enabled);
      g_config->save();
      break;

    case MNID_MUSIC:
      SoundManager::current()->enable_music(g_config->music_enabled);
      g_config->save();
      break;

    default:
      break;
  }
}

/* EOF */
