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

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/debug.hpp"

TriggerBase::TriggerBase() :
  m_hit(),
  m_losetouch_listeners()
{
}

TriggerBase::~TriggerBase()
{
  // unregister remove_listener hooks, so nobody will try to call us after we've been destroyed
  for (auto& p : m_losetouch_listeners) {
    p->del_remove_listener(this);
  }
  m_losetouch_listeners.clear();
}

void
TriggerBase::update()
{
  for (unsigned i = 0; i < m_losetouch_listeners.size(); i++)
  {
    if (std::find(m_hit.begin(), m_hit.end(), m_losetouch_listeners[i]) == m_hit.end())
    {
      event(*m_losetouch_listeners[i], EVENT_LOSETOUCH);
      m_losetouch_listeners[i]->del_remove_listener(this);
      m_losetouch_listeners.erase(m_losetouch_listeners.begin() + i);
      i--;
    }
  }

  m_hit.clear();
}

HitResponse
TriggerBase::collision(MovingObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    m_hit.push_back(player);
    if (std::find(m_losetouch_listeners.begin(), m_losetouch_listeners.end(), player) == m_losetouch_listeners.end()) {
      m_losetouch_listeners.push_back(player);
      player->add_remove_listener(this);
      event(*player, EVENT_TOUCH);
    }
  }

  return ABORT_MOVE;
}

void
TriggerBase::object_removed(GameObject* object)
{
  m_losetouch_listeners.erase(std::remove(m_losetouch_listeners.begin(),
                                          m_losetouch_listeners.end(),
                                          object),
                              m_losetouch_listeners.end());
}


Trigger::Trigger(const ReaderMapping& reader) :
  MovingObject(reader)
{
  set_group(COLGROUP_TOUCHABLE);

  if (m_col.m_bbox.get_width() == 0.f)
    m_col.m_bbox.set_width(32.f);

  if (m_col.m_bbox.get_height() == 0.f)
    m_col.m_bbox.set_height(32.f);
}

void
Trigger::draw_special_filled_box(DrawingContext& context, Color color)
{
  if (Editor::is_active() || g_debug.show_collision_rects) {
    if (Editor::current()->get_triggers_visible())
    {
      context.color().draw_filled_rect(m_col.m_bbox, color, 0.0f, LAYER_OBJECTS);
    }
    else
    {
      Rectf& box = m_col.m_bbox;
      context.color().draw_line(
        { box.get_left(), box.get_top() },
        { box.get_right(), box.get_top() }, color, LAYER_OBJECTS);
      context.color().draw_line(
        { box.get_left(), box.get_top() },
        { box.get_left(), box.get_bottom() }, color, LAYER_OBJECTS);
      context.color().draw_line(
        { box.get_left(), box.get_bottom() },
        { box.get_right(), box.get_bottom() }, color, LAYER_OBJECTS);
      context.color().draw_line(
        { box.get_right(), box.get_top() },
        { box.get_right(), box.get_bottom() }, color, LAYER_OBJECTS);
    }
  }
}

SpritedTrigger::SpritedTrigger(const ReaderMapping& reader, const std::string& sprite_name, int layer) :
  MovingSprite(reader, sprite_name, layer, COLGROUP_TOUCHABLE)
{
}


StickyTrigger::StickyTrigger(const ReaderMapping& reader, const std::string& sprite_name, int layer) :
  StickyObject(reader, sprite_name, layer, COLGROUP_TOUCHABLE)
{
}
