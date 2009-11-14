//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
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

#ifndef __TREEWILLOWISP_H__
#define __TREEWILLOWISP_H__

#include "badguy.hpp"

class GhostTree;
class SoundSource;

class TreeWillOWisp : public BadGuy
{
public:
  TreeWillOWisp(GhostTree* tree, const Vector& pos, float radius, float speed);
  virtual ~TreeWillOWisp();

  void activate();

  /**
   * make TreeWillOWisp vanish
   */
  void vanish();
  void start_sucking(Vector suck_target);
  bool was_sucked;

  void active_update(float elapsed_time);
  void set_color(const Color& color);
  Color get_color() const;

  virtual bool is_flammable() const { return false; }
  virtual bool is_freezable() const { return false; }
  virtual void kill_fall() { vanish(); }

  virtual void draw(DrawingContext& context);

protected:
  virtual bool collides(GameObject& other, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

private:
  enum MyState {
    STATE_DEFAULT, STATE_VANISHING, STATE_SUCKED
  };
  MyState mystate;

  Color color;
  float angle;
  float radius;
  float speed;

  std::auto_ptr<SoundSource> sound_source;
  GhostTree* tree;

  Vector suck_target;
};

#endif

