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

std::list<SpriteChange*> SpriteChange::s_all_sprite_changes;

SpriteChange::SpriteChange(const ReaderMapping& mapping) :
  m_pos(0.0f, 0.0f),
  m_change_on_touch(false),
  m_sprite(),
  m_sprite_name(),
  m_stay_action(),
  m_stay_group(),
  m_in_stay_action(false)
{
  mapping.get("x", m_pos.x);
  mapping.get("y", m_pos.y);
  mapping.get("change-on-touch", m_change_on_touch);

  if (!mapping.get("sprite", m_sprite_name)) m_sprite_name = "";
  m_sprite = SpriteManager::current()->create(m_sprite_name);

  mapping.get("stay-action", m_stay_action);
  mapping.get("initial-stay-action", m_in_stay_action);

  mapping.get("stay-group", m_stay_group);

  s_all_sprite_changes.push_back(this);
}

SpriteChange::~SpriteChange()
{
  s_all_sprite_changes.remove(this);
}

void
SpriteChange::draw(DrawingContext& context)
{
  if (m_in_stay_action && !m_stay_action.empty()) {
    m_sprite->set_action(m_stay_action);
    m_sprite->draw(context.color(), m_pos * 32, LAYER_OBJECTS-1);
  }
}

void
SpriteChange::update(float )
{
}

bool
SpriteChange::show_stay_action() const
{
  return m_in_stay_action;
}

void
SpriteChange::set_stay_action()
{
  m_in_stay_action = true;
}

void
SpriteChange::clear_stay_action(bool propagate)
{
  m_in_stay_action = false;

  // if we are in a stay_group, also clear all stay actions in this group
  if (!m_stay_group.empty() && propagate) {
    for (auto& sc : s_all_sprite_changes) {
      if (sc->m_stay_group != m_stay_group) continue;
      sc->m_in_stay_action = false;
    }
  }
}

} // namespace worldmap

/* EOF */
