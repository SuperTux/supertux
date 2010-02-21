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

#include "supertux/menu/language_menu.hpp"

extern "C" {
#include <findlocale.h>
}
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"

enum {
  MNID_LANGUAGE_AUTO_DETECT = 0,
  MNID_LANGUAGE_ENGLISH     = 1,
  MNID_LANGUAGE_NEXT        = 10
};

LanguageMenu::LanguageMenu() 
{
  add_label(_("Language"));
  add_hl();
  add_entry(MNID_LANGUAGE_AUTO_DETECT, _("<auto-detect>"));
  add_entry(MNID_LANGUAGE_ENGLISH, "English");

  int mnid = MNID_LANGUAGE_NEXT;
  std::set<tinygettext::Language> languages = dictionary_manager->get_languages();
  for (std::set<tinygettext::Language>::iterator i = languages.begin(); i != languages.end(); i++) 
  {
    add_entry(mnid++, i->get_name());
  }

  add_hl();
  add_back(_("Back"));
}

void
LanguageMenu::menu_action(MenuItem* item) 
{
  if (item->id == MNID_LANGUAGE_AUTO_DETECT) // auto detect
  {
    FL_Locale *locale;
    FL_FindLocale(&locale);
    tinygettext::Language language = tinygettext::Language::from_spec(locale->lang, locale->country, locale->variant);
    FL_FreeLocale(&locale);

    dictionary_manager->set_language(language);
    g_config->locale = language.str();
    g_config->save();
    MenuManager::pop_current();
  }
  else if (item->id == MNID_LANGUAGE_ENGLISH) // english
  {
    g_config->locale = "en";
    dictionary_manager->set_language(tinygettext::Language::from_name(g_config->locale));
    g_config->save();
    MenuManager::pop_current();
  }
  else
  {
    int mnid = MNID_LANGUAGE_NEXT;
    std::set<tinygettext::Language> languages = dictionary_manager->get_languages();

    for (std::set<tinygettext::Language>::iterator i = languages.begin(); i != languages.end(); i++) 
    {
      if (item->id == mnid++) 
      {
        g_config->locale = i->str();
        dictionary_manager->set_language(*i);
        g_config->save();
        MenuManager::pop_current();
        break;
      }
    }
  }
}

/* EOF */
