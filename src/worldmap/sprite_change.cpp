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

#include "worldmap/sprite_change.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

namespace worldmap {

SpriteChange::SpriteChange(const ReaderMapping& lisp) :
  pos(),
  change_on_touch(false),
  sprite(),
  sprite_name(),
  stay_action(),
  stay_group(),
  in_stay_action(false)
{
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  lisp.get("change-on-touch", change_on_touch);

  if (!lisp.get("sprite", sprite_name)) sprite_name = "";
  sprite = SpriteManager::current()->create(sprite_name);

  lisp.get("stay-action", stay_action);
  lisp.get("initial-stay-action", in_stay_action);

  lisp.get("stay-group", stay_group);

  all_sprite_changes.push_back(this);
}

SpriteChange::~SpriteChange()
{
  all_sprite_changes.remove(this);
}

void
SpriteChange::draw(DrawingContext& context)
{
  if(in_stay_action && !stay_action.empty()) {
    sprite->set_action(stay_action);
    sprite->draw(context.color(), pos * 32, LAYER_OBJECTS-1);
  }
}

void
SpriteChange::update(float )
{
}

bool
SpriteChange::show_stay_action() const
{
  return in_stay_action;
}

void
SpriteChange::set_stay_action()
{
  in_stay_action = true;
}

void
SpriteChange::clear_stay_action(bool propagate)
{
  in_stay_action = false;

  // if we are in a stay_group, also clear all stay actions in this group
  if (!stay_group.empty() && propagate) {
    for (auto& sc : all_sprite_changes) {
      if (sc->stay_group != stay_group) continue;
      sc->in_stay_action = false;
    }
  }
}

std::list<SpriteChange*> SpriteChange::all_sprite_changes;

}

/* EOF */
