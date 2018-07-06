//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_POINT_MARKER_HPP
#define HEADER_SUPERTUX_POINT_MARKER_HPP

#include "supertux/moving_object.hpp"

class DrawingContext;

class PointMarker : public MovingObject
{
  public:
    PointMarker(const Vector& pos);
    PointMarker();

    virtual void update(float elapsed_time) override {
      //No updates needed
    }

    virtual void draw(DrawingContext& context) override;

    void collision_solid(const CollisionHit& hit) override {
      //This function wouldn't be called anyway.
    }

    HitResponse collision(GameObject& other, const CollisionHit& hit) override {
      return FORCE_MOVE;
    }

    virtual Vector get_point_vector() const;
    virtual Vector get_offset() const;

  private:
    PointMarker(const PointMarker&);
    PointMarker& operator=(const PointMarker&);
};

#endif // HEADER_SUPERTUX_EDITOR_POINT_MARKER_HPP

/* EOF */
