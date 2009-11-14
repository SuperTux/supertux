//  SuperTux - Boss "Yeti"
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __YETI_H__
#define __YETI_H__

#include "badguy.hpp"

#include <memory>

class Yeti : public BadGuy
{
public:
  Yeti(const lisp::Lisp& lisp);
  ~Yeti();

  void draw(DrawingContext& context);
  void write(lisp::Writer& writer);
  void initialize();
  void active_update(float elapsed_time);
  void collision_solid(const CollisionHit& hit);
  bool collision_squished(GameObject& object);
  void kill_squished(GameObject& object);
  void kill_fall();

  virtual Yeti* clone() const { return new Yeti((Yeti&)*this); }

private:
  void run();
  void jump_up();
  void be_angry();
  void drop_stalactite();
  void summon_snowball();
  void jump_down();

  void draw_hit_points(DrawingContext& context);

  void take_hit(Player& player);

  enum YetiState {
    JUMP_DOWN,
    RUN,
    JUMP_UP,
    BE_ANGRY,
    SQUISHED
  };
  YetiState state;
  Timer state_timer;
  Timer safe_timer;
  int stomp_count;
  int hit_points;
  std::auto_ptr<Surface> hud_head;
};

#endif
