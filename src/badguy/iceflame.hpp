//  SuperTux badguy - Iceflame a flame-like enemy that can be killed with fireballs
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#ifndef HEADER_SUPERTUX_BADGUY_ICEFLAME_HPP
#define HEADER_SUPERTUX_BADGUY_ICEFLAME_HPP

#include "badguy/flame.hpp"

class Iceflame : public Flame
{
public:
  Iceflame(const ReaderMapping& reader);
  Iceflame(const Iceflame& iceflame);

  void active_update(float elapsed_time);

  void ignite();
  bool is_flammable() const;
  std::string get_class() const {
    return "iceflame";
  }

  std::string get_display_name() const {
    return _("Ice flame");
  }
};

#endif

/* EOF */
