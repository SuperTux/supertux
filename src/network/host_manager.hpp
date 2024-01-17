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

#ifndef HEADER_SUPERTUX_NETWORK_HOST_MANAGER_HPP
#define HEADER_SUPERTUX_NETWORK_HOST_MANAGER_HPP

#include "util/currenton.hpp"

#include <memory>
#include <vector>

namespace network {

class Host;

class HostManager final : public Currenton<HostManager>
{
public:
  HostManager();

  void update();

  template<typename T, typename... Args>
  T& create(Args&&... args)
  {
    auto host = std::make_unique<T>(std::forward<Args>(args)...);
    T& host_ref = *host;
    m_hosts.push_back(std::move(host));
    return host_ref;
  }

private:
  std::vector<std::unique_ptr<Host>> m_hosts;

private:
  HostManager(const HostManager&) = delete;
  HostManager& operator=(const HostManager&) = delete;
};

} // namespace network

#endif

/* EOF */
