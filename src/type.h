//
// C Interface: type
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_TYPE_H
#define SUPERTUX_TYPE_H

#include "SDL.h"

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


struct string_list_type
{
  int num_items;
  int active_item;
  char **item;
};

void  string_list_init(string_list_type* pstring_list);
char* string_list_active(string_list_type* pstring_list);
void  string_list_copy(string_list_type* pstring_list, string_list_type pstring_list_orig);
int   string_list_find(string_list_type* pstring_list, const char* str);
void  string_list_sort(string_list_type* pstring_list);
void  string_list_add_item(string_list_type* pstring_list, const char* str);
void  string_list_free(string_list_type* pstring_list);

#endif /*SUPERTUX_TYPE_H*/

