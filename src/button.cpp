//
// C Implementation: button
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <string.h>
#include <stdlib.h>
#include "setup.h"
#include "screen.h"
#include "globals.h"
#include "button.h"

void button_load(button_type* pbutton,char* icon_file, char* info, SDLKey shortcut, int x, int y)
{
  char filename[1024];

  if(icon_file != NULL)
    {
      snprintf(filename, 1024, "%s/%s", DATA_PREFIX, icon_file);
      if(!faccessible(filename))
        snprintf(filename, 1024, "%s/images/icons/default-icon.png", DATA_PREFIX);
    }
  else
    {
      snprintf(filename, 1024, "%s/images/icons/default-icon.png", DATA_PREFIX);
    }
  texture_load(&pbutton->icon,filename,USE_ALPHA);

  if(info == NULL)
    {
      pbutton->info = NULL;
    }
  else
    {
      pbutton->info = (char*) malloc(sizeof(char)*(strlen(info) + 1));
      strcpy(pbutton->info,info);
    }

  pbutton->shortcut = shortcut;

  pbutton->x = x;
  pbutton->y = y;
  pbutton->w = pbutton->icon.w;
  pbutton->h = pbutton->icon.h;
  pbutton->tag = -1;
  pbutton->state = -1;
  pbutton->show_info = NO;
  pbutton->bkgd = NULL;
}

button_type* button_change_icon(button_type* pbutton,char* icon_file)
{
  char filename[1024];

  if(icon_file != NULL)
    {
      snprintf(filename, 1024, "%s/%s", DATA_PREFIX, icon_file);
      if(!faccessible(filename))
        snprintf(filename, 1024, "%s/images/icons/default-icon.png", DATA_PREFIX);
    }
  else
    {
      snprintf(filename, 1024, "%s/images/icons/default-icon.png", DATA_PREFIX);
    }
  
  texture_free(&pbutton->icon);
  texture_load(&pbutton->icon,filename,USE_ALPHA);
}

button_type* button_create(char* icon_file, char* info, SDLKey shortcut, int x, int y)
{
  button_type* pnew_button = (button_type*) malloc(sizeof(button_type));
  button_load(pnew_button,icon_file, info, shortcut, x, y);
  return pnew_button;
}

void button_draw(button_type* pbutton)
{
  fillrect(pbutton->x,pbutton->y,pbutton->w,pbutton->h,75,75,75,200);
  fillrect(pbutton->x+1,pbutton->y+1,pbutton->w-2,pbutton->h-2,175,175,175,200);
  if(pbutton->bkgd != NULL)
  {
  texture_draw(pbutton->bkgd,pbutton->x,pbutton->y,NO_UPDATE);
  }
  texture_draw(&pbutton->icon,pbutton->x,pbutton->y,NO_UPDATE);
  if(pbutton->show_info == YES)
    {
      char str[80];
      int i = -32;

      if(0 > pbutton->x - (int)strlen(pbutton->info) * white_small_text.w)
        i = pbutton->w + strlen(pbutton->info) * white_small_text.w;

      if(pbutton->info)
        text_draw(&white_small_text, pbutton->info, i + pbutton->x - strlen(pbutton->info) * white_small_text.w, pbutton->y, 1, NO_UPDATE);
      sprintf(str,"(%s)", SDL_GetKeyName(pbutton->shortcut));
      text_draw(&white_small_text, str, i + pbutton->x - strlen(str) * white_small_text.w, pbutton->y + white_small_text.h+2, 1, NO_UPDATE);
    }
  if(pbutton->state == BN_PRESSED)
    fillrect(pbutton->x,pbutton->y,pbutton->w,pbutton->h,75,75,75,200);
  else if(pbutton->state == BN_HOVER)
    fillrect(pbutton->x,pbutton->y,pbutton->w,pbutton->h,150,150,150,128);
}

void button_free(button_type* pbutton)
{
  free(pbutton->info);
  texture_free(&pbutton->icon);
}

