//  SuperTux - Wind
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

#ifndef HEADER_SUPERTUX_OBJECT_WIND_HPP
#define HEADER_SUPERTUX_OBJECT_WIND_HPP

#include "supertux/moving_object.hpp"
#include "supertux/script_interface.hpp"
#include "util/reader_fwd.hpp"

class Player;

/**
 * Defines an area that will gently push Players in one direction
 */
class Wind : public MovingObject,
             public ScriptInterface
{
public:
  Wind(const ReaderMapping& reader);
  virtual void save(Writer& writer);

  void update(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  /**
   * @name Scriptable Methods
   * @{
   */

  /**
   * start blowing
   */
  void start();

  /**
   * stop blowing
   */
  void stop();

  /**
   * @}
   */

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual std::string get_class() const {
    return "wind";
  }

  virtual ObjectSettings get_settings();

private:
  bool blowing; /**< true if wind is currently switched on */
  Vector speed;
  float acceleration;

  float elapsed_time; /**< stores last elapsed_time gotten at update() */
};

#endif

/* EOF */
