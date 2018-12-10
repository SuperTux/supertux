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

#include "collision/collision_system.hpp"

#include "collision/collision.hpp"
#include "editor/editor.hpp"
#include "math/aatriangle.hpp"
#include "math/rect.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "supertux/constants.hpp"
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
  m_objects()
{
}

void
CollisionSystem::add(CollisionObject* object)
{
  m_objects.push_back(object);
}

void
CollisionSystem::remove(CollisionObject* object)
{
  m_objects.erase(
    std::find(m_objects.begin(), m_objects.end(),
              object));
}

void
CollisionSystem::draw(DrawingContext& context)
{
  const Color color(1.0f, 0.0f, 0.0f, 0.75f);

  for (auto& object : m_objects) {
    const Rectf& rect = object->get_bbox();

    context.color().draw_filled_rect(rect, color, LAYER_FOREGROUND1 + 10);
  }
}

namespace {

/** r1 is supposed to be moving, r2 a solid object */
void check_collisions(collision::Constraints* constraints,
                      const Vector& obj_movement, const Rectf& obj_rect, const Rectf& other_rect,
                      CollisionObject* object = nullptr, CollisionObject* other = nullptr, const Vector& other_movement = Vector(0,0))
{
  if (!collision::intersects(obj_rect, other_rect))
    return;

  const CollisionHit dummy;
  if (other != nullptr && object != nullptr && !other->collides(*object, dummy))
    return;
  if (object != nullptr && other != nullptr && !object->collides(*other, dummy))
    return;

  // calculate intersection
  const float itop    = obj_rect.get_bottom() - other_rect.get_top();
  const float ibottom = other_rect.get_bottom() - obj_rect.get_top();
  const float ileft   = obj_rect.get_right() - other_rect.get_left();
  const float iright  = other_rect.get_right() - obj_rect.get_left();

  if (fabsf(obj_movement.y) > fabsf(obj_movement.x)) {
    if (ileft < SHIFT_DELTA) {
      constraints->constrain_right(other_rect.get_left(), other_movement.x);
      return;
    } else if (iright < SHIFT_DELTA) {
      constraints->constrain_left(other_rect.get_right(), other_movement.x);
      return;
    }
  } else {
    // shiftout bottom/top
    if (itop < SHIFT_DELTA) {
      constraints->constrain_bottom(other_rect.get_top(), other_movement.y);
      return;
    } else if (ibottom < SHIFT_DELTA) {
      constraints->constrain_top(other_rect.get_bottom(), other_movement.y);
      return;
    }
  }

  constraints->ground_movement += other_movement;
  if (other != nullptr && object != nullptr) {
    const HitResponse response = other->collision(*object, dummy);
    if (response == ABORT_MOVE)
      return;

    if (other->get_movement() != Vector(0, 0)) {
      // TODO what todo when we collide with 2 moving objects?!?
      constraints->ground_movement += other->get_movement();
    }
  }

  const float vert_penetration = std::min(itop, ibottom);
  const float horiz_penetration = std::min(ileft, iright);

  if (vert_penetration < horiz_penetration) {
    if (itop < ibottom) {
      constraints->constrain_bottom(other_rect.get_top(), other_movement.y);
      constraints->hit.bottom = true;
    } else {
      constraints->constrain_top(other_rect.get_bottom(), other_movement.y);
      constraints->hit.top = true;
    }
  } else {
    if (ileft < iright) {
      constraints->constrain_right(other_rect.get_left(), other_movement.x);
      constraints->hit.right = true;
    } else {
      constraints->constrain_left(other_rect.get_right(), other_movement.x);
      constraints->hit.left = true;
    }
  }
}

} // namespace

