//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#ifndef SUPERTUX_TYPE_H
#define SUPERTUX_TYPE_H

#include <string>
#include "SDL.h"
#include "scene.h"

/* 'Base' type for game objects */

struct base_type
{
  float x;
  float y;
  float xm;
  float ym;
  float width;
  float height;
};


/* Base class for all dynamic game object. */
class GameObject
{

public:
  GameObject() {};
  virtual ~GameObject() {};
  virtual void action(double frame_ratio) = 0;
  virtual void draw() = 0;
  virtual std::string type() = 0;
  /* Draw ignoring the scroll_x value. FIXME: Hack? Should be discussed. @tobgle*/
  void draw_on_screen(float x = -1, float y = -1)
  {
    base_type btmp = base;
    if(x != -1 || y != -1)
    {
      btmp = base;
      if(x != -1)
        base.x = x;
      if(y != -1)
        base.y = y;
    }
    float tmp = scroll_x;
    scroll_x = 0; draw();
    scroll_x = tmp;
    base = btmp;
  };
void move_to(float x, float y) { base.x = x; base.y = y; };

  base_type base;
  base_type old_base;
};

struct string_list_type
{
  int num_items;
  int active_item;
  char **item;
};

void  string_list_init(string_list_type* pstring_list);
const char* string_list_active(string_list_type* pstring_list);
void  string_list_copy(string_list_type* pstring_list, string_list_type pstring_list_orig);
int   string_list_find(string_list_type* pstring_list, const char* str);
void  string_list_sort(string_list_type* pstring_list);
void  string_list_add_item(string_list_type* pstring_list, const char* str);
void  string_list_free(string_list_type* pstring_list);

#endif /*SUPERTUX_TYPE_H*/

