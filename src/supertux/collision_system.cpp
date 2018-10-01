//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/collision_system.hpp"

#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "math/rect.hpp"
#include "object/tilemap.hpp"
#include "object/player.hpp"
#include "supertux/collision.hpp"
#include "supertux/constants.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

namespace {

const float MAX_SPEED = 16.0f;

// a small value... be careful as CD is very sensitive to it
const float DELTA = .002f;

} // namespace

CollisionSystem::CollisionSystem(Sector& sector) :
  m_sector(sector),
  m_moving_objects()
{
}

void
CollisionSystem::add(MovingObject* object)
{
  m_moving_objects.push_back(object);
}

void
CollisionSystem::remove(MovingObject* moving_object)
{
  m_moving_objects.erase(
    std::find(m_moving_objects.begin(), m_moving_objects.end(),
              moving_object));
}

void
CollisionSystem::draw(DrawingContext& context)
{
  Color color(1.0f, 0.0f, 0.0f, 0.75f);
  for(auto& object : m_moving_objects) {
    const Rectf& rect = object->get_bbox();

    context.color().draw_filled_rect(rect, color, LAYER_FOREGROUND1 + 10);
  }
}


/** r1 is supposed to be moving, r2 a solid object */
void check_collisions(collision::Constraints* constraints,
                      const Vector& obj_movement, const Rectf& obj_rect, const Rectf& other_rect,
                      GameObject* object = NULL, MovingObject* other = NULL, const Vector& other_movement = Vector(0,0))
{
  if(!collision::intersects(obj_rect, other_rect))
    return;

  auto moving_object = dynamic_cast<MovingObject*> (object);
  CollisionHit dummy;
  if(other != NULL && object != NULL && !other->collides(*object, dummy))
    return;
  if(moving_object != NULL && other != NULL && !moving_object->collides(*other, dummy))
    return;

  // calculate intersection
  float itop    = obj_rect.get_bottom() - other_rect.get_top();
  float ibottom = other_rect.get_bottom() - obj_rect.get_top();
  float ileft   = obj_rect.get_right() - other_rect.get_left();
  float iright  = other_rect.get_right() - obj_rect.get_left();

  if(fabsf(obj_movement.y) > fabsf(obj_movement.x)) {
    if(ileft < SHIFT_DELTA) {
      constraints->constrain_right(other_rect.get_left(), other_movement.x);
      return;
    } else if(iright < SHIFT_DELTA) {
      constraints->constrain_left(other_rect.get_right(), other_movement.x);
      return;
    }
  } else {
    // shiftout bottom/top
    if(itop < SHIFT_DELTA) {
      constraints->constrain_bottom(other_rect.get_top(), other_movement.y);
      return;
    } else if(ibottom < SHIFT_DELTA) {
      constraints->constrain_top(other_rect.get_bottom(), other_movement.y);
      return;
    }
  }

  constraints->ground_movement += other_movement;
  if(other != NULL && object != NULL) {
    HitResponse response = other->collision(*object, dummy);
    if(response == ABORT_MOVE)
      return;

    if(other->get_movement() != Vector(0, 0)) {
      // TODO what todo when we collide with 2 moving objects?!?
      constraints->ground_movement += other->get_movement();
    }
  }

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if(vert_penetration < horiz_penetration) {
    if(itop < ibottom) {
      constraints->constrain_bottom(other_rect.get_top(), other_movement.y);
      constraints->hit.bottom = true;
    } else {
      constraints->constrain_top(other_rect.get_bottom(), other_movement.y);
      constraints->hit.top = true;
    }
  } else {
    if(ileft < iright) {
      constraints->constrain_right(other_rect.get_left(), other_movement.x);
      constraints->hit.right = true;
    } else {
      constraints->constrain_left(other_rect.get_right(), other_movement.x);
      constraints->hit.left = true;
    }
  }
}

