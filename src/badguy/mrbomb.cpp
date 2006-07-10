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

#include <config.h>

#include "mrbomb.hpp"
#include "bomb.hpp"
#include "sprite/sprite_manager.hpp"

MrBomb::MrBomb(const lisp::Lisp& reader)
	: WalkingBadguy(reader, "images/creatures/mr_cherry/mr_cherry.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 0;

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/creatures/mr_cherry/mr_cherry.sprite";
    return;
  }
  //Replace sprite 
  sprite = sprite_manager->create( sprite_name );
}

/* MrBomb created by a despencer always gets default sprite atm.*/
MrBomb::MrBomb(const Vector& pos, Direction d)
	: WalkingBadguy(pos, d, "images/creatures/mr_cherry/mr_cherry.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 0;
}

void
MrBomb::write(lisp::Writer& writer)
{
  writer.start_list("mrbomb");
  WalkingBadguy::write(writer);
  writer.end_list("mrbomb");
}

bool
MrBomb::collision_squished(Player& player)
{
  remove_me();
  Sector::current()->add_object(new Bomb(get_pos(), dir, sprite_name ));
  kill_squished(player);
  return true;
}

void
MrBomb::kill_fall()
{
  remove_me();
  Bomb* bomb = new Bomb(get_pos(), dir, sprite_name );
  Sector::current()->add_object(bomb);
  bomb->explode();
}

IMPLEMENT_FACTORY(MrBomb, "mrbomb")
