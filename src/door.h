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

#ifndef SUPERTUX_DOOR_H
#define SUPERTUX_DOOR_H

#include <string>

#include "video/surface.h"
#include "interactive_object.h"
#include "serializable.h"
#include "special/timer.h"

namespace SuperTux {
class Sprite;
class LispReader;
}

/** data images */
#define DOOR_OPENING_TIME 1500
#define DOOR_OPENING_FRAMES 8
extern Sprite* door;
extern Surface* door_opening[DOOR_OPENING_FRAMES];

class Door : public InteractiveObject, public Serializable
{
public:
  Door(LispReader& reader);
  Door(int x, int y);
  virtual ~Door();

  virtual void write(LispWriter& writer);
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual void interaction(InteractionType type);

private:
  std::string target_sector;
  std::string target_spawnpoint;
  Timer animation_timer; //Used for door animation
  bool door_activated;
};

#endif /*SUPERTUX_DOOR_H*/