void
CollisionSystem::collision_tilemap(collision::Constraints* constraints,
                                   const Vector& movement, const Rectf& dest,
                                   CollisionObject& object) const
{
  // calculate rectangle where the object will move
  const float x1 = dest.get_left();
  const float x2 = dest.get_right();
  const float y1 = dest.get_top();
  const float y2 = dest.get_bottom();

  for (const auto& solids : m_sector.get_solid_tilemaps())
  {
    // test with all tiles in this rectangle
    const Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));

    for (int x = test_tiles.left; x < test_tiles.right; ++x)
    {
      for (int y = test_tiles.top; y < test_tiles.bottom; ++y)
      {
        const Tile& tile = solids->get_tile(x, y);

        // skip non-solid tiles
        if (!tile.is_solid ())
          continue;
        Rectf tile_bbox = solids->get_tile_bbox(x, y);

        /* If the tile is a unisolid tile, the "is_solid()" function above
         * didn't do a thorough check. Calculate the position and (relative)
         * movement of the object and determine whether or not the tile is
         * solid with regard to those parameters. */
        if (tile.is_unisolid ()) {
          Vector relative_movement = movement
            - solids->get_movement(/* actual = */ true);

          if (!tile.is_solid (tile_bbox, object.get_bbox(), relative_movement))
            continue;
        }

        if (tile.is_slope ()) { // slope tile
          AATriangle triangle;
          int slope_data = tile.get_data();
          if (solids->get_flip() & VERTICAL_FLIP)
            slope_data = AATriangle::vertical_flip(slope_data);
          triangle = AATriangle(tile_bbox, slope_data);

          collision::rectangle_aatriangle(constraints, dest, triangle,
              solids->get_movement(/* actual = */ false));
        } else { // normal rectangular tile
          check_collisions(constraints, movement, dest, tile_bbox, nullptr, nullptr,
              solids->get_movement(/* actual = */ false));
        }
      }
    }
  }
}

