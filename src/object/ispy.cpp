//  SuperTux - Ispy
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
//                2022 Jiri Palecek <narre@protonmail.com>
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

#include "object/ispy.hpp"

#include "editor/editor.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

Ispy::Ispy(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/ispy/ispy.sprite", LAYER_TILES + 5, COLGROUP_DISABLED),
  m_state(ISPYSTATE_IDLE),
  m_script(),
  m_dir(Direction::AUTO)
{
  reader.get("script", m_script);

  std::string dir_str;
  if (reader.get("direction", dir_str))
    m_dir = string_to_dir(dir_str);
  else if (!Editor::is_active())
    m_dir = Direction::LEFT;

  if (m_dir == Direction::AUTO)
    log_warning << "Setting an Ispy's direction to AUTO is no good idea." << std::endl;

  set_sprite_action("idle");
}

ObjectSettings
Ispy::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_script(_("Script"), &m_script, "script");
  result.add_direction(_("Direction"), &m_dir, Direction::AUTO, "direction");

  result.reorder({"script", "facing-down", "direction", "x", "y"});

  return result;
}

void
Ispy::after_editor_set()
{
  MovingSprite::after_editor_set();
  set_sprite_action("idle");
}

HitResponse
Ispy::collision(GameObject& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
Ispy::update(float dt_sec)
{

  if (m_state == ISPYSTATE_IDLE)
  {
    //Check if a player has been spotted
    Vector eye = m_col.m_bbox.get_middle();

    switch (m_dir)
    {
      case Direction::DOWN:  eye = Vector(m_col.m_bbox.get_middle().x, m_col.m_bbox.get_bottom());   break;
      case Direction::UP:    eye = Vector(m_col.m_bbox.get_middle().x, m_col.m_bbox.get_top());      break;
      case Direction::LEFT:  eye = Vector(m_col.m_bbox.get_left(),     m_col.m_bbox.get_middle().y); break;
      case Direction::RIGHT: eye = Vector(m_col.m_bbox.get_right(),    m_col.m_bbox.get_middle().y); break;
      default: break;
    }

    if (Sector::get().can_see_player(eye))
    {
      set_sprite_action("alert", 1);
      m_state = ISPYSTATE_ALERT;
    }
  }
  if (m_state == ISPYSTATE_ALERT)
  {
    if (m_sprite->animation_done())
    {
      set_sprite_action("hiding", 1);
      m_state = ISPYSTATE_HIDING;

      Sector::get().run_script(m_script, "Ispy");
    }
  }
  if (m_state == ISPYSTATE_HIDING)
  {
    if (m_sprite->animation_done())
    {
      set_sprite_action("showing", 1);
      m_state = ISPYSTATE_SHOWING;
    }
  }
  if (m_state == ISPYSTATE_SHOWING)
  {
    if (m_sprite->animation_done())
    {
      set_sprite_action("idle");
      m_state = ISPYSTATE_IDLE;
    }
  }
}

void
Ispy::set_sprite_action(const std::string& action, int loops)
{
  switch (m_dir)
  {
    case Direction::DOWN:  m_sprite->set_action(action + "-down",  loops); break;
    case Direction::UP:    m_sprite->set_action(action + "-up",    loops); break;
    case Direction::LEFT:  m_sprite->set_action(action + "-left",  loops); break;
    case Direction::RIGHT: m_sprite->set_action(action + "-right", loops); break;
    default: break;
  }
}

void
Ispy::on_flip(float height)
{
  MovingSprite::on_flip(height);
  if (m_dir == Direction::UP)
  {
    m_dir = Direction::DOWN;
    m_sprite->set_action("idle-down");
  }
  else if (m_dir == Direction::DOWN) {
    m_dir = Direction::UP;
    m_sprite->set_action("idle-up");
  }
}

/* EOF */
