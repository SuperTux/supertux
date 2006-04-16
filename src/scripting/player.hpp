//  $Id$
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

namespace Scripting
{

class Player
{
public:
#ifndef SCRIPTING_API
  virtual ~Player()
  {}
#endif

  /**
   * Set tux bonus.
   * This can be "grow", "fireflower" or "iceflower" at the moment
   */
  virtual void add_bonus(const std::string& bonus) = 0;
  /**
   * Give tux more coins
   */
  virtual void add_coins(int count) = 0;
  /**
   * Make tux invicible for a short amount of time
   */
  virtual void make_invincible() = 0;
  /**
   * Deactivate user input for Tux
   */
  virtual void deactivate() = 0;
  /**
   * Give control back to user
   */
  virtual void activate() = 0;
  /**
   * Make Tux walk
   */
  virtual void walk(float speed) = 0;
  /**
   * Set player visible or not visible
   */
  virtual void set_visible(bool visible) = 0;
  /**
   * returns true if the player is currently visible (that is he was not set
   * inivisible by the set_visible method)
   */
  virtual bool get_visible() = 0;
};

}

#endif

