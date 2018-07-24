//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_BADGUY_WALKING_CANDLE_HPP
#define HEADER_SUPERTUX_BADGUY_WALKING_CANDLE_HPP

#include "badguy/walking_badguy.hpp"

class WalkingCandle : public WalkingBadguy
{
public:
  WalkingCandle(const ReaderMapping& reader);

  bool is_freezable() const;
  bool is_flammable() const;

  void freeze();
  void unfreeze();

  HitResponse collision(GameObject& other, const CollisionHit& hit);

  void kill_fall() { };

  ObjectSettings get_settings();
  virtual void after_editor_set();
  std::string get_class() const {
    return "walking_candle";
  }
  std::string get_display_name() const {
    return _("Mr. Candle");
  }

private:

  Color lightcolor;
};

#endif

/* EOF */
