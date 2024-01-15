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

#ifndef HEADER_SUPERTUX_EDITOR_NETWORK_USER_HPP
#define HEADER_SUPERTUX_EDITOR_NETWORK_USER_HPP

#include <string>

#include "gui/mousecursor.hpp"

class ReaderMapping;
class Writer;

/** Represents a remote peer, editing a level over the network. */
class EditorNetworkUser final
{
public:
  EditorNetworkUser(const std::string& nickname);
  EditorNetworkUser(const ReaderMapping& reader);

  void write(Writer& writer) const;
  std::string serialize() const;

public:
  std::string nickname;
  std::string sector;
  MouseCursor mouse_cursor;

private:
  EditorNetworkUser(const EditorNetworkUser&) = delete;
  EditorNetworkUser& operator=(const EditorNetworkUser&) = delete;
};

#endif

/* EOF */
