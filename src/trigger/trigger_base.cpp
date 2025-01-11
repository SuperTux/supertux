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
#include "util/reader_mapping.hpp"

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
  MovingObject(reader),
  m_trigger_direction(Direction::AUTO)
{
  set_group(COLGROUP_TOUCHABLE);

  if (m_col.m_bbox.get_width() == 0.f)
    m_col.m_bbox.set_width(32.f);

  if (m_col.m_bbox.get_height() == 0.f)
    m_col.m_bbox.set_height(32.f);

  std::string trigger_direction;
  reader.get("trigger-direction", trigger_direction, dir_to_string(Direction::AUTO).c_str());
  m_trigger_direction = string_to_dir(trigger_direction);
}

ObjectSettings
Trigger::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();
  auto available_directions = { Direction::AUTO, Direction::LEFT, Direction::RIGHT, Direction::UP, Direction::DOWN };
  result.add_direction(_("Trigger Direction"), &m_trigger_direction, 
                       available_directions, "trigger-direction");

  return result;
}

bool
Trigger::is_triggering_for_object(const MovingObject& object) const
{
  if(m_trigger_direction == Direction::AUTO)
    return true;

  const auto& bbox = get_bbox();
  const auto& object_bbox = object.get_bbox();
  Direction object_direction = Direction::NONE;

  if (object_bbox.get_left() < bbox.get_left())
    object_direction = Direction::LEFT;
  if (object_bbox.get_right() > bbox.get_right())
    object_direction = Direction::RIGHT;
  if (object_bbox.get_top() < bbox.get_top())
    object_direction = Direction::UP;
  if (object_bbox.get_bottom() > bbox.get_bottom())
    object_direction = Direction::DOWN;

  return object_direction == m_trigger_direction;
}

void
Trigger::draw_debug(DrawingContext& context, const Color& color)
{
  if (!Editor::is_active() && !g_debug.show_collision_rects)
    return;

  const auto& bbox = m_col.m_bbox;
  const float transparency = 0.0f;
  const int layer = LAYER_OBJECTS;

  context.color().draw_filled_rect(bbox, color, transparency, layer);

  if (!Editor::is_active() || m_trigger_direction == Direction::NONE)
    return;

  const float indicator_width = 2.0f;
  const float indicator_length = 50.0f;
  std::vector<Rectf> indicators;

  Rectf left_indicator = Rectf(bbox.get_left() - indicator_length,
                                bbox.get_middle().y - indicator_width, 
                                bbox.get_left(),
                                bbox.get_middle().y + indicator_width);
  
  Rectf right_indicator = Rectf(bbox.get_right(),
                                bbox.get_middle().y - indicator_width,
                                bbox.get_right() + indicator_length,
                                bbox.get_middle().y + indicator_width);

  Rectf up_indicator = Rectf(bbox.get_middle().x - indicator_width,
                              bbox.get_top() - indicator_length, 
                              bbox.get_middle().x + indicator_width,
                              bbox.get_top());

  Rectf down_indicator = Rectf(bbox.get_middle().x - indicator_width,
                                bbox.get_bottom(), 
                                bbox.get_middle().x + indicator_width,
                                bbox.get_bottom() + indicator_length);

  if(m_trigger_direction == Direction::LEFT  || m_trigger_direction == Direction::AUTO)
    indicators.push_back(left_indicator);
  if(m_trigger_direction == Direction::RIGHT || m_trigger_direction == Direction::AUTO)
    indicators.push_back(right_indicator);
  if(m_trigger_direction == Direction::UP    || m_trigger_direction == Direction::AUTO)
    indicators.push_back(up_indicator);
  if(m_trigger_direction == Direction::DOWN  || m_trigger_direction == Direction::AUTO)
    indicators.push_back(down_indicator);
  
  for(const auto& dir_indicator : indicators)
  {
    context.color().draw_filled_rect(dir_indicator, color, transparency, layer);
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

/* EOF */
