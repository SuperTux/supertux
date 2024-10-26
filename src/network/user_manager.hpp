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

#ifndef HEADER_SUPERTUX_NETWORK_USER_MANAGER_HPP
#define HEADER_SUPERTUX_NETWORK_USER_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>

#include "network/server_user.hpp"

class ReaderMapping;

namespace network {

/** Base class for classes, which manage server users. */
class UserManager
{
  friend class UserProtocol;

public:
  UserManager();

protected:
  ServerUser* get_server_user(const std::string& nickname) const;

  void parse_server_users(const std::string& data);
  std::string save_server_users(ServerUser* except = nullptr) const;

  virtual std::unique_ptr<ServerUser> create_server_user(const std::string& nickname) const;
  virtual std::unique_ptr<ServerUser> create_server_user(const ReaderMapping& reader) const;

protected:
  std::vector<std::unique_ptr<ServerUser>> m_server_users;

private:
  UserManager(const UserManager&) = delete;
  UserManager& operator=(const UserManager&) = delete;
};

} // namespace network

#endif

/* EOF */
