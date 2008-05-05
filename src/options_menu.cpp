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
  
  // FIXME: Implement me: if (get_parent() == main_menu)
    add_submenu(_("Change Profile"), get_profile_menu());

  add_submenu(_("Select Language"), language_menu.get());
  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), config->use_fullscreen);
  if (sound_manager->is_audio_enabled()) {
    add_toggle(MNID_SOUND, _("Sound"), config->sound_enabled);
    add_toggle(MNID_MUSIC, _("Music"), config->music_enabled);
  } else {
    add_deactive(MNID_SOUND, _("Sound (disabled)"));
    add_deactive(MNID_SOUND, _("Music (disabled)"));
  }
  add_submenu(_("Setup Keyboard"), main_controller->get_key_options_menu());
  add_submenu(_("Setup Joystick"),main_controller->get_joystick_options_menu());
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
