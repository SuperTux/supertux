//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
#ifndef __TILEMAP_H__
#define __TILEMAP_H__

#include "game_object.h"
#include "drawable.h"

class Level;

/**
 * This class is reponsible for drawing the level tiles
 */
class TileMap : public GameObject, public Drawable
{
public:
  TileMap(DisplayManager& manager, Level* level);
  virtual ~TileMap();

  virtual void action(float elapsed_time);
  virtual void draw(Camera& viewport, int layer);
  
private:
  Level* level;
};

#endif

