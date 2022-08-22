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

#include "math/anchor_point.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/moving_object.hpp"
#include "video/drawing_context.hpp"
#include "video/flip.hpp"

class ReaderMapping;

/** Abstract base class for MovingObjects that are represented by a Sprite */
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

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  static std::string class_name() { return "moving-sprite"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_default_sprite_name() const { return m_default_sprite_name; }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual int get_layer() const override { return m_layer; }

  std::string get_sprite_name() const;
  void change_sprite(const std::string& new_sprite_name);
  void spawn_explosion_sprites(int count, const std::string& sprite_path);

protected:
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

protected:
  std::string m_sprite_name;

  /** The default sprite for this MovingObject */
  std::string m_default_sprite_name;
  SpritePtr m_sprite;
  int m_layer; /**< Sprite's z-position. Refer to video/drawing_context.hpp for sensible values. */

  Flip m_flip;

private:
  MovingSprite(const MovingSprite&) = delete;
  MovingSprite& operator=(const MovingSprite&) = delete;
};

#endif

/* EOF */
