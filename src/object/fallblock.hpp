//  Copyright (C) 2020 Daniel Ward <weluvgoatz@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_FALLBLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_FALLBLOCK_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"

class Player;

class FallBlock : public MovingSprite

{
public:
  FallBlock(const ReaderMapping& reader);
  
  virtual void update(float dt_sec) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void collision_solid(const CollisionHit& hit) override;

  virtual void draw(DrawingContext& context) override;
  
  static std::string class_name() { return "fallblock"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Falling Platform"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual void on_flip(float height) override;
  
protected:
  enum State
  {
    IDLE,
    SHAKE,
    FALL,
    LAND
  };
  
private:
  State state;
    
  Physic physic;
  Timer timer;
  
  bool found_victim_down() const;

private:
  FallBlock(const FallBlock&) = delete;
  FallBlock& operator=(const FallBlock&) = delete;
};

#endif

/* EOF */
