//  $Id: player.hpp 3962 2006-07-09 10:32:59Z matzebraun $
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

#ifndef __SCRIPTING_THUNDERSTORM_H__
#define __SCRIPTING_THUNDERSTORM_H__

namespace Scripting
{

class Thunderstorm
{
public:
#ifndef SCRIPTING_API
  virtual ~Thunderstorm()
  {}
#endif

    /**
     * Start playing thunder and lightning at configured interval
     */
    virtual void start() = 0;
    
    /**
     * Stop playing thunder and lightning at configured interval
     */
    virtual void stop() = 0;

    /**
     * Play thunder
     */
    virtual void thunder() = 0;

    /**
     * Play lightning, i.e. call flash() and electrify()
     */
    virtual void lightning() = 0;

    /**
     * Display a nice flash
     */
    virtual void flash() = 0;

    /**
     * Electrify water throughout the whole sector for a short time
     */
    virtual void electrify() = 0;


};

}

#endif

