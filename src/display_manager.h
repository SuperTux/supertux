//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
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
#ifndef __DISPLAY_MANAGER_H__
#define __DISPLAY_MANAGER_H__

#include <vector>

#include "drawable.h"
#include "camera.h"

// some constants for predefined layer values
enum {
  LAYER_BACKGROUND0 = -300,
  LAYER_BACKGROUND1 = -200,
  LAYER_BACKGROUNDTILES = -100,
  LAYER_TILES = 0,
  LAYER_OBJECTS = 100,
  LAYER_FOREGROUNDTILES = 200,
  LAYER_FOREGROUND0 = 300,
  LAYER_FOREGROUND1 = 400
};

/** This class holds a list of all things that should be drawn to screen
 */
class DisplayManager
{
public:
  DisplayManager();
  ~DisplayManager();
  
  /** adds an object to the list of stuff that should be drawn each frame.
   * The layer argument specifies how early an object is drawn.
   */
  void add_drawable(Drawable* object, int layer);

  void remove_drawable(Drawable* object);

  void draw(Camera& camera);

private:
  class DrawingQueueEntry {
  public:
    DrawingQueueEntry(Drawable* newobject, int newlayer)
      : object(newobject), layer(newlayer)
    { }

    bool operator <(int olayer) const
    {
      return layer < olayer;
    }

    Drawable* object;
    int layer;
  };

  typedef std::vector<DrawingQueueEntry> DisplayList;
  DisplayList displaylist;
};

#endif

