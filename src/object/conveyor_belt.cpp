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

#include "sprite/sprite_manager.hpp"
#include "supertux/direction.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

ConveyorBelt::ConveyorBelt(const ReaderMapping &reader) :
    MovingObject(reader), // TODO: sprite
    m_conveyor_fast(false),
    m_direction(Direction::LEFT),
    m_running(true),
    m_sprite(SpriteManager::current()->create("images/objects/conveyor_belt/conveyor_belt.sprite")),
    m_length(1)
{
    set_group(COLGROUP_STATIC);
    reader.get("running", m_running);
    std::string dir_str;
    if (reader.get("direction", dir_str))
        m_direction = string_to_dir(dir_str);

    reader.get("fast", m_conveyor_fast);
    reader.get("length", m_length, 1);
    if (m_length <= 0)
        m_length = 1;

    m_col.m_bbox.set_size(32.0f * m_length, 32.0f);

    log_info << "Conveyor Length is " << m_length << std::endl;
}

ObjectSettings
ConveyorBelt::get_settings()
{
    ObjectSettings result = MovingObject::get_settings();

    result.add_direction(_("Direction"), &m_direction, Direction::LEFT, "direction");
    result.add_bool(_("Faster Conveyor"), &m_conveyor_fast, "fast", false);
    result.add_bool(_("Running"), &m_running, "running", true);
    result.add_int(_("Length"), &m_length, "length", 3);

    result.reorder({"running", "name", "direction", "fast", "sprite", "x", "y"});

    return result;
}

HitResponse
ConveyorBelt::collision(GameObject &other, const CollisionHit &hit)
{
    auto mo = dynamic_cast<MovingObject*>(&other);
    if (!mo || !m_running) return FORCE_MOVE;
    auto pos = mo->get_pos();
    pos.x += (m_conveyor_fast ? 4.0f : 2.0f) * (m_direction == Direction::LEFT ? -1.0f : 1.0f);
    mo->set_pos(pos);
    return FORCE_MOVE;
}

void
ConveyorBelt::update(float dt_sec)
{
}

void
ConveyorBelt::draw(DrawingContext &context)
{
    for (int i = 0; i < m_length; i++)
    {
        Vector pos = get_pos();
        pos.x += i * 32.0f;
        m_sprite->draw(context.color(), pos, get_layer());
    }
}
