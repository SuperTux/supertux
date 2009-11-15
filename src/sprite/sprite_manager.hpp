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

#ifndef HEADER_SUPERTUX_SPRITE_SPRITE_MANAGER_HPP
#define HEADER_SUPERTUX_SPRITE_SPRITE_MANAGER_HPP

#include <map>
#include <string>

class SpriteData;
class Sprite;

class SpriteManager
{
private:
  typedef std::map<std::string, SpriteData*> Sprites;
  Sprites sprites;

public:
  SpriteManager();
  ~SpriteManager();

  /** loads a sprite. */
  Sprite* create(const std::string& filename);

private:
  SpriteData* load(const std::string& filename);
};

extern SpriteManager* sprite_manager;

#endif

/* EOF */
