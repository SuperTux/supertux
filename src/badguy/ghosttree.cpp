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
#include <config.h>

#include "ghosttree.hpp"
#include "treewillowisp.hpp"
#include "root.hpp"

static const size_t WILLOWISP_COUNT = 10;

GhostTree::GhostTree(const lisp::Lisp& lisp)
  : BadGuy(lisp, "images/creatures/ghosttree/ghosttree.sprite",
           LAYER_OBJECTS - 10),
    willo_spawn_y(0), willo_radius(200), willo_speed(1.8f), willo_color(0),
    treecolor(0)
{
}

GhostTree::~GhostTree()
{
}

void
GhostTree::activate()
{
  willowisp_timer.start(1.0f, true);
  colorchange_timer.start(13, true);
  root_timer.start(5, true);
  set_group(COLGROUP_DISABLED);
}

void
GhostTree::active_update(float elapsed_time)
{
  (void) elapsed_time;

  if(colorchange_timer.check()) {
    sound_manager->play("sounds/tree_howling.ogg", get_pos());
    suck_timer.start(3);
    treecolor = (treecolor + 1) % 3;

    Color col;
    switch(treecolor) {
    case 0: col = Color(1, 0, 0); break;
    case 1: col = Color(0, 1, 0); break;
    case 2: col = Color(0, 0, 1); break;
    case 3: col = Color(1, 1, 0); break;
    case 4: col = Color(1, 0, 1); break;
    case 5: col = Color(0, 1, 1); break;
    default: assert(false);
    }
    sprite->set_color(col);
  }

  if(suck_timer.check()) {
    Color col = sprite->get_color();
    sound_manager->play("sounds/tree_suck.ogg", get_pos());
    std::vector<TreeWillOWisp*>::iterator iter;
    for(iter = willowisps.begin(); iter != willowisps.end(); ++iter) {
      TreeWillOWisp *willo = *iter;
      if(willo->get_color() == col) {
        willo->start_sucking();
      }
    }
  }

  if(willowisp_timer.check()) {
    if(willowisps.size() < WILLOWISP_COUNT) {
      Vector pos = Vector(bbox.get_width() / 2, bbox.get_height() / 2 + willo_spawn_y);
      TreeWillOWisp *willowisp 
        = new TreeWillOWisp(this, pos, 200 + willo_radius, willo_speed);

      Sector::current()->add_object(willowisp);
      willowisps.push_back(willowisp);

      willo_spawn_y -= 40;
      if(willo_spawn_y < -160)
        willo_spawn_y = 0;

      willo_radius += 20;
      if(willo_radius > 120)
        willo_radius = 0;

      if(willo_speed == 1.8f) {
        willo_speed = 1.5f;
      } else {
        willo_speed = 1.8f;
      }

      do {
        willo_color = (willo_color + 1) % 3;
      } while(willo_color == treecolor);

      switch(willo_color) {
      case 0: willowisp->set_color(Color(1, 0, 0)); break;
      case 1: willowisp->set_color(Color(0, 1, 0)); break;
      case 2: willowisp->set_color(Color(0, 0, 1)); break;
      case 3: willowisp->set_color(Color(1, 1, 0)); break;
      case 4: willowisp->set_color(Color(1, 0, 1)); break;
      case 5: willowisp->set_color(Color(0, 1, 1)); break;
      default: assert(false);
      }
    }
  }

  if(root_timer.check()) {
    /* TODO indicate root with an animation */
    Player* player = get_nearest_player();
    Root* root = new Root(Vector(player->get_bbox().get_left(), get_bbox().get_bottom()));
    Sector::current()->add_object(root);
  }
}

void
GhostTree::willowisp_died(TreeWillOWisp *willowisp)
{
  willowisps.erase(std::find(willowisps.begin(), willowisps.end(), willowisp));
}

IMPLEMENT_FACTORY(GhostTree, "ghosttree");

