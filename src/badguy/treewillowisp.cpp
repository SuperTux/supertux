//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2007 Matthias Braun
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

#include "badguy/treewillowisp.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "badguy/dispenser.hpp"
#include "badguy/ghosttree.hpp"
#include "math/util.hpp"
#include "object/lantern.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"

static const std::string SOUND = "sounds/willowisp.wav";

TreeWillOWisp::TreeWillOWisp(GhostTree* tree_, const Vector& pos,
                             float radius_, float speed_) :
  BadGuy(tree_->get_pos() + pos, "images/creatures/willowisp/willowisp.sprite",
         tree_->get_layer() - 1),
  m_state(STATE_DEFAULT),
  m_color(),
  m_angle(0),
  m_radius(radius_),
  m_speed(speed_),
  m_sound(),
  m_tree(tree_),
  m_suck_target(0.0f, 0.0f),
  m_suck_timer()
{
  SoundManager::current()->preload(SOUND);
  set_colgroup_active(COLGROUP_DISABLED);
  m_physic.enable_gravity(false);
}

TreeWillOWisp::~TreeWillOWisp()
{
}

void
TreeWillOWisp::activate()
{
  m_sound = SoundManager::current()->create_sound_source(SOUND);
  m_sound->set_position(get_pos());
  m_sound->set_looping(true);
  m_sound->set_gain(1.0f);
  m_sound->set_reference_distance(32);
  m_sound->play();
}

void
TreeWillOWisp::vanish()
{
  m_state = STATE_VANISHING;
  m_layer = m_tree->get_layer() + 1;
  set_action("vanishing", 1);

  if (m_parent_dispenser != nullptr)
  {
    m_parent_dispenser->notify_dead();
  }
}

void
TreeWillOWisp::start_sucking(const Vector& suck_target_, float duration)
{
  m_state = STATE_SUCKED;
  m_layer = m_tree->get_layer() + 1;
  m_suck_start = get_pos();
  m_suck_target = suck_target_;
  m_suck_timer.start(duration);
}

void
TreeWillOWisp::fly_away(const Vector& from)
{
  m_state = STATE_FLY_AWAY;
  m_layer = m_tree->get_layer() + 1;

  m_fly_dir = glm::normalize(get_pos() - from) * 30.f;
}

void
TreeWillOWisp::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer);
  if (m_layer > m_tree->get_layer())
    m_sprite->draw(context.light(), get_pos(), m_layer);
}

void
TreeWillOWisp::active_update(float dt_sec)
{
  switch (m_state) {
    case STATE_VANISHING:
      // Remove the TreeWillOWisp if it has completely vanished.
      if (m_sprite->animation_done()) {
        remove_me();
      }
      break;

    case STATE_SUCKED:
    {
      Vector dir = m_suck_target - m_suck_start;
      double mult = QuinticEaseOut(static_cast<double>(m_suck_timer.get_progress()));
      set_pos(m_suck_start + (dir * mult));

      if (m_suck_timer.check())
      {
        m_state = STATE_IDLE;
        m_tree->willowisp_suck_finished(this);
      }
      break;
    }

    case STATE_IDLE:
      break;

    case STATE_DEFAULT:
    {
      m_angle = fmodf(m_angle + dt_sec * m_speed, math::TAU);
      Vector newpos(m_start_position + Vector(sinf(m_angle) * m_radius, 0));
      m_col.set_movement(newpos - get_pos());
      float sizemod = cosf(m_angle) * 0.8f;
      /* TODO: Modify sprite size using the 'sizeMod' value. */

      if (sizemod < 0) {
        m_layer = m_tree->get_layer() - 1;
      } else {
        m_layer = m_tree->get_layer() + 1;
      }
      break;
    }

    case STATE_FLY_AWAY:
      m_col.set_movement(m_fly_dir);
      break;
  }

  m_sound->set_position(get_pos());
}

void
TreeWillOWisp::set_color(const Color& color_)
{
  m_color = color_;
  m_sprite->set_color(color_);
}

void TreeWillOWisp::stop_looping_sounds()
{
  if (m_sound) {
    m_sound->stop();
  }
}

void TreeWillOWisp::play_looping_sounds()
{
  if (m_sound) {
    m_sound->play();
  }
}

/* EOF */
