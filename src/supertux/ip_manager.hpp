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

#ifndef HEADER_SUPERTUX_SUPERTUX_IP_MANAGER_HPP
#define HEADER_SUPERTUX_SUPERTUX_IP_MANAGER_HPP

#include "util/currenton.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <vector>

class IPManager final : public Currenton<IPManager>
{
public:
  IPManager();

  inline sockaddr_in get_ip(int id) const { return m_ips[id]; }
  inline const std::vector<sockaddr_in>& get_ips() const { return m_ips; }

  void add_ip(const std::string& ip);
  void add_ip_addr(const sockaddr_in& addr);
  void delete_ip(int id);
  void clear_ips_exept_0();

private:
  std::vector<sockaddr_in> m_ips;

private:
  IPManager(const IPManager&) = delete;
  IPManager& operator=(const IPManager&) = delete;
};

#endif

/* EOF */
