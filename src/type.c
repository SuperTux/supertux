//
// C Implementation: type
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

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

char* string_list_active(string_list_type* pstring_list)
{
  if(pstring_list == NULL)
    return "";

  if(pstring_list->active_item != -1)
    return pstring_list->item[pstring_list->active_item];
  else
    return "";
}

void string_list_add_item(string_list_type* pstring_list, char* str)
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

int string_list_find(string_list_type* pstring_list, char* str)
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
  short int sorter[pstring_list->num_items];

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