void
CollisionSystem::collision_tilemap(collision::Constraints* constraints,
                                   const Vector& movement, const Rectf& dest,
                                   MovingObject& object) const
{
  // calculate rectangle where the object will move
  float x1 = dest.get_left();
  float x2 = dest.get_right();
  float y1 = dest.get_top();
  float y2 = dest.get_bottom();

  for(const auto& solids : m_sector.m_solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      for(int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        // skip non-solid tiles
        if(!tile.is_solid ())
          continue;
        Rectf tile_bbox = solids->get_tile_bbox(x, y);

        /* If the tile is a unisolid tile, the "is_solid()" function above
         * didn't do a thorough check. Calculate the position and (relative)
         * movement of the object and determine whether or not the tile is
         * solid with regard to those parameters. */
        if(tile.is_unisolid ()) {
          Vector relative_movement = movement
            - solids->get_movement(/* actual = */ true);

          if (!tile.is_solid (tile_bbox, object.get_bbox(), relative_movement))
            continue;
        } /* if (tile.is_unisolid ()) */

        if(tile.is_slope ()) { // slope tile
          AATriangle triangle;
          int slope_data = tile.get_data();
          if (solids->get_flip() & VERTICAL_FLIP)
            slope_data = AATriangle::vertical_flip(slope_data);
          triangle = AATriangle(tile_bbox, slope_data);

          collision::rectangle_aatriangle(constraints, dest, triangle,
              solids->get_movement(/* actual = */ false));
        } else { // normal rectangular tile
          check_collisions(constraints, movement, dest, tile_bbox, NULL, NULL,
              solids->get_movement(/* actual = */ false));
        }
      }
    }
  }
}

uint32_t
CollisionSystem::collision_tile_attributes(const Rectf& dest, const Vector& mov) const
{
  float x1 = dest.p1.x;
  float y1 = dest.p1.y;
  float x2 = dest.p2.x;
  float y2 = dest.p2.y;

  uint32_t result = 0;
  for(auto& solids: m_sector.m_solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));
    // For ice (only), add a little fudge to recognize tiles Tux is standing on.
    Rect test_tiles_ice = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2 + SHIFT_DELTA));

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      int y;
      for(y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        if ( tile.is_collisionful( solids->get_tile_bbox(x, y), dest, mov) ) {
          result |= tile.get_attributes();
        }
      }
      for(; y < test_tiles_ice.bottom; ++y) {
        const Tile& tile = solids->get_tile(x, y);
        if ( tile.is_collisionful( solids->get_tile_bbox(x, y), dest, mov) ) {
          result |= (tile.get_attributes() & Tile::ICE);
        }
      }
    }
  }

  return result;
}

/** fills in CollisionHit and Normal vector of 2 intersecting rectangle */
static void get_hit_normal(const Rectf& r1, const Rectf& r2, CollisionHit& hit,
                           Vector& normal)
{
  float itop = r1.get_bottom() - r2.get_top();
  float ibottom = r2.get_bottom() - r1.get_top();
  float ileft = r1.get_right() - r2.get_left();
  float iright = r2.get_right() - r1.get_left();

  float vert_penetration = std::min(itop, ibottom);
  float horiz_penetration = std::min(ileft, iright);
  if(vert_penetration < horiz_penetration) {
    if(itop < ibottom) {
      hit.bottom = true;
      normal.y = vert_penetration;
    } else {
      hit.top = true;
      normal.y = -vert_penetration;
    }
  } else {
    if(ileft < iright) {
      hit.right = true;
      normal.x = horiz_penetration;
    } else {
      hit.left = true;
      normal.x = -horiz_penetration;
    }
  }
}

