//  $Id$
//
//  SuperTux - Boss "GhostTree"
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef __GHOSTTREE_H__
#define __GHOSTTREE_H__

#include <vector>
#include "badguy.hpp"

class TreeWillOWisp;

class GhostTree : public BadGuy
{
public:
  GhostTree(const lisp::Lisp& lisp);
  ~GhostTree();

  void activate();
  void active_update(float elapsed_time);
  void willowisp_died(TreeWillOWisp *willowisp);
  void start_sucking();

private:
  Timer willowisp_timer;
  float willo_spawn_y;
  float willo_radius;
  float willo_speed;
  int   willo_color;

  Timer colorchange_timer;
  Timer root_timer;
  int   treecolor;

  std::vector<TreeWillOWisp*> willowisps;
};

#endif

