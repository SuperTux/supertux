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

#include "supertux/d_scope.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/game_session.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"

namespace scripting {

void
Level_finish(bool win)
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.finish(win);
}

bool
Level_has_active_sequence()
{
  SCRIPT_GUARD_GAMESESSION(false);
  return game_session.has_active_sequence();
}

void
Level_spawn(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.respawn(sector, spawnpoint);
}

void
Level_spawn_transition(const std::string& sector, const std::string& spawnpoint, const std::string& transition)
{
  SCRIPT_GUARD_GAMESESSION();

  ScreenFade::FadeType fade_type = ScreenFade::FadeType::NONE;

  if (transition == "fade")
    fade_type = ScreenFade::FadeType::FADE;
  else if (transition == "circle")
    fade_type = ScreenFade::FadeType::CIRCLE;
  else
    log_warning << "Invalid transition type '" << transition << "'." << std::endl;

  game_session.respawn_with_fade(sector, spawnpoint, fade_type, {0.0f, 0.0f}, true);
}

void
Level_set_start_point(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.set_start_point(sector, spawnpoint);
}

void
Level_set_start_pos(const std::string& sector, float x, float y)
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.set_start_pos(sector, Vector(x, y));
}

void
Level_set_respawn_point(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.set_respawn_point(sector, spawnpoint);
}

void
Level_set_respawn_pos(const std::string& sector, float x, float y)
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.set_respawn_pos(sector, Vector(x, y));
}

void
Level_flip_vertically()
{
  SCRIPT_GUARD_GAMESESSION();
  BIND_SECTOR(::Sector::get());
  FlipLevelTransformer flip_transformer;
  flip_transformer.transform(game_session.get_current_level());
}

void
Level_toggle_pause()
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.toggle_pause();
}

void
Level_pause_target_timer()
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.set_target_timer_paused(true);
}

void
Level_resume_target_timer()
{
  SCRIPT_GUARD_GAMESESSION();
  game_session.set_target_timer_paused(false);
}

} // namespace scripting

/* EOF */