void
CollisionSystem::collision_object(MovingObject* object1, MovingObject* object2) const
{
  using namespace collision;

  const Rectf& r1 = object1->dest;
  const Rectf& r2 = object2->dest;

  CollisionHit hit;
  if(intersects(object1->dest, object2->dest)) {
    Vector normal;
    get_hit_normal(r1, r2, hit, normal);

    if(!object1->collides(*object2, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    if(!object2->collides(*object1, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);

    HitResponse response1 = object1->collision(*object2, hit);
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    HitResponse response2 = object2->collision(*object1, hit);
    if(response1 == CONTINUE && response2 == CONTINUE) {
      normal *= (0.5f + DELTA);
      object1->dest.move(-normal);
      object2->dest.move(normal);
    } else if (response1 == CONTINUE && response2 == FORCE_MOVE) {
      normal *= (1 + DELTA);
      object1->dest.move(-normal);
    } else if (response1 == FORCE_MOVE && response2 == CONTINUE) {
      normal *= (1 + DELTA);
      object2->dest.move(normal);
    }
  }
}

void
CollisionSystem::collision_static(collision::Constraints* constraints,
                                  const Vector& movement, const Rectf& dest,
                                  MovingObject& object)
{
  collision_tilemap(constraints, movement, dest, object);

  // collision with other (static) objects
  for(auto& moving_object : m_moving_objects) {
    if(moving_object->get_group() != COLGROUP_STATIC
       && moving_object->get_group() != COLGROUP_MOVING_STATIC)
      continue;
    if(!moving_object->is_valid())
      continue;

    if(moving_object != &object)
      check_collisions(constraints, movement, dest, moving_object->bbox,
                       &object, moving_object);
  }
}

void
CollisionSystem::collision_static_constrains(MovingObject& object)
{
  using namespace collision;
  float infinity = (std::numeric_limits<float>::has_infinity ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max());

  Constraints constraints;
  Vector movement = object.get_movement();
  Vector pressure = Vector(0,0);
  Rectf& dest = object.dest;

  for(int i = 0; i < 2; ++i) {
    collision_static(&constraints, Vector(0, movement.y), dest, object);
    if(!constraints.has_constraints())
      break;

    // apply calculated horizontal constraints
    if(constraints.get_position_bottom() < infinity) {
      float height = constraints.get_height ();
      if(height < object.get_bbox().get_height()) {
        // we're crushed, but ignore this for now, we'll get this again
        // later if we're really crushed or things will solve itself when
        // looking at the vertical constraints
        pressure.y += object.get_bbox().get_height() - height;
      } else {
        dest.p2.y = constraints.get_position_bottom() - DELTA;
        dest.p1.y = dest.p2.y - object.get_bbox().get_height();
      }
    } else if(constraints.get_position_top() > -infinity) {
      dest.p1.y = constraints.get_position_top() + DELTA;
      dest.p2.y = dest.p1.y + object.get_bbox().get_height();
    }
  }
  if(constraints.has_constraints()) {
    if(constraints.hit.bottom) {
      dest.move(constraints.ground_movement);
    }
    if(constraints.hit.top || constraints.hit.bottom) {
      constraints.hit.left = false;
      constraints.hit.right = false;
      object.collision_solid(constraints.hit);
    }
  }

  constraints = Constraints();
  for(int i = 0; i < 2; ++i) {
    collision_static(&constraints, movement, dest, object);
    if(!constraints.has_constraints())
      break;

    // apply calculated vertical constraints
    float width = constraints.get_width ();
    if(width < infinity) {
      if(width + SHIFT_DELTA < object.get_bbox().get_width()) {
        // we're crushed, but ignore this for now, we'll get this again
        // later if we're really crushed or things will solve itself when
        // looking at the horizontal constraints
        pressure.x += object.get_bbox().get_width() - width;
      } else {
        float xmid = constraints.get_x_midpoint ();
        dest.p1.x = xmid - object.get_bbox().get_width()/2;
        dest.p2.x = xmid + object.get_bbox().get_width()/2;
      }
    } else if(constraints.get_position_right() < infinity) {
      dest.p2.x = constraints.get_position_right() - DELTA;
      dest.p1.x = dest.p2.x - object.get_bbox().get_width();
    } else if(constraints.get_position_left() > -infinity) {
      dest.p1.x = constraints.get_position_left() + DELTA;
      dest.p2.x = dest.p1.x + object.get_bbox().get_width();
    }
  }

  if(constraints.has_constraints()) {
    if( constraints.hit.left || constraints.hit.right
        || constraints.hit.top || constraints.hit.bottom
        || constraints.hit.crush )
      object.collision_solid(constraints.hit);
  }

  // an extra pass to make sure we're not crushed vertically
  if (pressure.y > 0) {
    constraints = Constraints();
    collision_static(&constraints, movement, dest, object);
    if(constraints.get_position_bottom() < infinity) {
      float height = constraints.get_height ();
      if(height + SHIFT_DELTA < object.get_bbox().get_height()) {
        CollisionHit h;
        h.top = true;
        h.bottom = true;
        h.crush = pressure.y > 16;
        object.collision_solid(h);
      }
    }
  }

  // an extra pass to make sure we're not crushed horizontally
  if (pressure.x > 0) {
    constraints = Constraints();
    collision_static(&constraints, movement, dest, object);
    if(constraints.get_position_right() < infinity) {
      float width = constraints.get_width ();
      if(width + SHIFT_DELTA < object.get_bbox().get_width()) {
        CollisionHit h;
        h.top = true;
        h.bottom = true;
        h.left = true;
        h.right = true;
        h.crush = pressure.x > 16;
        object.collision_solid(h);
      }
    }
  }
}

void
CollisionSystem::update()
{
  if (Editor::is_active()) {
    return;
    //Oběcts in editor shouldn't collide.
  }

  using namespace collision;

  // calculate destination positions of the objects
  for(const auto& moving_object : m_moving_objects) {
    Vector mov = moving_object->get_movement();

    // make sure movement is never faster than MAX_SPEED. Norm is pretty fat, so two addl. checks are done before.
    if (((mov.x > MAX_SPEED * M_SQRT1_2) || (mov.y > MAX_SPEED * M_SQRT1_2)) && (mov.norm() > MAX_SPEED)) {
      moving_object->movement = mov.unit() * MAX_SPEED;
      //log_debug << "Temporarily reduced object's speed of " << mov.norm() << " to " << moving_object->movement.norm() << "." << std::endl;
    }

    moving_object->dest = moving_object->get_bbox();
    moving_object->dest.move(moving_object->get_movement());
  }

  // part1: COLGROUP_MOVING vs COLGROUP_STATIC and tilemap
  for(const auto& moving_object : m_moving_objects) {
    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !moving_object->is_valid())
      continue;

    collision_static_constrains(*moving_object);
  }

  // part2: COLGROUP_MOVING vs tile attributes
  for(const auto& moving_object : m_moving_objects) {
    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC
        && moving_object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !moving_object->is_valid())
      continue;

    uint32_t tile_attributes = collision_tile_attributes(moving_object->dest, moving_object->get_movement());
    if(tile_attributes >= Tile::FIRST_INTERESTING_FLAG) {
      moving_object->collision_tile(tile_attributes);
    }
  }

  // part2.5: COLGROUP_MOVING vs COLGROUP_TOUCHABLE
  for(const auto& moving_object : m_moving_objects) {
    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC)
       || !moving_object->is_valid())
      continue;

    for(auto& moving_object_2 : m_moving_objects) {
      if(moving_object_2->get_group() != COLGROUP_TOUCHABLE
         || !moving_object_2->is_valid())
        continue;

      if(intersects(moving_object->dest, moving_object_2->dest)) {
        Vector normal;
        CollisionHit hit;
        get_hit_normal(moving_object->dest, moving_object_2->dest,
                       hit, normal);
        if(!moving_object->collides(*moving_object_2, hit))
          continue;
        if(!moving_object_2->collides(*moving_object, hit))
          continue;

        moving_object->collision(*moving_object_2, hit);
        moving_object_2->collision(*moving_object, hit);
      }
    }
  }

  // part3: COLGROUP_MOVING vs COLGROUP_MOVING
  for(auto i = m_moving_objects.begin(); i != m_moving_objects.end(); ++i) {
    auto moving_object = *i;

    if((moving_object->get_group() != COLGROUP_MOVING
        && moving_object->get_group() != COLGROUP_MOVING_STATIC)
       || !moving_object->is_valid())
      continue;

    for(auto i2 = i+1; i2 != m_moving_objects.end(); ++i2) {
      auto moving_object_2 = *i2;
      if((moving_object_2->get_group() != COLGROUP_MOVING
          && moving_object_2->get_group() != COLGROUP_MOVING_STATIC)
         || !moving_object_2->is_valid())
        continue;

      collision_object(moving_object, moving_object_2);
    }
  }

  // apply object movement
  for(const auto& moving_object : m_moving_objects) {
    moving_object->bbox = moving_object->dest;
    moving_object->movement = Vector(0, 0);
  }
}

