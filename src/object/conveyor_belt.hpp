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

#ifndef HEADER_SUPERTUX_OBJECT_CONVEYOR_BELT_HPP
#define HEADER_SUPERTUX_OBJECT_CONVEYOR_BELT_HPP


#include "object/moving_sprite.hpp"

/** This class represents a platform that moves entities riding it */
class ConveyorBelt : public MovingSprite
{
public:
    ConveyorBelt(const ReaderMapping& reader);

    ObjectSettings get_settings() override;

    virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

    static std::string class_name() { return "conveyor-belt"; }
    virtual std::string get_class_name() const override { return class_name(); }
    static std::string display_name() { return _("Conveyor Belt"); }
    virtual std::string get_display_name() const override { return display_name(); }

private:
    Direction m_direction;
    bool m_running;

    /** true if the conveyor is fast, false if the conveyor is slow */
    bool m_conveyor_fast;



private:
    ConveyorBelt(const ConveyorBelt&) = delete;
    ConveyorBelt& operator=(const ConveyorBelt&) = delete;
};


#endif //SUPERTUX_CONVEYOR_BELT_HPP
