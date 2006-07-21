//  $Id$
//
//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef __WILLOWISP_H__
#define __WILLOWISP_H__

#include "badguy.hpp"

class WillOWisp : public BadGuy
{
public:
  WillOWisp(const lisp::Lisp& reader);

  void activate();
  void deactivate();

  void write(lisp::Writer& write);
  void active_update(float elapsed_time);
  void kill_fall();

  virtual void draw(DrawingContext& context);

protected:
  HitResponse collision_player(Player& player, const CollisionHit& hit);

private:
  enum MyState {
    STATE_IDLE, STATE_TRACKING, STATE_VANISHING, STATE_WARPING
  };
  MyState mystate;

  std::string target_sector;
  std::string target_spawnpoint;

  std::auto_ptr<SoundSource> sound_source;
};

#endif
