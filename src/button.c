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
  pbutton->state = -1;
  pbutton->show_info = NO;
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
  texture_draw(&pbutton->icon,pbutton->x,pbutton->y,NO_UPDATE);
  if(pbutton->show_info == YES)
    {
      char str[80];
      if(pbutton->info)
        text_draw(&white_small_text, pbutton->info, pbutton->x - strlen(pbutton->info) * white_small_text.w, pbutton->y, 1, NO_UPDATE);
      sprintf(str,"(%s)", SDL_GetKeyName(pbutton->shortcut));
      text_draw(&white_small_text, str, pbutton->x - strlen(str) * white_small_text.w, pbutton->y + white_small_text.h+2, 1, NO_UPDATE);
    }
  if(pbutton->state == BN_PRESSED)
    fillrect(pbutton->x,pbutton->y,pbutton->w,pbutton->h,75,75,75,200);
}

void button_free(button_type* pbutton)
{
  free(pbutton->info);
  texture_free(&pbutton->icon);
}

void button_event(button_type* pbutton, SDL_Event *event)
{
  if(event->type == SDL_KEYDOWN)
    {
      SDLKey key = event->key.keysym.sym;
      if(key == pbutton->shortcut)
        pbutton->state = BN_CLICKED;
    }
  else if(event->type == SDL_MOUSEMOTION)
    {

      if(pbutton->show_info)
        {
          pbutton->show_info = NO;
        }
    }

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
      if(event->type == SDL_MOUSEBUTTONUP)
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
    }
  else
    {
      pbutton->state = -1;
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
    return pbutton->state;
}

void button_panel_init(button_panel_type* pbutton_panel, int x, int y, int w, int h)
{
  pbutton_panel->num_items = 0;
  pbutton_panel->item = NULL;
  pbutton_panel->x = x;
  pbutton_panel->y = y;
  pbutton_panel->w = w;
  pbutton_panel->h = h;
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
  int i;
  for(i = 0; i < pbutton_panel->num_items; ++i)
    {
      button_draw(&pbutton_panel->item[i]);
    }
}

void button_panel_additem(button_panel_type* pbutton_panel, button_type* pbutton)
{
  int max_cols, row, col;

  ++pbutton_panel->num_items;
  pbutton_panel->item = (button_type*) realloc(pbutton_panel->item, sizeof(button_type) * pbutton_panel->num_items);
  memcpy(&pbutton_panel->item[pbutton_panel->num_items-1],pbutton,sizeof(button_type));
  free(pbutton);

  /* A button_panel takes control of the buttons it contains and arranges them */

  max_cols = pbutton_panel->w / 32;

  row = pbutton_panel->num_items / max_cols;
  col = pbutton_panel->num_items % max_cols;

  pbutton_panel->item[pbutton_panel->num_items-1].x = pbutton_panel->x + col * 32;
  pbutton_panel->item[pbutton_panel->num_items-1].y = pbutton_panel->y + row * 32;

}

