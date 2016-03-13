//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_SPAWNPOINT_MARKER_HPP
#define HEADER_SUPERTUX_EDITOR_SPAWNPOINT_MARKER_HPP

#include "supertux/moving_object.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"
#include "video/drawing_context.hpp"

class SpawnPoint;

class SpawnPointMarker : public MovingObject
{
  public:
    SpawnPointMarker(const ReaderMapping& lisp);
    SpawnPointMarker(const SpawnPoint* sp);
    ~SpawnPointMarker();

    void update(float elapsed_time) {
      //No updates needed
    }

    void draw(DrawingContext& context);

    void collision_solid(const CollisionHit& hit) {
      //This function wouldn't be called anyway.
    }

    HitResponse collision(GameObject& other, const CollisionHit& hit) {
      return FORCE_MOVE;
    }

    virtual std::string get_class() const {
      return "spawnpoint";
    }

    virtual ObjectSettings get_settings();

  private:
    SurfacePtr surface;

    void setup();
};

#endif // HEADER_SUPERTUX_EDITOR_SPAWNPOINT_MARKER_HPP

/* EOF */
