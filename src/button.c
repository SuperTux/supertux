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
#include "button.h"

void button_load(button_type* pbutton,char* icon_file, char* text, char* info, int x, int y)
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

  if(text == NULL)
    {
      pbutton->text = NULL;
    }
  else
    {
      pbutton->text = (char*) malloc(sizeof(char)*(strlen(text) + 1));
      strcpy(pbutton->text,text);
    }
  if(info == NULL)
    {
      pbutton->info = NULL;
    }
  else
    {
      pbutton->info = (char*) malloc(sizeof(char)*(strlen(info) + 1));
      strcpy(pbutton->info,info);
    }
  pbutton->x = x;
  pbutton->y = y;
  pbutton->w = pbutton->icon.w;
  pbutton->h = pbutton->icon.h;
  pbutton->state = -1;
}

void button_draw(button_type* pbutton)
{
  fillrect(pbutton->x,pbutton->y,pbutton->w,pbutton->h,75,75,75,200);
  fillrect(pbutton->x+1,pbutton->y+1,pbutton->w-2,pbutton->h-2,175,175,175,200);
  texture_draw(&pbutton->icon,pbutton->x,pbutton->y,NO_UPDATE);
}

void button_free(button_type* pbutton)
{
free(pbutton->text);
free(pbutton->info);
texture_free(&pbutton->icon);
}

int button_pressed(button_type* pbutton, int x, int y)
{
if(x >= pbutton->x && x <= pbutton->x + pbutton->w && y >= pbutton->y && y <= pbutton->y + pbutton->h)
return YES;
else
return NO;
}
