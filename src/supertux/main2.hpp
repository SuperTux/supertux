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

#ifndef HEADER_SUPERTUX_SUPERTUX_MAIN2_HPP
#define HEADER_SUPERTUX_SUPERTUX_MAIN2_HPP

#include "util/currenton.hpp"

class Main2 final : public Currenton<Main2>
{
public:
  Main2();

  /** We call it run() instead of main() as main collides with
      #define main SDL_main from SDL.h */
  int run(int argc, char** argv);

private:
  Main2(const Main2&) = delete;
  Main2& operator=(const Main2&) = delete;
};

#endif

/* EOF */
