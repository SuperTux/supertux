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

#ifndef SCRIPTING_API
#include <string>

#include "scripting/badguy.hpp"

class WillOWisp;
#endif

namespace scripting {

class WillOWisp final : public scripting::BadGuy
#ifndef SCRIPTING_API
  , virtual public GameObject<::WillOWisp>
#endif
{
#ifndef SCRIPTING_API
public:
  WillOWisp(UID uid) :
    BadGuy(uid),
    GameObject<::BadGuy>(uid),
    GameObject<::WillOWisp>(uid)
  {}

private:
  WillOWisp(const WillOWisp&) = delete;
  WillOWisp& operator=(const WillOWisp&) = delete;
#endif

public:
  /** Move willowisp to given node */
  void goto_node(int node_no);

  /** set willowisp state; can be:
   * -stopped          willowisp doesn't move
   * -move_path        willowisp moves along the path (call goto_node)
   * -move_path_track  willowisp moves along path but catches tux when he is near
   * -normal           "normal" mode starts tracking tux when he is near enough
   * -vanish           vanish
   */
  void set_state(const std::string& state);

  /**
   * Start following the path
   */
  void start_moving();
  /**
   * Stop following the path
   */
  void stop_moving();
};

} // namespace scripting

#endif

/* EOF */