bool
CollisionSystem::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid) const
{
  using namespace collision;

  for(const auto& solids : m_sector.m_solid_tilemaps) {
    // test with all tiles in this rectangle
    Rect test_tiles = solids->get_tiles_overlapping(rect);

    for(int x = test_tiles.left; x < test_tiles.right; ++x) {
      for(int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        if(!(tile.get_attributes() & Tile::SOLID))
          continue;
        if(tile.is_unisolid () && ignoreUnisolid)
          continue;
        if(tile.is_slope ()) {
          AATriangle triangle;
          Rectf tbbox = solids->get_tile_bbox(x, y);
          triangle = AATriangle(tbbox, tile.get_data());
          Constraints constraints;
          if(!collision::rectangle_aatriangle(&constraints, rect, triangle))
            continue;
        }
        // We have a solid tile that overlaps the given rectangle.
        return false;
      }
    }
  }

  return true;
}

bool
CollisionSystem::is_free_of_statics(const Rectf& rect, const MovingObject* ignore_object, const bool ignoreUnisolid) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect, ignoreUnisolid)) return false;

  for(const auto& moving_object : m_moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if (moving_object->get_group() == COLGROUP_STATIC) {
      if(intersects(rect, moving_object->get_bbox())) return false;
    }
  }

  return true;
}

