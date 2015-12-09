//  SuperTux
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_WILLOWISP_HPP
#define HEADER_SUPERTUX_SCRIPTING_WILLOWISP_HPP

namespace scripting {

class WillOWisp
{
public:
#ifndef SCRIPTING_API
  virtual ~WillOWisp()
  {}
#endif

  /** Move willowisp to given node */
  virtual void goto_node(int node_no) = 0;

  /** set willowisp state; can be:
   * -stopped          willowisp doesn't move
   * -move_path        willowisp moves along the path (call goto_node)
   * -move_path_track  willowisp moves along path but catches tux when he is near
   * -normal           "normal" mode starts tracking tux when he is near enough
   * -vanish           vanish
   */
  virtual void set_state(const std::string& state) = 0;

  virtual void start_moving() = 0;
  virtual void stop_moving() = 0;
};

}

#endif

/* EOF */
