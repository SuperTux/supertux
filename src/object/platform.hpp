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
#include "object/path_walker.hpp"
#include "supertux/script_interface.hpp"

/**
 * This class is the base class for platforms that tux can stand on
 */
class Platform : public MovingSprite,
                 public ScriptInterface
{
public:
  Platform(const ReaderMapping& reader);
  Platform(const ReaderMapping& reader, const std::string& default_sprite);
  Platform(const Platform& platform);
  virtual void save(Writer& writer);
  virtual ObjectSettings get_settings();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void update(float elapsed_time);

  const Vector& get_speed() const
  {
    return speed;
  }

  /**
   * @name Scriptable Methods
   * @{
   */

  /** Move platform until at given node, then stop */
  void goto_node(int node_no);

  /** Start moving platform */
  void start_moving();

  /** Stop platform at next node */
  void stop_moving();

  /**
   * @}
   */

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  virtual void move_to(const Vector& pos);

  Path& get_path() const {
    return *path.get();
  }
  std::string get_class() const {
    return "platform";
  }
  std::string get_display_name() const {
    return _("Platform");
  }

private:
  std::unique_ptr<Path> path;
  std::unique_ptr<PathWalker> walker;

  Vector speed;

  bool automatic; /**< true if Platform will automatically pick a destination based on collisions and current Player position */
  bool player_contact; /**< true if a Player touched the Platform during the last round of collision detections */
  bool last_player_contact; /**< true if a Player touched the Platform during the round before the last round of collision detections */

};

#endif

/* EOF */