bool
CollisionSystem::is_free_of_movingstatics(const Rectf& rect, const MovingObject* ignore_object) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect)) return false;

  for(const auto& moving_object : m_moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if ((moving_object->get_group() == COLGROUP_MOVING)
        || (moving_object->get_group() == COLGROUP_MOVING_STATIC)
        || (moving_object->get_group() == COLGROUP_STATIC)) {
      if(intersects(rect, moving_object->get_bbox())) return false;
    }
  }

  return true;
}

bool
CollisionSystem::free_line_of_sight(const Vector& line_start, const Vector& line_end, const MovingObject* ignore_object) const
{
  using namespace collision;

  // check if no tile is in the way
  float lsx = std::min(line_start.x, line_end.x);
  float lex = std::max(line_start.x, line_end.x);
  float lsy = std::min(line_start.y, line_end.y);
  float ley = std::max(line_start.y, line_end.y);
  for (float test_x = lsx; test_x <= lex; test_x += 16) {
    for (float test_y = lsy; test_y <= ley; test_y += 16) {
      for(const auto& solids : m_sector.m_solid_tilemaps) {
        const Tile& tile = solids->get_tile_at(Vector(test_x, test_y));
        // FIXME: check collision with slope tiles
        if((tile.get_attributes() & Tile::SOLID)) return false;
      }
    }
  }

  // check if no object is in the way
  for(const auto& moving_object : m_moving_objects) {
    if (moving_object == ignore_object) continue;
    if (!moving_object->is_valid()) continue;
    if ((moving_object->get_group() == COLGROUP_MOVING)
        || (moving_object->get_group() == COLGROUP_MOVING_STATIC)
        || (moving_object->get_group() == COLGROUP_STATIC)) {
      if(intersects_line(moving_object->get_bbox(), line_start, line_end)) return false;
    }
  }

  return true;
}

std::vector<MovingObject*>
CollisionSystem::get_nearby_objects (const Vector& center, float max_distance) const
{
  std::vector<MovingObject*> ret;
  std::vector<Player*> players = Sector::current()->get_players();

  for(const auto& player_ : players) {
    float distance = player_->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(player_);
  }

  for (const auto& object_ : m_moving_objects) {
    float distance = object_->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(object_);
  }

  return ret;
}

/* EOF */
