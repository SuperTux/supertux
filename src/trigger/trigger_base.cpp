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

#include "trigger/trigger_base.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"

TriggerBase::TriggerBase() :
  sprite(),
  lasthit(false),
  hit(false),
  losetouch_listeners()
{
  set_group(COLGROUP_TOUCHABLE);
}

TriggerBase::~TriggerBase()
{
  // unregister remove_listener hooks, so nobody will try to call us after we've been destroyed
  for (auto& p : losetouch_listeners) {
    p->del_remove_listener(this);
  }
  losetouch_listeners.clear();
}

void
TriggerBase::update(float )
{
  if (lasthit && !hit) {
    for (auto& p : losetouch_listeners) {
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
  if(!sprite.get())
    return;

  sprite->draw(context.color(), get_pos(), LAYER_TILES+1);
}

HitResponse
TriggerBase::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*> (&other);
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
  for (auto i = losetouch_listeners.begin(); i != losetouch_listeners.end(); ++i) {
    auto p = *i;
    if (p == object) {
      losetouch_listeners.erase(i);
      break;
    }
  }
}

/* EOF */
