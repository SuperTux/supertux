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
#ifndef __WORLDMAP_SPRITE_CHANGE_HPP__
#define __WORLDMAP_SPRITE_CHANGE_HPP__

#include <string>
#include <memory>
#include <list>
#include "supertux/game_object.hpp"
#include "lisp/lisp.hpp"
#include "math/vector.hpp"

class Sprite;

namespace WorldMapNS
{

class SpriteChange : public GameObject
{
public:
  SpriteChange(const lisp::Lisp* lisp);
  virtual ~SpriteChange();

  Vector pos;
  /**
   * should tuxs sprite change when the tile has been completely entered,
   * or already when the tile was just touched
   */
  bool change_on_touch;
  /// sprite to change tux image to
  std::auto_ptr<Sprite> sprite;
  /**
   * stay action can be used for objects like boats or cars, if it is
   * != "" then this sprite will be displayed when tux left the tile towards
   * another SpriteChange object.
   */
  std::string stay_action;

  /**
   * name of a group in which only one SpriteChange will ever have its stay_action displayed.
   * Leave empty if you don't care.
   */
  std::string stay_group;

  virtual void draw(DrawingContext& context);
  virtual void update(float elapsed_time);

  /**
   * Activates the SpriteChange's stay action, if applicable
   */
  void set_stay_action();

  /**
   * Deactivates the SpriteChange's stay action, if applicable
   */
  void clear_stay_action();

private:
  /**
   * should the stayaction be displayed
   */
  bool in_stay_action;

  static std::list<SpriteChange*> all_sprite_changes;

};

}

#endif
