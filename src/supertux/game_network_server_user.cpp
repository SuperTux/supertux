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

#include "supertux/game_network_server_user.hpp"

#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

GameServerUser::GameServerUser(const std::string& username_, Color username_color_,
                               int num_players) :
  network::ServerUser(username_, username_color_),
  player_controllers()
{
  for (int i = 0; i < num_players; i++)
    player_controllers.push_back(std::make_unique<NetworkController>());
}

GameServerUser::GameServerUser(const ReaderMapping& reader) :
  network::ServerUser(reader),
  player_controllers()
{
  int num_players = 1;
  reader.get("num-players", num_players);
  for (int i = 0; i < num_players; i++)
    player_controllers.push_back(std::make_unique<NetworkController>());
}

void
GameServerUser::write(Writer& writer) const
{
  network::ServerUser::write(writer);

  writer.write("num-players", static_cast<int>(player_controllers.size()));
}

/* EOF */
