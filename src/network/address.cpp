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

#include "network/address.hpp"

namespace network {

Address::Address() :
  host(),
  port()
{
}

Address::Address(ENetAddress& address) :
  Address()
{
  char hostname[1024];
  enet_address_get_host(&address, hostname, 1024);

  host = std::string(hostname);
  port = static_cast<uint16_t>(address.port);
}

std::string
Address::to_string() const
{
  return host + ":" + std::to_string(port);
}

std::ostream& operator<<(std::ostream& os, const Address& address)
{
  return os << address.to_string();
}

} // namespace network

/* EOF */
