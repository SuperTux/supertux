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

#ifndef HEADER_SUPERTUX_BADGUY_YETI_STALACTITE_HPP
#define HEADER_SUPERTUX_BADGUY_YETI_STALACTITE_HPP

#include "badguy/stalactite.hpp"

class YetiStalactite : public Stalactite
{
public:
  YetiStalactite(const ReaderMapping& lisp);
  virtual ~YetiStalactite();

  void active_update(float elapsed_time);
  void start_shaking();
  bool is_hanging() const;
  void update(float elapsed_time);

  bool is_flammable() const;
  virtual std::string get_class() {
    return "yeti_stalactite";
  }

  virtual ObjectSettings get_settings();

};

#endif

/* EOF */
