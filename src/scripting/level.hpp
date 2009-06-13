//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __LEVEL_H__
#define __LEVEL_H__

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
