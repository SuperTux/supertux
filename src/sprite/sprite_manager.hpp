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

#ifndef SUPERTUX_SPRITE_MANAGER_H
#define SUPERTUX_SPRITE_MANAGER_H

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
