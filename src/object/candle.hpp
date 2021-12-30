//  SuperTux
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

#ifndef HEADER_SUPERTUX_OBJECT_CANDLE_HPP
#define HEADER_SUPERTUX_OBJECT_CANDLE_HPP

#include "object/moving_sprite.hpp"
#include "scripting/candle.hpp"
#include "squirrel/exposed_object.hpp"
#include "video/flip.hpp"

/**
 * A burning candle: Simple, scriptable level decoration.
 */
class Candle final : public MovingSprite,
               public ExposedObject<Candle, scripting::Candle>
{
public:
  Candle(const ReaderMapping& mapping);
  virtual void draw(DrawingContext& context) override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual std::string get_class() const override { return "candle"; }
  virtual std::string get_display_name() const override { return _("Candle"); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void on_flip(float height) override;

  /** @name Scriptable Methods
      @{ */
  void puff_smoke(); /**< spawn a puff of smoke */
  bool get_burning() const; /**< returns true if candle is lighted */
  void set_burning(bool burning); /**< true: light candle, false: extinguish candle */
  /** @} */

private:
  bool burning; /**< true if candle is currently lighted */
  bool flicker; /**< true if candle light is to flicker */
  Color lightcolor; /**< determines color or light given off */
  SpritePtr candle_light_1; /**< drawn to lightmap */
  SpritePtr candle_light_2; /**< drawn to lightmap (alternative image) */

  Flip m_flip;

private:
  Candle(const Candle&) = delete;
  Candle& operator=(const Candle&) = delete;
};

#endif

/* EOF */
