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
#include <string>

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"

class ReaderMapping;
class Sprite;

namespace worldmap {

class SpriteChange final : public GameObject
{
private:
  static std::list<SpriteChange*> s_all_sprite_changes;

public:
  SpriteChange(const ReaderMapping& mapping);
  ~SpriteChange() override;

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  /**
   * Activates the SpriteChange's stay action, if applicable
   */
  void set_stay_action();

  /**
   * Deactivates the SpriteChange's stay action, if applicable
   * @param propagate : Also change stay actions in the same stay group
   */
  void clear_stay_action(bool propagate = true);

  /*
   * Get the current value of in_stay_action
   */
   bool show_stay_action() const;

  Vector get_pos() const { return m_pos; }

private:
  Vector m_pos;

public:
  /** should tuxs sprite change when the tile has been completely entered,
      or already when the tile was just touched */
  bool m_change_on_touch;

  /** sprite to change tux image to */
  SpritePtr m_sprite;
  std::string m_sprite_name;

private:
  /** stay action can be used for objects like boats or cars, if it is
      not empty then this sprite will be displayed when tux left the
      tile towards another SpriteChange object. */
  std::string m_stay_action;

  /** name of a group in which only one SpriteChange will ever have
      its stay_action displayed.  Leave empty if you don't care. */
  std::string m_stay_group;

private:
  /** should the stayaction be displayed */
  bool m_in_stay_action;

private:
  SpriteChange(const SpriteChange&) = delete;
  SpriteChange& operator=(const SpriteChange&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
