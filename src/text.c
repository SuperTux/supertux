//
// C Implementation: text
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
#include "globals.h"
#include "defines.h"
#include "screen.h"
#include "text.h"

void text_load(text_type* ptext, char* file)
{
  int x, y, c;

  c = 0;

  for(y = 0; y < 3 ; ++y)
    {
      for(x = 0; x < 26 ; ++x)
        {
          texture_load_part(&ptext->chars[c],file,x*16,y*16,16,16, USE_ALPHA);
          ++c;
        }
    }
}

void text_draw(text_type* ptext, char* text, int x, int y, int shadowsize, int update)
{
  int len;
  int i;

if(shadowsize != 0)
text_draw(&black_text,text,x+shadowsize,y+shadowsize, 0, update);

  len = strlen(text);

  for( i = 0; i < len; ++i)
    {
      if( text[i] >= 'A' && text[i] <= 'Z')
      {
        texture_draw(&ptext->chars[(int)(text[i] - 'A')],x+i*16,y,update);
	}
      else if( text[i] >= 'a' && text[i] <= 'z')
      {
        texture_draw(&ptext->chars[(int)(text[i] - 'a') + 26],x+i*16,y,update);
	}
      else if ( text[i] >= '!' && text[i] <= '9')
	{
	texture_draw(&ptext->chars[(int)(text[i] - '!') + 52],x+i*16,y,update);
	}
      else if ( text[i] == '?')
	{
	texture_draw(&ptext->chars[77],x+i*16,y,update);
	}
      else if ( text[i] == '\n')
	{
	y += 18;
	}
    }
}

void text_drawf(text_type* ptext, char* text, int x, int y, int halign, int valign, int shadowsize, int update)
{
if(halign == A_RIGHT)
x += screen->w;
else if(halign == A_HMIDDLE)
x += screen->w/2 - ((strlen(text)*16)/2);

if(valign == A_BOTTOM)
y += screen->h - 16;
else if(valign == A_VMIDDLE)
y += screen->h/2;

text_draw(ptext,text,x,y,shadowsize,update);

}

void text_free(text_type* ptext)
{
  int c;
  for( c = 0; c < 78; ++c)
    texture_free(&ptext->chars[c]);
}
