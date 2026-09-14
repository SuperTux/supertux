//  SuperTux
//  Copyright (C) 2026 MatusGuy
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

#include "debug_cheevos_menu.hpp"

#include "cheevos/cheevo_manager.hpp"
#include "supertux/profile_manager.hpp"

DebugCheevosMenu::DebugCheevosMenu():
  Menu()
{
  add_label(_("Achievement Management"));
  add_hl();

  add_entry(_("Reset Local Achievement Progress for this Profile"), []{
    g_cheevos.reset_all_local(ProfileManager::current()->get_current_profile());
  });
  add_hl();

  {
    Profile& profile = ProfileManager::current()->get_current_profile();
    CheevosUnlocked const& cheevos = g_cheevos.get_unlocked(profile);

    auto it = g_cheevo_data.begin();
    for (int i = 0; it != g_cheevo_data.end(); it++, i++) {
      const CheevoId& id = it->first;

      // TODO: handle resetting cheevo
      add_toggle(i, it->second.get_name(),
                 [id, &cheevos]() -> bool {
                   auto findit = cheevos.find(id); // twist it! squeeze it!
                   return findit != cheevos.end() && findit->second.unlocked();
                 },
                 [id, &profile](bool) { g_cheevos.unlock(id, profile); });
    }
  }

  add_hl();
  add_back(_("Back"));
}

void
DebugCheevosMenu::menu_action(MenuItem& item)
{
}
