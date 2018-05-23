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

#include "badguy/smartblock.hpp"

#include "sprite/sprite_manager.hpp"

SmartBlock::SmartBlock(const ReaderMapping& reader) :
  MrIceBlock(reader)
{
  max_drop_height = 16;
  sprite = SpriteManager::current()->create("images/creatures/mr_iceblock/smart_block/smart_block.sprite");
}

/* EOF */
