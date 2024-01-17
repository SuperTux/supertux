//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "network/host_manager.hpp"

#include "network/host.hpp"

namespace network {

HostManager::HostManager() :
  m_hosts()
{
}

void
HostManager::update()
{
  /** Update hosts.
      Remove ones, queued for removal. */
  auto it = m_hosts.begin();
  while (it != m_hosts.end())
  {
    const auto& host = *it;
    host->update();

    if (host->is_valid())
      it++;
    else
      it = m_hosts.erase(it);
  }
}

} // namespace network

/* EOF */
