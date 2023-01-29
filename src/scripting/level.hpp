//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2023 Vankata453
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

#ifndef HEADER_SUPERTUX_SCRIPTING_LEVEL_HPP
#define HEADER_SUPERTUX_SCRIPTING_LEVEL_HPP

#ifndef SCRIPTING_API

#include <string>

/** Macro to help easily check if there is a current GameSession and define it, if so. **/
#define SCRIPT_GUARD_GAMESESSION                        \
  if (!GameSession::current()) return;                  \
  GameSession& game_session = *GameSession::current()

#endif

namespace scripting {

/**
  NOTE: For Level functions to be binded to the Level class in the Squirrel root VM,
  they must be added to the Level class definition in "data/scripts/default.nut".
**/

/** Instantly finish the currently played level */
void Level_finish(bool win);

/** spawn tux at specified sector and spawnpoint */
void Level_spawn(const std::string& sector, const std::string& spawnpoint);

/** Set the default start spawnpoint of the level.  */
void Level_set_start_point(const std::string& sector, const std::string& spawnpoint);

/** Set the default start position of the level.  */
void Level_set_start_pos(const std::string& sector, float x, float y);

/** Set the default respawn spawnpoint of the level.  */
void Level_set_respawn_point(const std::string& sector, const std::string& spawnpoint);

/** Set the default respawn position of the level.  */
void Level_set_respawn_pos(const std::string& sector, float x, float y);

/** Flip level vertically */
void Level_flip_vertically();

/** toggle pause */
void Level_toggle_pause();

/** Switch to and from edit mode */
void Level_edit(bool edit_mode);

} // namespace scripting

#endif

/* EOF */
