//  SuperTux - Boss "GhostTree"
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_GHOSTTREE_HPP
#define HEADER_SUPERTUX_BADGUY_GHOSTTREE_HPP

#include "badguy/badguy.hpp"

class TreeWillOWisp;
class Lantern;

class GhostTree : public BadGuy
{
public:
  GhostTree(const Reader& lisp);
  ~GhostTree();

  virtual bool is_flammable() const { return false; }
  virtual bool is_freezable() const { return false; }
  virtual void kill_fall() { }

  void activate();
  void active_update(float elapsed_time);
  void willowisp_died(TreeWillOWisp* willowisp);
  virtual void draw(DrawingContext& context);

  virtual bool collides(GameObject& other, const CollisionHit& hit);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

  void die();
  virtual std::string get_class() {
    return "ghosttree";
  }

  virtual ObjectSettings get_settings();

private:
  enum MyState {
    STATE_IDLE, STATE_SUCKING, STATE_SWALLOWING, STATE_DYING
  };
  MyState mystate;
  Timer willowisp_timer;
  float willo_spawn_y;
  float willo_radius;
  float willo_speed;
  int   willo_color;

  SpritePtr glow_sprite;
  Timer colorchange_timer;
  Timer suck_timer;
  Timer root_timer;
  int   treecolor;
  Color suck_lantern_color;

  Lantern* suck_lantern; /**< Lantern that is currently being sucked in */

  std::vector<std::shared_ptr<TreeWillOWisp> > willowisps;

  bool is_color_deadly(Color color) const;
  void spawn_lantern();

private:
  GhostTree(const GhostTree&);
  GhostTree& operator=(const GhostTree&);
};

#endif

/* EOF */
