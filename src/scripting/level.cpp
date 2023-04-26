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

#include "scripting/level.hpp"

#include "supertux/flip_level_transformer.hpp"
#include "supertux/game_session.hpp"

namespace scripting {

void
Level_finish(bool win)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.finish(win);
}

void
Level_spawn(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.respawn(sector, spawnpoint);
}

void
Level_set_start_point(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.set_start_point(sector, spawnpoint);
}

void
Level_set_start_pos(const std::string& sector, float x, float y)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.set_start_pos(sector, Vector(x, y));
}

void
Level_set_respawn_point(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.set_respawn_point(sector, spawnpoint);
}

void
Level_set_respawn_pos(const std::string& sector, float x, float y)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.set_respawn_pos(sector, Vector(x, y));
}

void
Level_flip_vertically()
{
  SCRIPT_GUARD_GAMESESSION;
  FlipLevelTransformer flip_transformer;
  flip_transformer.transform(game_session.get_current_level());
}

void
Level_toggle_pause()
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.toggle_pause();
}

void
Level_edit(bool edit_mode)
{
  SCRIPT_GUARD_GAMESESSION;
  game_session.set_editmode(edit_mode);
}

} // namespace scripting

/* EOF */
