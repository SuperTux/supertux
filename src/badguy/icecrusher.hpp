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
#include "supertux/physic.hpp"

class Player;

/** This class is the base class for icecrushers that tux can stand on */
class IceCrusher final : public MovingSprite
{
public:
  enum IceCrusherState {
    IDLE,
    CRUSHING,
	  CRUSHING_RIGHT,
	  CRUSHING_LEFT,
    RECOVERING,
	  RECOVERING_RIGHT,
	  RECOVERING_LEFT
  };

private:
  enum IceCrusherSize {
    NORMAL,
    LARGE
  };

public:
  IceCrusher(const ReaderMapping& reader);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual bool is_sideways() const;
  virtual std::string get_class() const override { return "icecrusher"; }
  virtual std::string get_display_name() const override { return _("Icecrusher"); }

  virtual void after_editor_set() override;
  
  virtual ObjectSettings get_settings() override;

  bool is_big() const { return ic_size == LARGE; }
  IceCrusherState get_state() const { return state; }

private:
  bool found_victim_down() const;
  bool found_victim_right() const;
  bool found_victim_left() const;
  void set_state(IceCrusherState state, bool force = false);
  Vector eye_position(bool right) const;

  void after_sprite_set();

private:
  IceCrusherState state;
  Vector start_position;
  Physic physic;
  float cooldown_timer;

  SpritePtr lefteye;
  SpritePtr righteye;
  SpritePtr whites;

  IceCrusherSize ic_size;
  
  bool sideways;

private:
  IceCrusher(const IceCrusher&) = delete;
  IceCrusher& operator=(const IceCrusher&) = delete;
};

#endif

/* EOF */
