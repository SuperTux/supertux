//  $Id: player.hpp 3350 2006-04-16 14:43:57Z sommer $
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __SCRIPTING_RANDOM_GENERATOR_H__
#define __SCRIPTING_RANDOM_GENERATOR_H__

namespace Scripting
{

class RandomGenerator
{
public:
#ifndef SCRIPTING_API
  virtual ~RandomGenerator()
  {}
#endif

  /**
   * Seed random number generator
   */
  virtual int srand(int x) = 0;
  /**
   * Return random number in range [0, RAND_MAX)
   */
  virtual int rand() = 0;
  /**
   * Return random number in range [0, v)
   */
  virtual int rand1i(int v) = 0;
  /**
   * Return random number in range [u, v)
   */
  virtual int rand2i(int u, int v) = 0;
  /**
   * Return random number in range [0, v)
   */
  virtual float rand1f(float v) = 0;
  /**
   * Return random number in range [u, v)
   */
  virtual float rand2f(float u, float v) = 0;
};

}

#endif

