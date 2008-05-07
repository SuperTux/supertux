//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobas Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include "profile_menu.hpp"
#include "options_menu.hpp"
#include "gui/menu.hpp"
#include "audio/sound_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "main.hpp"
#include "gettext.hpp"
#include "gameconfig.hpp"

Menu* options_menu   = 0;

enum OptionsMenuIDs {
  MNID_FULLSCREEN,
  MNID_ASPECTRATIO,
  MNID_SOUND,
  MNID_MUSIC
};

class LanguageMenu : public Menu
{
public:
  LanguageMenu() {
    add_label(_("Language"));
    add_hl();
    add_entry(0, std::string("(")+_("auto-detect language")+")");
    add_entry(1, "English");

    int mnid = 10;    
    std::set<std::string> languages = dictionary_manager.get_languages();
    for (std::set<std::string>::iterator i = languages.begin(); i != languages.end(); i++) {
      std::string locale_name = *i;
      TinyGetText::LanguageDef ldef = TinyGetText::get_language_def(locale_name);
      std::string locale_fullname = locale_name;
      if (std::string(ldef.code) == locale_name) {
        locale_fullname = ldef.name;
      }
      add_entry(mnid++, locale_fullname);
    } 

    add_hl();
    add_back(_("Back"));
  }

  virtual void menu_action(MenuItem* item) {
    if (item->id == 0) {
      config->locale = "";
      dictionary_manager.set_language(config->locale);
      config->save();
      Menu::set_current(0);
    }
    else if (item->id == 1) {
      config->locale = "en";
      dictionary_manager.set_language(config->locale);
      config->save();
      Menu::set_current(0);
    }
    int mnid = 10;    
    std::set<std::string> languages = dictionary_manager.get_languages();
    for (std::set<std::string>::iterator i = languages.begin(); i != languages.end(); i++) {
      std::string locale_name = *i;
      if (item->id == mnid++) {
        config->locale = locale_name;
        dictionary_manager.set_language(config->locale);
        config->save();
        Menu::set_current(0);
      }
    }
  }
};


class OptionsMenu : public Menu
{
public:
  OptionsMenu();
  virtual ~OptionsMenu();

  virtual void menu_action(MenuItem* item);

protected:
  std::auto_ptr<LanguageMenu> language_menu;
  
};

OptionsMenu::OptionsMenu()
{
  language_menu.reset(new LanguageMenu());

  add_label(_("Options"));
  add_hl();

  add_submenu(_("Select Language"), language_menu.get())
    ->set_help(_("Switch to another language"));

  add_submenu(_("Select Profile"), get_profile_menu())
    ->set_help(_("Switch between different savegames"));

  add_toggle(MNID_SOUND, _("Profile on Startup"), config->sound_enabled)
    ->set_help(_("Display the profile menu when the game is newly started"));
  
  // FIXME: Implement me: if (get_parent() == main_menu)
  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), config->use_fullscreen)
    ->set_help(_("Let the game cover the whole screen"));

  MenuItem* aspect = add_string_select(MNID_ASPECTRATIO, _("Aspect Ratio"));
  aspect->set_help(_("Adjust the aspect ratio"));
  
  aspect->list.push_back("16:9");
  aspect->list.push_back("16:10");
  aspect->list.push_back("4:3");
  aspect->list.push_back("5:4");

  std::ostringstream out;
  out << config->aspect_width << ":" << config->aspect_height;
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
  
  if (sound_manager->is_audio_enabled()) {
    add_toggle(MNID_SOUND, _("Sound"), config->sound_enabled)
      ->set_help(_("Disable all sound effects in the game"));
    add_toggle(MNID_MUSIC, _("Music"), config->music_enabled)
      ->set_help(_("Disable all music in the game"));
  } else {
    add_deactive(MNID_SOUND, _("Sound (disabled)"));
    add_deactive(MNID_SOUND, _("Music (disabled)"));
  }
  
  add_submenu(_("Setup Keyboard"), main_controller->get_key_options_menu())
    ->set_help(_("Configure how your keyboard maps to the game"));

  add_submenu(_("Setup Joystick"),main_controller->get_joystick_options_menu())
    ->set_help(_("Configure how your joystick maps to the game"));
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
      { // FIXME: Really crude and ugly here, move to video or so
        if(sscanf(item->list[item->selected].c_str(), "%d:%d", &config->aspect_width, &config->aspect_height) == 2) 
          {
            float aspect_ratio = static_cast<double>(config->aspect_width) /
              static_cast<double>(config->aspect_height);

            if (aspect_ratio > 1) {
              SCREEN_WIDTH  = static_cast<int> (600 * aspect_ratio + 0.5);
              SCREEN_HEIGHT = 600;
            } else {
              SCREEN_WIDTH  = 600;
              SCREEN_HEIGHT = static_cast<int> (600 * 1/aspect_ratio + 0.5);
            }

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
            std::cout << __FILE__ << ":" << __LINE__ << ": change aspect ratio to " << item->list[item->selected] << std::endl;

            // Reposition the menu to be in the center of the screen again
            set_pos(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
          }
      }
      break;

    case MNID_FULLSCREEN:
      if(config->use_fullscreen != options_menu->is_toggled(MNID_FULLSCREEN)) {
        config->use_fullscreen = !config->use_fullscreen;
        init_video();
        config->save();
      }
      break;
    case MNID_SOUND:
      if(config->sound_enabled != options_menu->is_toggled(MNID_SOUND)) {
        config->sound_enabled = !config->sound_enabled;
        sound_manager->enable_sound(config->sound_enabled);
        config->save();
      }
      break;
    case MNID_MUSIC:
      if(config->music_enabled != options_menu->is_toggled(MNID_MUSIC)) {
        config->music_enabled = !config->music_enabled;
        sound_manager->enable_music(config->music_enabled);
        config->save();
      }
      break;
    default:
      break;
  }
}

Menu* get_options_menu()
{
  //static OptionsMenu menu;
  options_menu = new OptionsMenu();
  return options_menu;
}

void free_options_menu()
{
  delete options_menu;
  options_menu = 0;
}
