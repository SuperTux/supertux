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
#include "special/sprite.h"
#include "trigger_base.h"
#include "serializable.h"
#include "timer.h"

class Door : public TriggerBase, public Serializable
{
public:
  Door(const lisp::Lisp& reader);
  Door(int x, int y, std::string sector, std::string spawnpoint);
  virtual ~Door();

  virtual void write(lisp::Writer& writer);
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual void event(Player& player, EventType type);

private:
  std::string target_sector;
  std::string target_spawnpoint;
  Sprite* sprite;
};

#endif
