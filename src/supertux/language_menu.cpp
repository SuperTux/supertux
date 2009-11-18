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

#include "supertux/language_menu.hpp"

#include "gui/menu_item.hpp"
#include "supertux/gameconfig.hpp"

LanguageMenu::LanguageMenu() 
{
  add_label(_("Language"));
  add_hl();
  add_entry(0, std::string("<")+_("auto-detect")+">");
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

void
LanguageMenu::menu_action(MenuItem* item) 
{
  if (item->id == 0) {
    g_config->locale = "";
    dictionary_manager.set_language(g_config->locale);
    g_config->save();
    Menu::pop_current();
  }
  else if (item->id == 1) {
    g_config->locale = "en";
    dictionary_manager.set_language(g_config->locale);
    g_config->save();
    Menu::pop_current();
  }
  int mnid = 10;    
  std::set<std::string> languages = dictionary_manager.get_languages();
  for (std::set<std::string>::iterator i = languages.begin(); i != languages.end(); i++) {
    std::string locale_name = *i;
    if (item->id == mnid++) {
      g_config->locale = locale_name;
      dictionary_manager.set_language(g_config->locale);
      g_config->save();
      Menu::pop_current();
    }
  }
}

/* EOF */
