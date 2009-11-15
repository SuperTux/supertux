//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

namespace Scripting
{

    /** Instantly finish the currently played level */
    void Level_finish(bool win);
    /** spawn tux at specified sector and spawnpoint */
    void Level_spawn(const std::string& sector, const std::string& spawnpoint);
    /** Flip level vertically */
    void Level_flip_vertically();
    /** toggle pause */
    void Level_toggle_pause();

    /** Switch to and from edit mode */
    void Level_edit(bool edit_mode);
}

#endif

/* EOF */
