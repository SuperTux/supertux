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

#ifndef HEADER_SUPERTUX_SCRIPTING_DISPLAY_EFFECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_DISPLAY_EFFECT_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class DisplayEffect;
#endif

namespace scripting {

/**
 * @summary ""DisplayEffect"" is an interface for toying with the display.
 * @instances SuperTux creates an instance named ""Effect"" when starting the scripting engine.
              Its usage is preferred – creating another instance might have unexpected side effects and is strongly discouraged.
              (Use ""sector.Effect"" in the console.)
 */
class DisplayEffect final
#ifndef SCRIPTING_API
  : public GameObject<::DisplayEffect>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  DisplayEffect(const DisplayEffect&) = delete;
  DisplayEffect& operator=(const DisplayEffect&) = delete;
#endif

public:
  /**
   * Gradually fades out the screen to black for the next ""time"" seconds.
   * @param float $time
   */
  void fade_out(float time);
  /**
   * Gradually fades in the screen from black for the next ""time"" seconds.
   * @param float $time
   */
  void fade_in(float time);
  /**
   * Blackens or un-blackens the screen (depending on the value of ""black"").
   * @param bool $black
   */
  void set_black(bool black);
  /**
   * Returns ""true"" if the screen has been blackened by ""set_black"".
     Note: Calling ""fade_in"" or ""fade_out"" resets the return value to ""false"".
   */
  bool is_black() const;
  /**
   * Sets the display ratio to 16:9, effectively adding black bars at the top and bottom of the screen.
     Should be used before cutscenes. Gradually fades to this state for the next ""time"" seconds.
   * @param float $time
   */
  void sixteen_to_nine(float time);
  /**
   * Sets the display ratio to 4:3, removing the black bars added by ""sixteen_to_nine()"".
     Should be used after cutscenes. Gradually fades to this state for the next ""time"" seconds.
   * @param float $time
   */
  void four_to_three(float time);
};

} // namespace scripting

#endif

/* EOF */
