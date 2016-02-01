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

#ifndef HEADER_SUPERTUX_WORLDMAP_SPRITE_CHANGE_HPP
#define HEADER_SUPERTUX_WORLDMAP_SPRITE_CHANGE_HPP

#include <list>
#include <memory>
#include <string>

#include "util/reader_fwd.hpp"
#include "math/vector.hpp"
#include "supertux/game_object.hpp"

class Sprite;

namespace worldmap {

class SpriteChange : public GameObject
{
public:
  SpriteChange(const ReaderMapping& lisp);
  virtual ~SpriteChange();

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

public:
  Vector pos;

  /** should tuxs sprite change when the tile has been completely entered,
      or already when the tile was just touched */
  bool change_on_touch;

  /** sprite to change tux image to */
  SpritePtr sprite;
  std::string sprite_name;

  /** stay action can be used for objects like boats or cars, if it is
      != "" then this sprite will be displayed when tux left the tile
      towards another SpriteChange object. */
  std::string stay_action;

  /** name of a group in which only one SpriteChange will ever have
      its stay_action displayed.  Leave empty if you don't care. */
  std::string stay_group;

private:
  /** should the stayaction be displayed */
  bool in_stay_action;

private:
  static std::list<SpriteChange*> all_sprite_changes;
};

} // namespace worldmap

#endif

/* EOF */
