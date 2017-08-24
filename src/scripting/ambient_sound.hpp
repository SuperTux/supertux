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

#ifndef HEADER_SUPERTUX_SCRIPTING_AMBIENT_SOUND_HPP
#define HEADER_SUPERTUX_SCRIPTING_AMBIENT_SOUND_HPP

#ifndef SCRIPTING_API
#include "scripting/sqrat_object.hpp"
class AmbientSound;
#endif

namespace scripting {

#ifdef SCRIPTING_API
class AmbientSound
#else
class AmbientSound: public SQRatObject<AmbientSound>
#endif 
{
#ifndef SCRIPTING_API
private:
  ::AmbientSound* m_parent;

public:
  AmbientSound(::AmbientSound* parent);
   virtual ~AmbientSound();
  void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass);

private:
  AmbientSound(const AmbientSound&) = delete;
  AmbientSound& operator=(const AmbientSound&) = delete;
#endif

public:
  void set_pos(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;
};

}

#endif

/* EOF */
