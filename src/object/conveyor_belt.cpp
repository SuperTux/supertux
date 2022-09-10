//  SuperTux
//  Copyright (C) 2022 Raoul1808 <raoulthegeek@gmail.com>
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

#include "object/conveyor_belt.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/direction.hpp"
#include "util/log.hpp"
#include "math/util.hpp"
#include "util/reader_mapping.hpp"

ConveyorBelt::ConveyorBelt(const ReaderMapping &reader) :
    MovingObject(reader), // TODO: sprite
    ExposedObject<ConveyorBelt, scripting::ConveyorBelt>(this),
    m_running(true),
    m_direction(Direction::LEFT),
    m_length(1),
    m_speed(1.0f),
    m_frame(0.0f),
    m_frame_index(0),
    m_sprite(SpriteManager::current()->create("images/objects/conveyor_belt/conveyor.sprite"))
{
    set_group(COLGROUP_STATIC);
    reader.get("running", m_running);
    std::string dir_str;
    if (reader.get("direction", dir_str))
        m_direction = string_to_dir(dir_str);

    reader.get("speed", m_speed);
    m_speed = math::clamp(m_speed, 0.0f, MAX_SPEED);

    reader.get("length", m_length, 1);
    if (m_length <= 0)
        m_length = 1;

    m_col.m_bbox.set_size(32.0f * static_cast<float>(m_length), 32.0f);

    if (!m_running)
        m_sprite->set_action("stopped");
    else
        m_sprite->set_action((m_direction == Direction::LEFT ? "left" : "right"));
}

ObjectSettings
ConveyorBelt::get_settings()
{
    ObjectSettings result = MovingObject::get_settings();

    result.add_direction(_("Direction"), &m_direction, Direction::LEFT, "direction");
    result.add_float(_("Speed"), &m_speed, "speed", 1.0f);
    result.add_bool(_("Running"), &m_running, "running", true);
    result.add_int(_("Length"), &m_length, "length", 3);

    result.reorder({"running", "name", "direction", "fast", "length", "sprite", "x", "y"});

    return result;
}

HitResponse
ConveyorBelt::collision(GameObject &other, const CollisionHit &hit)
{
    return FORCE_MOVE;
}

void
ConveyorBelt::update(float dt_sec)
{
    if (m_running)
    {
        Vector shift_movement(m_speed * (m_direction == Direction::LEFT ? -1.0f : 1.0f) * 32.0f * dt_sec, 0.0f);
        m_col.propagate_movement(shift_movement);

        m_frame += (m_speed * static_cast<float>(m_sprite->get_frames())) * dt_sec;

        while (m_frame >= 1.0f)
        {
            m_frame -= 1.0f;
            m_frame_index++;
        }

        while (m_frame_index >= m_sprite->get_frames())
        {
            m_frame_index -= m_sprite->get_frames();
        }
    }
}

void
ConveyorBelt::draw(DrawingContext &context)
{
    int frame_index = m_running ? m_frame_index : 0;
    for (int i = 0; i < m_length; i++)
    {
        m_sprite->set_frame(frame_index);
        Vector pos = get_pos();
        pos.x += static_cast<float>(i) * 32.0f;
        m_sprite->draw(context.color(), pos, get_layer());
    }
}

void
ConveyorBelt::start()
{
    m_running = true;
    m_sprite->set_action((m_direction == Direction::LEFT ? "left" : "right"));
}

void
ConveyorBelt::stop()
{
    m_running = false;
    m_sprite->set_action("stopped");
}

void
ConveyorBelt::move_left()
{
    m_direction = Direction::LEFT;
    if (m_running)
        m_sprite->set_action("left");
}

void
ConveyorBelt::move_right()
{
    m_direction = Direction::RIGHT;
    if (m_running)
        m_sprite->set_action("right");
}

void
ConveyorBelt::set_speed(float target_speed)
{
    target_speed = math::clamp(target_speed, 0.0f, MAX_SPEED);
    m_speed = target_speed;
}

/* EOF */
