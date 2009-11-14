//  SuperTux
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __CANDLE_H__
#define __CANDLE_H__

#include <string>

#include "lisp/lisp.hpp"
#include "object/moving_sprite.hpp"
#include "script_interface.hpp"
#include "video/surface.hpp"

/**
 * A burning candle: Simple, scriptable level decoration.
 */
class Candle : public MovingSprite, public ScriptInterface
{
public:
  Candle(const lisp::Lisp& lisp);
  virtual Candle* clone() const { return new Candle(*this); }
  virtual void draw(DrawingContext& context);

  HitResponse collision(GameObject& other, const CollisionHit& hit);

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  /**
   * @name Scriptable Methods
   * @{
   */
  void puff_smoke(); /**< spawn a puff of smoke */
  bool get_burning(); /**< returns true if candle is lighted */
  void set_burning(bool burning); /**< true: light candle, false: extinguish candle */
  /**
   * @}
   */

private:
  bool burning; /**< true if candle is currently lighted */
  Surface candle_light_1; /**< drawn to lightmap */
  Surface candle_light_2; /**< drawn to lightmap (alternative image) */

};

#endif
