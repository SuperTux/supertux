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

#include "squirrel/exposed_object.hpp"
#include "scripting/wind.hpp"
#include "supertux/moving_object.hpp"

class ReaderMapping;

/** Defines an area that will gently push Players in one direction */
class Wind final :
  public MovingObject,
  public ExposedObject<Wind, scripting::Wind>
{
public:
  Wind(const ReaderMapping& reader);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  virtual bool has_variable_size() const override { return true; }
  virtual std::string get_class() const override { return "wind"; }
  virtual std::string get_display_name() const override { return _("Wind");}

  virtual ObjectSettings get_settings() override;

  /** @name Scriptable Methods
      @{ */

  /** start blowing */
  void start();

  /** stop blowing */
  void stop();

  /** @} */

private:
  bool blowing; /**< true if wind is currently switched on */
  Vector speed;
  float acceleration;
  Vector new_size;

  float dt_sec; /**< stores last dt_sec gotten at update() */

  bool affects_badguys; /**< whether the wind can affect badguys */
  bool affects_objects; /**< whether the wind can affect objects */
  bool affects_player; /**< whether the wind can affect the player: useful for cinematic wind */
  bool fancy_wind;

private:
  Wind(const Wind&) = delete;
  Wind& operator=(const Wind&) = delete;
};

#endif

/* EOF */
