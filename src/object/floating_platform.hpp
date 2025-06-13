//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef FLOATING_PLATFORM_HPP
#define FLOATING_PLATFORM_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "badguy/badguy.hpp"  // Needed for BadGuy definition

class FloatingPlatform final : public MovingSprite
{
public:
  FloatingPlatform(const ReaderMapping& reader);
  
  virtual void update(float dt_sec) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(MovingObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "floating_platform"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Floating Platform"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  virtual void on_flip(float height) override;

private:
  Physic m_physic;
  bool m_on_ground;
  bool m_at_ceiling;
  bool m_floating;
  float m_last_sector_gravity;
  float m_sink_offset; 
  float m_max_sink_depth;
  float m_player_offset;
  int number_rocks;
  bool m_player_on_platform;

  FloatingPlatform(const FloatingPlatform&) = delete;
  FloatingPlatform& operator=(const FloatingPlatform&) = delete;
};

#endif

/* EOF */
