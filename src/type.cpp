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

#include "SDL_image.h"
#include "string.h"
#include "stdlib.h"
#include "setup.h"
#include "globals.h"
#include "screen.h"
#include "defines.h"
#include "type.h"
#include "scene.h"

void string_list_init(string_list_type* pstring_list)
{
  pstring_list->num_items = 0;
  pstring_list->active_item = -1;
  pstring_list->item = NULL;
}

const char* string_list_active(string_list_type* pstring_list)
{
  if(pstring_list == NULL)
    return "";

  if(pstring_list->active_item != -1)
    return pstring_list->item[pstring_list->active_item];
  else
    return "";
}

void string_list_add_item(string_list_type* pstring_list,const  char* str)
{
  char *pnew_string;
  pnew_string = (char*) malloc(sizeof(char)*(strlen(str)+1));
  strcpy(pnew_string,str);
  ++pstring_list->num_items;
  pstring_list->item = (char**) realloc(pstring_list->item,sizeof(char**)*pstring_list->num_items);
  pstring_list->item[pstring_list->num_items-1] = pnew_string;
  if(pstring_list->active_item == -1)
    pstring_list->active_item = 0;
}

void string_list_copy(string_list_type* pstring_list, string_list_type pstring_list_orig)
{
  int i;
  string_list_free(pstring_list);
  for(i = 0; i < pstring_list_orig.num_items; ++i)
    string_list_add_item(pstring_list,pstring_list_orig.item[i]);
}

int string_list_find(string_list_type* pstring_list,const  char* str)
{
  int i;
  for(i = 0; i < pstring_list->num_items; ++i)
    {
      if(strcmp(pstring_list->item[i],str) == 0)
        {
          return i;
        }
    }
  return -1;
}

void string_list_sort(string_list_type* pstring_list)
{
  int i,j,y;

  for(j = 0; j < pstring_list->num_items; ++j)
    for(i = 0; i < pstring_list->num_items-1; ++i)
      {

        y = strcmp(pstring_list->item[i],pstring_list->item[i+1]);
        if(y == 0)
          {
            continue;
          }
        else if(y < 0)
          {
            continue;
          }
        else if(y > 0)
          {
            char* char_pointer;
            char_pointer = pstring_list->item[i];
            pstring_list->item[i] = pstring_list->item[i+1];
            pstring_list->item[i+1] = char_pointer;
            continue;
          }

      }

}

void string_list_free(string_list_type* pstring_list)
{
  if(pstring_list != NULL)
    {
      int i;
      for(i=0; i < pstring_list->num_items; ++i)
        free(pstring_list->item[i]);
      free(pstring_list->item);
      pstring_list->item = NULL;
      pstring_list->num_items = 0;
      pstring_list->active_item = -1;
    }
}
