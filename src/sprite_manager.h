//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SPRITE_MANAGER_HXX
#define HEADER_SPRITE_MANAGER_HXX

#include <map>
#include "sprite.h"

class SpriteManager
{
 private:
  typedef std::map<std::string, Sprite*> Sprites;
  Sprites sprites;
 public:
  SpriteManager(const std::string& filename);
  ~SpriteManager();
  
  void    load_resfile(const std::string& filename);
  /** loads a sprite.
   * WARNING: You must not delete the returned object.
   */
  Sprite* load(const std::string& name);
};

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