void button_event(button_type* pbutton, SDL_Event *event)
{
  SDLKey key = event->key.keysym.sym;

  if(event->motion.x > pbutton->x && event->motion.x < pbutton->x + pbutton->w &&
      event->motion.y > pbutton->y && event->motion.y < pbutton->y + pbutton->h)
    {
      if(event->type == SDL_MOUSEBUTTONDOWN)
        {
          if(event->button.button == SDL_BUTTON_LEFT)
            {
              pbutton->state = BN_PRESSED;
            }
          else
            {
              pbutton->show_info = YES;
            }
        }
      else if(event->type == SDL_MOUSEBUTTONUP)
        {
          if(event->button.button == SDL_BUTTON_LEFT && pbutton->state == BN_PRESSED)
            {
              pbutton->state = BN_CLICKED;
            }
          else if(event->button.button != SDL_BUTTON_LEFT && pbutton->state != BN_PRESSED)
            {
              pbutton->show_info = YES;
            }
        }

      if(pbutton->state != BN_PRESSED && pbutton->state != BN_CLICKED)
        {
          pbutton->state = BN_HOVER;
        }
    }
  else if(event->type != SDL_KEYDOWN && event->type != SDL_KEYUP)
    {
      pbutton->state = -1;
      if(pbutton->show_info)
        {
          pbutton->show_info = NO;
        }
    }

  if(event->type == SDL_KEYDOWN)
    {
      if(key == pbutton->shortcut)
        pbutton->state = BN_PRESSED;
    }
  else if(event->type == SDL_KEYUP)
    {
      if(pbutton->state == BN_PRESSED && key == pbutton->shortcut)
        pbutton->state = BN_CLICKED;
    }
  else if(event->type == SDL_MOUSEMOTION)
    {

      if(pbutton->show_info)
        {
          pbutton->show_info = NO;
        }
    }
}

int button_get_state(button_type* pbutton)
{
  int state;
  if(pbutton->state == BN_CLICKED)
    {
      state = pbutton->state;
      pbutton->state = -1;
      return state;
    }
  else
    {
      return pbutton->state;
    }
}

void button_panel_init(button_panel_type* pbutton_panel, int x, int y, int w, int h)
{
  pbutton_panel->num_items = 0;
  pbutton_panel->item = NULL;
  pbutton_panel->x = x;
  pbutton_panel->y = y;
  pbutton_panel->w = w;
  pbutton_panel->h = h;
  pbutton_panel->hidden = NO;
}

button_type* button_panel_event(button_panel_type* pbutton_panel, SDL_Event* event)
{
  if(pbutton_panel->hidden == NO)
    {
      int i;
      for(i = 0; i < pbutton_panel->num_items; ++i)
        {
          button_event(&pbutton_panel->item[i],event);
          if(pbutton_panel->item[i].state != -1)
            return &pbutton_panel->item[i];
        }
      return NULL;
    }
  else
    {
      return NULL;
    }
}

void button_panel_free(button_panel_type* pbutton_panel)
{
  int i;
  for(i = 0; i < pbutton_panel->num_items; ++i)
    {
      button_free(&pbutton_panel->item[i]);
    }
  if(pbutton_panel->num_items)
    free(pbutton_panel->item);
}

void button_panel_draw(button_panel_type* pbutton_panel)
{
  if(pbutton_panel->hidden == NO)
    {
      int i;
      fillrect(pbutton_panel->x,pbutton_panel->y,pbutton_panel->w,pbutton_panel->h,100,100,100,200);
      for(i = 0; i < pbutton_panel->num_items; ++i)
        {
          button_draw(&pbutton_panel->item[i]);
        }
    }
}

void button_panel_additem(button_panel_type* pbutton_panel, button_type* pbutton, int tag)
{
  int max_cols, row, col;

  ++pbutton_panel->num_items;
  pbutton_panel->item = (button_type*) realloc(pbutton_panel->item, sizeof(button_type) * pbutton_panel->num_items);
  memcpy(&pbutton_panel->item[pbutton_panel->num_items-1],pbutton,sizeof(button_type));
  free(pbutton);

  /* A button_panel takes control of the buttons it contains and arranges them */

  max_cols = pbutton_panel->w / 32;

  row = (pbutton_panel->num_items-1) / max_cols;
  col = (pbutton_panel->num_items-1) % max_cols;

  pbutton_panel->item[pbutton_panel->num_items-1].x = pbutton_panel->x + col * 32;
  pbutton_panel->item[pbutton_panel->num_items-1].y = pbutton_panel->y + row * 32;
  pbutton_panel->item[pbutton_panel->num_items-1].tag = tag;

}

