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

#include "trigger_base.hpp"

#include "sprite/sprite.hpp"
#include "video/drawing_request.hpp"
#include "object/player.hpp"

TriggerBase::TriggerBase()
  : sprite(0), lasthit(false), hit(false)
{
  set_group(COLGROUP_TOUCHABLE);
}

TriggerBase::~TriggerBase()
{
  // unregister remove_listener hooks, so nobody will try to call us after we've been destroyed
  for (std::list<Player*>::iterator i = losetouch_listeners.begin(); i != losetouch_listeners.end(); i++) {
    Player* p = *i;
    p->del_remove_listener(this);
  }
  losetouch_listeners.clear();
}

void
TriggerBase::update(float )
{
  if (lasthit && !hit) {
    for (std::list<Player*>::iterator i = losetouch_listeners.begin(); i != losetouch_listeners.end(); i++) {
      Player* p = *i;
      event(*p, EVENT_LOSETOUCH);
      p->del_remove_listener(this);
    }
    losetouch_listeners.clear();
  }
  lasthit = hit;
  hit = false;
}

void
TriggerBase::draw(DrawingContext& context)
{
  if(!sprite)
    return;

  sprite->draw(context, get_pos(), LAYER_TILES+1);
}

HitResponse
TriggerBase::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    hit = true;
    if(!lasthit) {
      losetouch_listeners.push_back(player);
      player->add_remove_listener(this);
      event(*player, EVENT_TOUCH);
    }
  }

  return ABORT_MOVE;
}
  
void 
TriggerBase::object_removed(GameObject* object)
{
  for (std::list<Player*>::iterator i = losetouch_listeners.begin(); i != losetouch_listeners.end(); i++) {
    Player* p = *i;
    if (p == object) {
      losetouch_listeners.erase(i);
      break;
    }
  }
}

