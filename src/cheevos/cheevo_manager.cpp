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

#include "supertux/profile_manager.hpp"

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
CheevoManager::unlock(CheevoId cheevo, const Profile& profile, const Addon* addon)
{
  unlock_local(cheevo, profile, addon);
}

const std::vector<bool>& CheevoManager::get_unlocked(const Profile& profile, const Addon* addon)
{
  return get_unlocked_local(profile, addon);
}
