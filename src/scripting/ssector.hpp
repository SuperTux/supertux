//  SuperTux - Sector Scripting
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#ifndef __SECTOR_H__
#define __SECTOR_H__

namespace Scripting
{

class SSector
{
public:
#ifndef SCRIPTING_API
    virtual ~SSector()
    {}
#endif
  virtual void set_ambient_light(float red, float green, float blue) = 0;
  virtual float get_ambient_red() = 0;
  virtual float get_ambient_green() = 0;
  virtual float get_ambient_blue() = 0;
  virtual void set_gravity(float gravity) = 0;
};

}

#endif
