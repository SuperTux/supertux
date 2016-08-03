//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "gui/menu_badguy_select.hpp"

#include "audio/sound_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/item_action.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

std::vector<std::string> BadguySelectMenu::all_badguys;

BadguySelectMenu::BadguySelectMenu(std::vector<std::string>* badguys_) :
  badguys(badguys_),
  selected(),
  remove_item()
{
  //initialize badguy list
  if (all_badguys.empty()) {
    all_badguys.push_back("angrystone");
    all_badguys.push_back("bouncingsnowball");
    all_badguys.push_back("captainsnowball");
    all_badguys.push_back("crystallo");
    all_badguys.push_back("dart");
    all_badguys.push_back("darttrap");
    all_badguys.push_back("dispenser");
    all_badguys.push_back("fish");
    all_badguys.push_back("flame");
    all_badguys.push_back("flyingsnowball");
    all_badguys.push_back("ghostflame");
    all_badguys.push_back("ghosttree");
    all_badguys.push_back("haywire");
    all_badguys.push_back("iceflame");
    all_badguys.push_back("igel");
    all_badguys.push_back("jumpy");
    all_badguys.push_back("kamikazesnowball");
    all_badguys.push_back("kugelblitz");
    all_badguys.push_back("leafshot");
    all_badguys.push_back("livefire");
    all_badguys.push_back("livefire_asleep");
    all_badguys.push_back("livefire_dormant");
    all_badguys.push_back("mole");
    all_badguys.push_back("mole_rock");
    all_badguys.push_back("mrbomb");
    all_badguys.push_back("mrcandle");
    all_badguys.push_back("mriceblock");
    all_badguys.push_back("mrtree");
    all_badguys.push_back("owl");
    all_badguys.push_back("plant");
    all_badguys.push_back("poisonivy");
    all_badguys.push_back("short_fuse");
    all_badguys.push_back("sspiky");
    all_badguys.push_back("skydive");
    all_badguys.push_back("skullyhop");
    all_badguys.push_back("smartball");
    all_badguys.push_back("smartblock");
    all_badguys.push_back("snail");
    all_badguys.push_back("snowball");
    all_badguys.push_back("snowman");
    all_badguys.push_back("spidermite");
    all_badguys.push_back("spiky");
    all_badguys.push_back("stalactite");
    all_badguys.push_back("stumpy");
    all_badguys.push_back("toad");
    all_badguys.push_back("totem");
    all_badguys.push_back("walkingleaf");
    all_badguys.push_back("willowisp");
    all_badguys.push_back("yeti");
    all_badguys.push_back("yeti_stalactite");
    all_badguys.push_back("zeekling");
  }

  refresh_menu();
}

BadguySelectMenu::~BadguySelectMenu()
{

}

void
BadguySelectMenu::refresh_menu()
{
  items.clear();

  add_label(_("List of enemies"));
  add_hl();
  add_string_select(-2, _("Enemy"), &selected, all_badguys);
  add_entry(-3, _("Add"));
  add_hl();

  int i = 0;
  for (auto& badguy : *badguys) {
    add_entry(i, badguy);
    i++;
  }

  add_hl();
  add_back(_("OK"));
}

void
BadguySelectMenu::remove_badguy()
{
  badguys->erase(badguys->begin() + remove_item);
  refresh_menu();
  if (items[active_item]->skippable()) {
    //We are on the bottom headline.
    active_item++;
  }
}

void
BadguySelectMenu::add_badguy()
{
  badguys->push_back(all_badguys[selected]);
  refresh_menu();
}

void
BadguySelectMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0) {
    remove_item = item->id;
    auto self  = this;
    // confirmation dialog
    std::unique_ptr<Dialog> dialog(new Dialog);
    dialog->set_text(_("Do you want to delete this badguy from the list?"));
    dialog->add_default_button(_("Yes"), [self] {
      self->remove_badguy();
    });
    dialog->add_cancel_button(_("No"));
    MenuManager::instance().set_dialog(std::move(dialog));
  } else if (item->id == -3) {
    add_badguy();
  }
}

/* EOF */
