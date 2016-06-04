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

#include "badguy/yeti_stalactite.hpp"

#include "editor/editor.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

static const float YT_SHAKE_TIME = .8f;

YetiStalactite::YetiStalactite(const ReaderMapping& lisp)
  : Stalactite(lisp)
{
}

YetiStalactite::~YetiStalactite()
{
}

void
YetiStalactite::start_shaking()
{
  timer.start(YT_SHAKE_TIME);
  state = STALACTITE_SHAKING;
  if(((int)get_pos().x / 32) % 2 == 0) {
    physic.set_velocity_y(100);
  }
}

bool
YetiStalactite::is_hanging() const
{
  return state == STALACTITE_HANGING;
}

void
YetiStalactite::active_update(float elapsed_time)
{
  if(state == STALACTITE_HANGING)
    return;

  Stalactite::active_update(elapsed_time);
}

void
YetiStalactite::update(float elapsed_time)
{
  if (Editor::is_active() && sprite->get_action() != "yeti-stalactite" &&
      sprite->has_action("yeti-stalactite")) {
    sprite->set_action("yeti-stalactite");
  }

  // Respawn instead of removing once squished
  if(get_state() == STATE_SQUISHED && check_state_timer()) {
    set_state(STATE_ACTIVE);
    state = STALACTITE_HANGING;
    // Hopefully we shouldn't come into contact with anything...
    sprite->set_action("normal");
    set_pos(start_position);
    set_colgroup_active(COLGROUP_TOUCHABLE);
  }

  // Call back to badguy to do normal stuff
  BadGuy::update(elapsed_time);
}

bool
YetiStalactite::is_flammable() const
{
  return false;
}

/* EOF */
