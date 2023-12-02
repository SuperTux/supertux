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

#include "scripting/sound_object.hpp"

#include "object/sound_object.hpp"

namespace scripting {

void
SoundObject::start_playing()
{
  SCRIPT_GUARD_VOID;
  object.play_looping_sounds();
}

void
SoundObject::stop_playing()
{
  SCRIPT_GUARD_VOID;
  object.stop_looping_sounds();
}

void
SoundObject::set_volume(float volume)
{
  SCRIPT_GUARD_VOID;
  object.set_volume(volume);
}

float
SoundObject::get_volume() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_volume();
}

void
SoundObject::set_play_interval(float play_interval)
{
  SCRIPT_GUARD_VOID;
  object.set_play_interval(play_interval);
}

float
SoundObject::get_play_interval() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_play_interval();
}

} // namespace scripting

/* EOF */
