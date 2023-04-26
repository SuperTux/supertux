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

#ifdef DOXYGEN_SCRIPTING
/**
 * @summary The ""Level"" class provides basic controlling functions for the current level.
 * @instances An instance named ""Level"" is available from scripts and the console.
 */
class Level
{
public:
#endif

/**
 * Ends the current level.
 * @param bool $win If ""true"", the level is marked as completed if launched from a worldmap.
 */
void Level_finish(bool win);

/**
 * Respawns Tux in sector named ""sector"" at spawnpoint named ""spawnpoint"".${SRG_TABLENEWPARAGRAPH}
   Exceptions: If ""sector"" or ""spawnpoint"" are empty, or the specified sector does not exist, the function will bail out the first chance it gets.
   If the specified spawnpoint doesn't exist, Tux will be spawned at the spawnpoint named “main”.
   If that spawnpoint doesn't exist either, Tux will simply end up at the origin (top-left 0, 0).
 * @param string $sector
 * @param string $spawnpoint
 */
void Level_spawn(const std::string& sector, const std::string& spawnpoint);

/**
 * Sets the default start spawnpoint of the level.
 * @param string $sector
 * @param string $spawnpoint
 */
void Level_set_start_point(const std::string& sector, const std::string& spawnpoint);

/**
 * Sets the default start spawn position of the level.
 * @param string $sector
 * @param float $x
 * @param float $y
 */
void Level_set_start_pos(const std::string& sector, float x, float y);

/**
 * Sets the default respawn spawnpoint of the level.
 * @param string $sector
 * @param string $spawnpoint
 */
void Level_set_respawn_point(const std::string& sector, const std::string& spawnpoint);

/**
 * Sets the default respawn position of the level.
 * @param string $sector
 * @param float $x
 * @param float $y
 */
void Level_set_respawn_pos(const std::string& sector, float x, float y);

/**
 * Flips the level vertically (i.e. top is now bottom and vice versa).
   Call again to revert the effect. Make sure the player can land on something after the level is flipped!
 */
void Level_flip_vertically();

/**
 * Toggle pause.
 */
void Level_toggle_pause();

/**
 * Switch to/from edit mode
 * @param bool $edit_mode
 */
void Level_edit(bool edit_mode);

#ifdef DOXYGEN_SCRIPTING
}
#endif

} // namespace scripting

#endif

/* EOF */
