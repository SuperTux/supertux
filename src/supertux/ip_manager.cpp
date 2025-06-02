//  SuperTux
//  Copyright (C) 2025 Martim Ferreira <martim.silva.ferreira@tecnico.ulisboa.pt>
//                2025 Gonçalo Rocha <goncalo.r.f.rocha@tecnico.ulisboa.pt>
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

#include "supertux/ip_manager.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"

IPManager::IPManager() :
  m_ips()
{
}

void
IPManager::add_ip(const std::string& ip)
{
  sockaddr_in temp_addr;
  temp_addr.sin_family = AF_INET;
  temp_addr.sin_port = htons(50001);
  inet_pton(AF_INET, ip.c_str(), &temp_addr.sin_addr);
  m_ips.push_back(temp_addr);
}

void
IPManager::add_ip_addr(const sockaddr_in& addr)
{
  m_ips.push_back(addr);
}

void
IPManager::delete_ip(int id)
{
  if (id >= 0 && id < 4)
  {
    m_ips.erase(m_ips.begin() + id);
  }
  else
  {
    std::cerr << "Invalid IP index: " << id << std::endl;
  }
}

void
IPManager::clear_ips_exept_0()
{
  if (m_ips.size() > 1)
  {
    m_ips.erase(m_ips.begin() + 1, m_ips.end());
  }
}

/* EOF */
