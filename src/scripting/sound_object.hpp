//  SuperTux
//  Copyright (C) 2023 mrkubax10 <mrkubax10@onet.pl>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_SOUND_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_SOUND_OBJECT_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class SoundObject;
#endif

namespace scripting {

/**
 * @summary A ""SoundObject"" that was given a name can be controlled by scripts.
 * @instances A ""SoundObject"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class SoundObject final
#ifndef SCRIPTING_API
  : public GameObject<::SoundObject>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  SoundObject(const SoundObject&) = delete;
  SoundObject& operator=(const SoundObject&) = delete;
#endif

public:
  /**
   * Starts playing sound if it was stopped previously.
   */
  void start_playing();

  /**
   * Stops playing sound.
   */
  void stop_playing();

  /**
   * Sets the volume of sound played by SoundObject.
   * @param float $volume
   */
  void set_volume(float volume);

  /**
   * Returns the volume of sound played by SoundObject.
   */
  float get_volume() const;
};

} // namespace scripting

#endif

/* EOF */
