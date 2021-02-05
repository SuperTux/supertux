//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_NETWORK_SUPERTUX_PROTOCOL_HPP
#define HEADER_SUPERTUX_NETWORK_SUPERTUX_PROTOCOL_HPP

#include "network/connection_pool.hpp"

namespace network {

class SupertuxProtocol : public ConnectionPool
{
public:
  SupertuxProtocol();
private:
  SupertuxProtocol(const SupertuxProtocol&) = delete;
  SupertuxProtocol& operator=(const SupertuxProtocol&) = delete;
};

} // namespace network

#endif

/* EOF */
