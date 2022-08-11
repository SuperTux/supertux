//  IceCrusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_ICECRUSHER_HPP
#define HEADER_SUPERTUX_OBJECT_ICECRUSHER_HPP

#include "object/moving_sprite.hpp"
#include "supertux/direction.hpp"
#include "supertux/physic.hpp"

class Player;

/** This class is the base class for icecrushers that tux can stand on */
class IceCrusher final : public MovingSprite
{
public:
  enum IceCrusherState
  {
    IDLE,
    CRUSHING,
    RECOVERING
  };

  enum class Direction
  {
    DOWN,
    LEFT,
    RIGHT
  };

private:
  enum IceCrusherSize
  {
    NORMAL,
    LARGE
  };

public:
  IceCrusher(const ReaderMapping& reader);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void after_editor_set() override;
  virtual bool is_sideways() const { return m_sideways; }
  virtual std::string get_class() const override { return "icecrusher"; }
  virtual std::string get_display_name() const override { return _("Icecrusher"); }
  virtual ObjectSettings get_settings() override;

  virtual void on_flip(float height) override;

  Physic& get_physic() { return m_physic; }
  bool is_big() const { return m_ic_size == LARGE; }
  IceCrusherState get_state() const { return m_state; }

private:
  void spawn_roots(Direction direction);

  bool found_victim() const;
  bool not_ice() const;
  void set_state(IceCrusherState state, bool force = false);
  void after_sprite_set();
  Vector eye_position(bool right) const;

private:
  IceCrusherState m_state;
  IceCrusherSize m_ic_size;
  Vector m_start_position;
  Physic m_physic;
  float m_cooldown_timer;
  bool m_sideways;
  Direction m_side_dir;

  SpritePtr m_lefteye;
  SpritePtr m_righteye;
  SpritePtr m_whites;

private:
  IceCrusher(const IceCrusher&) = delete;
  IceCrusher& operator=(const IceCrusher&) = delete;
};

class CrusherRoot : public MovingSprite
{
public:
  CrusherRoot(Vector position, IceCrusher::Direction direction, float delay, int layer);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;

private:
  void start_animation();
  bool delay_gone() { return m_delay_remaining <= 0.f; }

private:
  Vector m_original_pos;
  IceCrusher::Direction m_direction;
  float m_delay_remaining;

private:
  CrusherRoot(const CrusherRoot&) = delete;
  CrusherRoot& operator=(const CrusherRoot&) = delete;
};

#endif

/* EOF */
