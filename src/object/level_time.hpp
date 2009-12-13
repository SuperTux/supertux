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

#ifndef HEADER_SUPERTUX_OBJECT_LEVEL_TIME_HPP
#define HEADER_SUPERTUX_OBJECT_LEVEL_TIME_HPP

#include <memory>

#include "supertux/game_object.hpp"
#include "supertux/script_interface.hpp"
#include "util/reader_fwd.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class LevelTime : public GameObject, 
                  public ScriptInterface
{
  static Color text_color;
public:
  LevelTime(const Reader& reader);

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  void update(float elapsed_time);
  void draw(DrawingContext& context);

  /**
   * @name Scriptable Methods
   * @{
   */

  /**
   * Resumes the countdown
   */
  void start();

  /**
   * Pauses the countdown
   */
  void stop();

  /**
   * Returns the number of seconds left on the clock
   */
  float get_time();

  /**
   * Changes the number of seconds left on the clock
   */
  void set_time(float time_left);

  /**
   * @}
   */

private:
  SurfacePtr time_surface;
  bool running;
  float time_left;
};

#endif

/* EOF */
