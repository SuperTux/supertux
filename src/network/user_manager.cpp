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

#include "network/user_manager.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

namespace network {

UserManager::UserManager()
{
}

ServerUser*
UserManager::get_server_user(const std::string& nickname) const
{
  auto it = std::find_if(m_server_users.begin(), m_server_users.end(),
                         [nickname](const auto& user)
    {
      return user->nickname == nickname;
    });

  if (it == m_server_users.end())
    return nullptr;

  return it->get();
}

void
UserManager::parse_server_users(const std::string& data)
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

    m_server_users.push_back(create_server_user(iter.as_mapping()));
  }
}

std::string
UserManager::save_server_users(ServerUser* except) const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-server-users");
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

std::unique_ptr<ServerUser>
UserManager::create_server_user(const std::string& nickname) const
{
  return std::make_unique<ServerUser>(nickname);
}

std::unique_ptr<ServerUser>
UserManager::create_server_user(const ReaderMapping& reader) const
{
  return std::make_unique<ServerUser>(reader);
}

} // namespace network

/* EOF */
