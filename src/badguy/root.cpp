//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/root.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

static const float SPEED_GROW = 256;
static const float SPEED_SHRINK = 128;
static const float HATCH_TIME = 0.75;

Root::Root(const Vector& pos, Flip flip) :
  BadGuy(pos, "images/creatures/ghosttree/root.sprite", LAYER_TILES-1),
  mystate(STATE_APPEARING),
  base_sprite(SpriteManager::current()->create("images/creatures/ghosttree/root-base.sprite")),
  offset_y(0),
  hatch_timer()
{
  base_sprite->set_action("appearing", 1);
  base_sprite->set_animation_loops(1); // TODO: Necessary because set_action ignores loops for default actions.
  m_physic.enable_gravity(false);
  set_colgroup_active(COLGROUP_TOUCHABLE);
  m_flip = flip;
}

Root::~Root()
{
}

void
Root::deactivate()
{
  remove_me();
  // No dead-script required for deactivation.
}

void
Root::active_update(float dt_sec)
{
  if (mystate == STATE_APPEARING) {
    if (base_sprite->animation_done()) {
      hatch_timer.start(HATCH_TIME);
      mystate = STATE_HATCHING;
    }
  }
  if (mystate == STATE_HATCHING) {
    if (!hatch_timer.started()) mystate = STATE_GROWING;
  }
  else if (mystate == STATE_GROWING) {
    offset_y -= dt_sec * SPEED_GROW;
    if (offset_y < static_cast<float>(-m_sprite->get_height())) {
      offset_y = static_cast<float>(-m_sprite->get_height());
      mystate = STATE_SHRINKING;
    }
    set_pos(m_start_position + Vector(0, (m_flip == NO_FLIP ? offset_y : -offset_y)));
  }
  else if (mystate == STATE_SHRINKING) {
    offset_y += dt_sec * SPEED_SHRINK;
    if (offset_y > 0) {
      offset_y = 0;
      mystate = STATE_VANISHING;
      base_sprite->set_action("vanishing", 2);
      base_sprite->set_animation_loops(2); // TODO: Verify if setting loops to 1 works as intended.
    }
    set_pos(m_start_position + Vector(0, (m_flip == NO_FLIP ? offset_y : -offset_y)));
  }
  else if (mystate == STATE_VANISHING) {
    if (base_sprite->animation_done()) remove_me();
  }
  BadGuy::active_update(dt_sec);
}

void
Root::draw(DrawingContext& context)
{
  base_sprite->draw(context.color(), m_start_position, LAYER_TILES+1, m_flip);
  if ((mystate != STATE_APPEARING) && (mystate != STATE_VANISHING)) BadGuy::draw(context);
}

/* EOF */
