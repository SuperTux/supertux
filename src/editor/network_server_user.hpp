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

#ifndef HEADER_SUPERTUX_EDITOR_NETWORK_SERVER_USER_HPP
#define HEADER_SUPERTUX_EDITOR_NETWORK_SERVER_USER_HPP

#include "network/server_user.hpp"

#include "gui/mousecursor.hpp"

/** Represents a remote server user, editing a level over the network. */
class EditorServerUser final : public network::ServerUser
{
public:
  EditorServerUser(const std::string& nickname, Color nickname_color = Color(1, 1, 1, 1));
  EditorServerUser(const ReaderMapping& reader);

  void write(Writer& writer) const override;

public:
  std::string sector;
  MouseCursor mouse_cursor;

private:
  EditorServerUser(const EditorServerUser&) = delete;
  EditorServerUser& operator=(const EditorServerUser&) = delete;
};

#endif

/* EOF */
