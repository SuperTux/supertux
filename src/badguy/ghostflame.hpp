//  SuperTux badguy - Ghostflame a flame-like enemy that cannot be killed
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

#ifndef HEADER_SUPERTUX_BADGUY_GHOSTFLAME_HPP
#define HEADER_SUPERTUX_BADGUY_GHOSTFLAME_HPP

#include "badguy/badguy.hpp"

class Ghostflame : public BadGuy
{
public:
  Ghostflame(const Reader& reader);
  Ghostflame(const Ghostflame& ghostflame);

  void active_update(float elapsed_time);
  void draw(DrawingContext& context);
  void kill_fall();
  virtual std::string get_class() const {
    return "ghostflame";
  }

  virtual ObjectSettings get_settings();
  bool is_flammable() const;

private:
  float angle;
  float radius;
  float speed;
  Color light;
  SpritePtr lightsprite;

};

#endif

/* EOF */
