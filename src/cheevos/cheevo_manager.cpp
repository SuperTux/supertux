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

#include "cheevo_manager.hpp"

#include <fmt/format.h>

#include "supertux/profile_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/notification.hpp"
#include "util/log.hpp"

CheevoManager g_cheevos;

CheevoManager::CheevoManager():
  m_profiledata()
{

}

void
CheevoManager::init()
{
  std::vector<Profile*> profiles = ProfileManager::current()->get_profiles();
  for (Profile* profile : profiles) {
    m_profiledata.try_emplace(profile->get_id(), CheevoProfileData{});
  }

  init_local();
}

void
CheevoManager::deinit()
{
  deinit_local();
}

void
CheevoManager::unlock_notify(CheevoId cheevo, const Addon* addon)
{
  if (MenuManager::current() == nullptr)
  {
    log_warning << "Attempted to show cheevo " << cheevo <<
                   " unlock notification with null MenuManager." << std::endl;
    return;
  }

  const CheevoData& cheevodata = g_cheevo_data.at(cheevo);
  auto notif = std::make_unique<Notification>("cheevo_unlock_" + cheevo, 20.f, false, true);
  notif->set_image(cheevodata.image);
  notif->set_text(fmt::format(fmt::runtime(_("Achievement unlocked: \"{}\"")), cheevodata.get_name()));
  notif->set_mini_text(cheevodata.get_requirement());
  MenuManager::instance().set_notification(std::move(notif));
}

void
CheevoManager::unlock(CheevoId cheevo, const Profile& profile, const Addon* addon)
{
  log_info << "Unlocked cheevo: " << cheevo;

  unlock_notify(cheevo, addon);

  unlock_local(cheevo, profile, addon);
}

CheevosUnlocked const&
CheevoManager::get_unlocked(const Profile& profile, const Addon* addon)
{
  return get_unlocked_local(profile, addon);
}