uint32_t
CollisionSystem::collision_tile_attributes(const Rectf& dest, const Vector& mov) const
{
  const float x1 = dest.get_left();
  const float y1 = dest.get_top();
  const float x2 = dest.get_right();
  const float y2 = dest.get_bottom();

  uint32_t result = 0;
  for (auto& solids: m_sector.get_solid_tilemaps())
  {
    // test with all tiles in this rectangle
    const Rect test_tiles = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2));

    // For ice (only), add a little fudge to recognize tiles Tux is standing on.
    const Rect test_tiles_ice = solids->get_tiles_overlapping(Rectf(x1, y1, x2, y2 + SHIFT_DELTA));

    for (int x = test_tiles.left; x < test_tiles.right; ++x) {
      int y;
      for (y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        if ( tile.is_collisionful( solids->get_tile_bbox(x, y), dest, mov) ) {
          result |= tile.get_attributes();
        }
      }
      for (; y < test_tiles_ice.bottom; ++y) {
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
  const float itop = r1.get_bottom() - r2.get_top();
  const float ibottom = r2.get_bottom() - r1.get_top();
  const float ileft = r1.get_right() - r2.get_left();
  const float iright = r2.get_right() - r1.get_left();

  const float vert_penetration = std::min(itop, ibottom);
  const float horiz_penetration = std::min(ileft, iright);

  if (vert_penetration < horiz_penetration) {
    if (itop < ibottom) {
      hit.bottom = true;
      normal.y = vert_penetration;
    } else {
      hit.top = true;
      normal.y = -vert_penetration;
    }
  } else {
    if (ileft < iright) {
      hit.right = true;
      normal.x = horiz_penetration;
    } else {
      hit.left = true;
      normal.x = -horiz_penetration;
    }
  }
}

void
CollisionSystem::collision_object(CollisionObject* object1, CollisionObject* object2) const
{
  using namespace collision;

  const Rectf& r1 = object1->m_dest;
  const Rectf& r2 = object2->m_dest;

  CollisionHit hit;
  if (intersects(object1->m_dest, object2->m_dest)) {
    Vector normal;
    get_hit_normal(r1, r2, hit, normal);

    if (!object1->collides(*object2, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    if (!object2->collides(*object1, hit))
      return;
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);

    HitResponse response1 = object1->collision(*object2, hit);
    std::swap(hit.left, hit.right);
    std::swap(hit.top, hit.bottom);
    HitResponse response2 = object2->collision(*object1, hit);
    if (response1 == CONTINUE && response2 == CONTINUE) {
      normal *= (0.5f + DELTA);
      object1->m_dest.move(-normal);
      object2->m_dest.move(normal);
    } else if (response1 == CONTINUE && response2 == FORCE_MOVE) {
      normal *= (1 + DELTA);
      object1->m_dest.move(-normal);
    } else if (response1 == FORCE_MOVE && response2 == CONTINUE) {
      normal *= (1 + DELTA);
      object2->m_dest.move(normal);
    }
  }
}

void
CollisionSystem::collision_static(collision::Constraints* constraints,
                                  const Vector& movement, const Rectf& dest,
                                  CollisionObject& object)
{
  collision_tilemap(constraints, movement, dest, object);

  // collision with other (static) objects
  for (auto& static_object : m_objects)
  {
    if (static_object->get_group() != COLGROUP_STATIC &&
        static_object->get_group() != COLGROUP_MOVING_STATIC)
      continue;
    if (!static_object->is_valid())
      continue;

    if (static_object != &object) {
      check_collisions(constraints, movement, dest, static_object->m_bbox,
                       &object, static_object);
    }
  }
}

void
CollisionSystem::collision_static_constrains(CollisionObject& object)
{
  using namespace collision;

  const float infinity = (std::numeric_limits<float>::has_infinity ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::max());

  Constraints constraints;
  const Vector movement = object.get_movement();
  Vector pressure = Vector(0,0);
  Rectf& dest = object.m_dest;

  for (int i = 0; i < 2; ++i) {
    collision_static(&constraints, Vector(0, movement.y), dest, object);
    if (!constraints.has_constraints())
      break;

    // apply calculated horizontal constraints
    if (constraints.get_position_bottom() < infinity) {
      float height = constraints.get_height ();
      if (height < object.get_bbox().get_height()) {
        // we're crushed, but ignore this for now, we'll get this again
        // later if we're really crushed or things will solve itself when
        // looking at the vertical constraints
        pressure.y += object.get_bbox().get_height() - height;
      } else {
        dest.set_bottom(constraints.get_position_bottom() - DELTA);
        dest.set_top(dest.get_bottom() - object.get_bbox().get_height());
      }
    } else if (constraints.get_position_top() > -infinity) {
      dest.set_top(constraints.get_position_top() + DELTA);
      dest.set_bottom(dest.get_top() + object.get_bbox().get_height());
    }
  }

  if (constraints.has_constraints()) {
    if (constraints.hit.bottom) {
      dest.move(constraints.ground_movement);
    }

    if (constraints.hit.top || constraints.hit.bottom) {
      constraints.hit.left = false;
      constraints.hit.right = false;
      object.collision_solid(constraints.hit);
    }
  }

  constraints = Constraints();
  for (int i = 0; i < 2; ++i) {
    collision_static(&constraints, movement, dest, object);
    if (!constraints.has_constraints())
      break;

    // apply calculated vertical constraints
    const float width = constraints.get_width();

    if (width < infinity) {
      if (width + SHIFT_DELTA < object.get_bbox().get_width()) {
        // we're crushed, but ignore this for now, we'll get this again
        // later if we're really crushed or things will solve itself when
        // looking at the horizontal constraints
        pressure.x += object.get_bbox().get_width() - width;
      } else {
        float xmid = constraints.get_x_midpoint ();
        dest.set_left(xmid - object.get_bbox().get_width()/2);
        dest.set_right(xmid + object.get_bbox().get_width()/2);
      }
    } else if (constraints.get_position_right() < infinity) {
      dest.set_right(constraints.get_position_right() - DELTA);
      dest.set_left(dest.get_right() - object.get_bbox().get_width());
    } else if (constraints.get_position_left() > -infinity) {
      dest.set_left(constraints.get_position_left() + DELTA);
      dest.set_right(dest.get_left() + object.get_bbox().get_width());
    }
  }

  if (constraints.has_constraints()) {
    if ( constraints.hit.left || constraints.hit.right
        || constraints.hit.top || constraints.hit.bottom
        || constraints.hit.crush )
      object.collision_solid(constraints.hit);
  }

  // an extra pass to make sure we're not crushed vertically
  if (pressure.y > 0) {
    constraints = Constraints();
    collision_static(&constraints, movement, dest, object);
    if (constraints.get_position_bottom() < infinity) {
      const float height = constraints.get_height ();

      if (height + SHIFT_DELTA < object.get_bbox().get_height()) {
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
    if (constraints.get_position_right() < infinity) {
      float width = constraints.get_width ();
      if (width + SHIFT_DELTA < object.get_bbox().get_width()) {
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
  for (const auto& object : m_objects)
  {
    const Vector mov = object->get_movement();

    // make sure movement is never faster than MAX_SPEED. Norm is pretty fat, so two addl. checks are done before.
    if (((mov.x > MAX_SPEED * static_cast<float>(M_SQRT1_2)) || (mov.y > MAX_SPEED * static_cast<float>(M_SQRT1_2))) && (mov.norm() > MAX_SPEED)) {
      object->m_movement = mov.unit() * MAX_SPEED;
      //log_debug << "Temporarily reduced object's speed of " << mov.norm() << " to " << object->movement.norm() << "." << std::endl;
    }

    object->m_dest = object->get_bbox();
    object->m_dest.move(object->get_movement());
  }

  // part1: COLGROUP_MOVING vs COLGROUP_STATIC and tilemap
  for (const auto& object : m_objects) {
    if ((object->get_group() != COLGROUP_MOVING
        && object->get_group() != COLGROUP_MOVING_STATIC
        && object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !object->is_valid())
      continue;

    collision_static_constrains(*object);
  }

  // part2: COLGROUP_MOVING vs tile attributes
  for (const auto& object : m_objects) {
    if ((object->get_group() != COLGROUP_MOVING
        && object->get_group() != COLGROUP_MOVING_STATIC
        && object->get_group() != COLGROUP_MOVING_ONLY_STATIC)
       || !object->is_valid())
      continue;

    uint32_t tile_attributes = collision_tile_attributes(object->m_dest, object->get_movement());
    if (tile_attributes >= Tile::FIRST_INTERESTING_FLAG) {
      object->collision_tile(tile_attributes);
    }
  }

  // part2.5: COLGROUP_MOVING vs COLGROUP_TOUCHABLE
  for (const auto& object : m_objects)
  {
    if ((object->get_group() != COLGROUP_MOVING
        && object->get_group() != COLGROUP_MOVING_STATIC)
       || !object->is_valid())
      continue;

    for (auto& object_2 : m_objects) {
      if (object_2->get_group() != COLGROUP_TOUCHABLE
         || !object_2->is_valid())
        continue;

      if (intersects(object->m_dest, object_2->m_dest)) {
        Vector normal;
        CollisionHit hit;
        get_hit_normal(object->m_dest, object_2->m_dest,
                       hit, normal);
        if (!object->collides(*object_2, hit))
          continue;
        if (!object_2->collides(*object, hit))
          continue;

        object->collision(*object_2, hit);
        object_2->collision(*object, hit);
      }
    }
  }

  // part3: COLGROUP_MOVING vs COLGROUP_MOVING
  for (auto i = m_objects.begin(); i != m_objects.end(); ++i)
  {
    auto object = *i;

    if ((object->get_group() != COLGROUP_MOVING
        && object->get_group() != COLGROUP_MOVING_STATIC)
       || !object->is_valid())
      continue;

    for (auto i2 = i+1; i2 != m_objects.end(); ++i2) {
      auto object_2 = *i2;
      if ((object_2->get_group() != COLGROUP_MOVING
          && object_2->get_group() != COLGROUP_MOVING_STATIC)
         || !object_2->is_valid())
        continue;

      collision_object(object, object_2);
    }
  }

  // apply object movement
  for (const auto& object : m_objects) {
    object->m_bbox = object->m_dest;
    object->m_movement = Vector(0, 0);
  }
}

bool
CollisionSystem::is_free_of_tiles(const Rectf& rect, const bool ignoreUnisolid) const
{
  using namespace collision;

  for (const auto& solids : m_sector.get_solid_tilemaps()) {
    // test with all tiles in this rectangle
    const Rect test_tiles = solids->get_tiles_overlapping(rect);

    for (int x = test_tiles.left; x < test_tiles.right; ++x) {
      for (int y = test_tiles.top; y < test_tiles.bottom; ++y) {
        const Tile& tile = solids->get_tile(x, y);

        if (!(tile.get_attributes() & Tile::SOLID))
          continue;
        if (tile.is_unisolid () && ignoreUnisolid)
          continue;
        if (tile.is_slope ()) {
          AATriangle triangle;
          const Rectf tbbox = solids->get_tile_bbox(x, y);
          triangle = AATriangle(tbbox, tile.get_data());
          Constraints constraints;
          if (!collision::rectangle_aatriangle(&constraints, rect, triangle))
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
CollisionSystem::is_free_of_statics(const Rectf& rect, const CollisionObject* ignore_object, const bool ignoreUnisolid) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect, ignoreUnisolid)) return false;

  for (const auto& object : m_objects) {
    if (object == ignore_object) continue;
    if (!object->is_valid()) continue;
    if (object->get_group() == COLGROUP_STATIC) {
      if (intersects(rect, object->get_bbox())) return false;
    }
  }

  return true;
}

bool
CollisionSystem::is_free_of_movingstatics(const Rectf& rect, const CollisionObject* ignore_object) const
{
  using namespace collision;

  if (!is_free_of_tiles(rect)) return false;

  for (const auto& object : m_objects) {
    if (object == ignore_object) continue;
    if (!object->is_valid()) continue;
    if ((object->get_group() == COLGROUP_MOVING)
        || (object->get_group() == COLGROUP_MOVING_STATIC)
        || (object->get_group() == COLGROUP_STATIC)) {
      if (intersects(rect, object->get_bbox())) return false;
    }
  }

  return true;
}

bool
CollisionSystem::free_line_of_sight(const Vector& line_start, const Vector& line_end, const CollisionObject* ignore_object) const
{
  using namespace collision;

  // check if no tile is in the way
  const float lsx = std::min(line_start.x, line_end.x);
  const float lex = std::max(line_start.x, line_end.x);
  const float lsy = std::min(line_start.y, line_end.y);
  const float ley = std::max(line_start.y, line_end.y);

  for (float test_x = lsx; test_x <= lex; test_x += 16) {
    for (float test_y = lsy; test_y <= ley; test_y += 16) {
      for (const auto& solids : m_sector.get_solid_tilemaps()) {
        const Tile& tile = solids->get_tile_at(Vector(test_x, test_y));
        // FIXME: check collision with slope tiles
        if ((tile.get_attributes() & Tile::SOLID)) return false;
      }
    }
  }

  // check if no object is in the way
  for (const auto& object : m_objects) {
    if (object == ignore_object) continue;
    if (!object->is_valid()) continue;
    if ((object->get_group() == COLGROUP_MOVING)
        || (object->get_group() == COLGROUP_MOVING_STATIC)
        || (object->get_group() == COLGROUP_STATIC)) {
      if (intersects_line(object->get_bbox(), line_start, line_end)) return false;
    }
  }

  return true;
}

std::vector<CollisionObject*>
CollisionSystem::get_nearby_objects (const Vector& center, float max_distance) const
{
  std::vector<CollisionObject*> ret;

  for (const auto& object : m_objects) {
    float distance = object->get_bbox().distance(center);
    if (distance <= max_distance)
      ret.push_back(object);
  }

  return ret;
}

/* EOF */
