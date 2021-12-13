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

#ifndef HEADER_SUPERTUX_OBJECT_PLATFORM_HPP
#define HEADER_SUPERTUX_OBJECT_PLATFORM_HPP

#include "object/moving_sprite.hpp"
#include "object/path_object.hpp"
#include "squirrel/exposed_object.hpp"
#include "scripting/platform.hpp"

/** This class is the base class for platforms that tux can stand
    on */
class Platform : public MovingSprite,
                 public ExposedObject<Platform, scripting::Platform>,
                 public PathObject
{
public:
  Platform(const ReaderMapping& reader);
  Platform(const ReaderMapping& reader, const std::string& default_sprite);

  virtual void finish_construction() override;

  virtual ObjectSettings get_settings() override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void update(float dt_sec) override;

  virtual void move_to(const Vector& pos) override;

  virtual std::string get_class() const override { return "platform"; }
  virtual std::string get_display_name() const override { return _("Platform"); }

  virtual void editor_update() override;

  const Vector& get_speed() const { return m_speed; }

  /** @name Scriptable Methods
      @{ */

  /** Move platform until at given node, then stop */
  void goto_node(int node_no);

  /** Start moving platform */
  void start_moving();

  /** Stop platform at next node */
  void stop_moving();

  /** Updates the platform to the given action  */
  void set_action(const std::string& action, int repeat);

  /** @} */

private:
  Vector m_speed;

  /** true if Platform will automatically pick a destination based on
      collisions and current Player position */
  bool m_automatic;

  /** true if a Player touched the Platform during the last round of
      collision detections */
  bool m_player_contact;

  /** true if a Player touched the Platform during the round before
      the last round of collision detections */
  bool m_last_player_contact;

  int m_starting_node;

private:
  Platform(const Platform&) = delete;
  Platform& operator=(const Platform&) = delete;
};

#endif

/* EOF */
