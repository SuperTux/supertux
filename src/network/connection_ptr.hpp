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

#ifndef HEADER_SUPERTUX_NETWORK_CONNECTION_PTR_HPP
#define HEADER_SUPERTUX_NETWORK_CONNECTION_PTR_HPP

#include "network/connection.hpp"

namespace network {

struct connection_deleter {
  void operator()(network::Connection* c)
  {
    if (c)
      c->destroy();
  }
};

/**
 * Contains and manages a Connection object.
 * The purpose of this class is to handle the destroy() function of the Connection
 * object before the object is destroyed (@see Connection::destroy() for details).
 * 
 * Connection objects should always be dealt through a ConnectionPtr object.
 * 
 * This class is to be interpreted as similar to std::unique_ptr<network::Connection>.
 */
typedef std::unique_ptr<network::Connection, connection_deleter> ConnectionPtr;

} // namespace network

#endif

/* EOF */
