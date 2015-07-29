//  SuperTux - MovingSprite Base Class
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_MOVING_SPRITE_HPP
#define HEADER_SUPERTUX_OBJECT_MOVING_SPRITE_HPP

#include "object/anchor_point.hpp"
#include "supertux/moving_object.hpp"
#include "util/reader_fwd.hpp"
#include "video/drawing_request.hpp"
#include "sprite/sprite_ptr.hpp"

/**
 * Abstract base class for MovingObjects that are represented by a Sprite
 */
class MovingSprite : public MovingObject
{
public:
  MovingSprite(const Vector& pos,
               const std::string& sprite_name,
               int layer = LAYER_OBJECTS,
               CollisionGroup collision_group = COLGROUP_MOVING);
  MovingSprite(const ReaderMapping& reader,
               const Vector& pos,
               int layer = LAYER_OBJECTS,
               CollisionGroup collision_group = COLGROUP_MOVING);
  MovingSprite(const ReaderMapping& reader,
               const std::string& sprite_name,
               int layer = LAYER_OBJECTS,
               CollisionGroup collision_group = COLGROUP_MOVING);
  MovingSprite(const ReaderMapping& reader,
               int layer = LAYER_OBJECTS,
               CollisionGroup collision_group = COLGROUP_MOVING);
  MovingSprite(const MovingSprite& moving_sprite);
  //MovingSprite& operator=(const MovingSprite& moving_sprite);
  ~MovingSprite();

  virtual void draw(DrawingContext& context);
  virtual void update(float elapsed_time);
  virtual void save(lisp::Writer& writer);
  virtual std::string get_class() {
    return "moving-sprite";
  }

  std::string get_sprite_name() const;

protected:
  std::string sprite_name;
  SpritePtr sprite;
  int layer; /**< Sprite's z-position. Refer to video/drawing_context.hpp for sensible values. */

  /** set new action for sprite and resize bounding box.  use with
      care as you can easily get stuck when resizing the bounding box. */
  void set_action(const std::string& action, int loops);

  /** set new action for sprite and re-center bounding box.  use with
      care as you can easily get stuck when resizing the bounding
      box. */
  void set_action_centered(const std::string& action, int loops);

  /** set new action for sprite and align bounding boxes at
      anchorPoint.  use with care as you can easily get stuck when
      resizing the bounding box. */
  void set_action(const std::string& action, int loops, AnchorPoint anchorPoint);

private:
  //MovingSprite(const MovingSprite&);
  MovingSprite& operator=(const MovingSprite&);
};

#endif

/* EOF */
