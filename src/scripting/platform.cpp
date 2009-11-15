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

#include "object/platform.hpp"
#include "scripting/platform.hpp"

#define NOIMPL      log_fatal << __PRETTY_FUNCTION__ << " not implemented."

namespace Scripting {

  Platform::Platform(::Platform* platform)
    : platform(platform)
  { }

  Platform::~Platform()
  { }

  void Platform::goto_node(int node_no)
  {
    platform->goto_node(node_no);
  }

  void Platform::start_moving()
  {
    platform->start_moving();
  }

  void Platform::stop_moving()
  {
    platform->stop_moving();
  }

}

/* EOF */
