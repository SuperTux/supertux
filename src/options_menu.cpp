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

class OptionsMenu : public Menu
{
public:
  OptionsMenu();
  virtual ~OptionsMenu();

  virtual void menu_action(MenuItem* item);
};

OptionsMenu::OptionsMenu()
{
  add_label(_("Options"));
  add_hl();
  add_toggle(MNID_FULLSCREEN,_("Fullscreen"), config->use_fullscreen);
  if (sound_manager->is_audio_enabled()) {
    add_toggle(MNID_SOUND, _("Sound"), config->sound_enabled);
    add_toggle(MNID_MUSIC, _("Music"), config->music_enabled);
  } else {
    add_deactive(MNID_SOUND, _("Sound disabled"));
    add_deactive(MNID_SOUND, _("Music disabled"));
  }
  add_submenu(_("Setup Keys"), main_controller->get_key_options_menu());
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
  if(options_menu == NULL)
    options_menu = new OptionsMenu();

  return options_menu;
}

void free_options_menu()
{
  delete options_menu;
  options_menu = NULL;
}
