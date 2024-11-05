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

#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "network/server_user.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace network {

template<class U>
class UserProtocol;

/** Base class for classes, which manage server users. */
template<class U = ServerUser>
class UserManager
{
  static_assert(std::is_base_of<ServerUser, U>::value, "U must derive from ServerUser!");

  friend class UserProtocol<U>;

public:
  UserManager();

  const std::vector<std::unique_ptr<U>>& get_server_users() const { return m_server_users; }
  const std::optional<U>& get_self_user() const { return m_self_user; }

protected:
  U* get_server_user(const std::string& username);

  void parse_server_users(const std::string& data);
  std::string save_server_users(ServerUser* except = nullptr) const;

protected:
  std::vector<std::unique_ptr<U>> m_server_users;
  std::optional<U> m_self_user;

private:
  UserManager(const UserManager&) = delete;
  UserManager& operator=(const UserManager&) = delete;
};


/** SOURCE */

template<class U>
UserManager<U>::UserManager() :
  m_server_users(),
  m_self_user()
{
}

template<class U>
U*
UserManager<U>::get_server_user(const std::string& username)
{
  if (m_self_user && username == m_self_user->username)
    return &*m_self_user;

  auto it = std::find_if(m_server_users.begin(), m_server_users.end(),
                         [username](const auto& user)
    {
      return user->username == username;
    });

  if (it == m_server_users.end())
    return nullptr;

  return it->get();
}

template<class U>
void
UserManager<U>::parse_server_users(const std::string& data)
{
  auto doc = ReaderDocument::from_string(data, "server-users");
  auto root = doc.get_root();
  if (root.get_name() != "supertux-server-users")
    throw std::runtime_error("Cannot parse server users: Data is not 'supertux-server-users'.");

  auto iter = root.get_mapping().get_iter();
  while (iter.next())
  {
    if (iter.get_key() != "user")
    {
      log_warning << "Unknown key '" << iter.get_key() << "' in server users data." << std::endl;
      continue;
    }

    m_server_users.push_back(std::make_unique<U>(iter.as_mapping()));
  }
}

template<class U>
std::string
UserManager<U>::save_server_users(ServerUser* except) const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-server-users");
  if (m_self_user)
  {
    writer.start_list("user");
    m_self_user->write(writer);
    writer.end_list("user");
  }
  for (const auto& user : m_server_users)
  {
    if (user.get() == except) continue;

    writer.start_list("user");
    user->write(writer);
    writer.end_list("user");
  }
  writer.end_list("supertux-server-users");

  return stream.str();
}

} // namespace network

#endif

/* EOF */
