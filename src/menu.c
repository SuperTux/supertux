/*
  menu.c
  
  Super Tux - Menu
  
  by Tobias Glaesser
  tobi.web@gmx.de
  http://www.newbreedsoftware.com/supertux/
  
  December 20, 2003 - March 15, 2004
*/

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "globals.h"
#include "menu.h"
#include "screen.h"
#include "setup.h"
#include "sound.h"
#include "scene.h"
#include "leveleditor.h"
#include "timer.h"
#include "high_scores.h"

/* (global) menu variables */
int menuaction;
int show_menu;
int menu_change;
texture_type checkbox, checkbox_checked, back, arrow_left, arrow_right;

menu_type main_menu, game_menu, options_menu, highscore_menu, load_game_menu, save_game_menu;
menu_type* current_menu, * last_menu;

/* input implementation variables */
int delete_character;
char mn_input_char;

/* Set the current menu */
void menu_set_current(menu_type* pmenu)
{
  if(pmenu != current_menu)
    {
      menu_change = YES;
      last_menu = current_menu;
      current_menu = pmenu;
      timer_start(&pmenu->effect, 500);
    }
}

/* Return a pointer to a new menu item */
menu_item_type* menu_item_create(int kind, char *text, int init_toggle, void* target_menu)
{
  menu_item_type *pnew_item = (menu_item_type*) malloc(sizeof(menu_item_type));
  pnew_item->kind = kind;
  pnew_item->text = (char*) malloc(sizeof(char) * (strlen(text) + 1));
  strcpy(pnew_item->text,text);
  if(kind == MN_TOGGLE)
    pnew_item->toggled = init_toggle;
  else
    pnew_item->toggled = NO;
  pnew_item->target_menu = target_menu;
  pnew_item->input = (char*) malloc(sizeof(char));
  pnew_item->input[0] = '\0';
  if(kind == MN_STRINGSELECT)
    {
      pnew_item->list = (string_list_type*) malloc(sizeof(string_list_type));
      string_list_init(pnew_item->list);
    }
  else
    pnew_item->list = NULL;
  return pnew_item;
}

void menu_item_change_text(menu_item_type* pmenu_item, char *text)
{
  if(text)
    {
      free(pmenu_item->text);
      pmenu_item->text = (char*) malloc(sizeof(char )*(strlen(text)+1));
      strcpy(pmenu_item->text,text);
    }
}
void menu_item_change_input(menu_item_type* pmenu_item, char *text)
{
  if(text)
    {
      free(pmenu_item->input);
      pmenu_item->input = (char*) malloc(sizeof(char )*(strlen(text)+1));
      strcpy(pmenu_item->input,text);
    }
}

/* Free a menu and all its items */
void menu_free(menu_type* pmenu)
{
  int i;
  if(pmenu->num_items != 0 && pmenu->item != NULL)
    {
      for(i = 0; i < pmenu->num_items; ++i)
        {
          free(pmenu->item[i].text);
          free(pmenu->item[i].input);
          string_list_free(pmenu->item[i].list);
        }
      free(pmenu->item);
    }
}

/* Initialize a menu */
void menu_init(menu_type* pmenu)
{
  pmenu->arrange_left = 0;
  pmenu->num_items = 0;
  pmenu->active_item = 0;
  pmenu->item = NULL;
  timer_init(&pmenu->effect,NO);
}

/* Add an item to a menu */
void menu_additem(menu_type* pmenu, menu_item_type* pmenu_item)
{
  ++pmenu->num_items;
  pmenu->item = (menu_item_type*) realloc(pmenu->item, sizeof(menu_item_type) * pmenu->num_items);
  memcpy(&pmenu->item[pmenu->num_items-1],pmenu_item,sizeof(menu_item_type));
  free(pmenu_item);
}

/* Process actions done on the menu */
void menu_action(menu_type* pmenu)
{
  int i;

  if(pmenu->num_items != 0 && pmenu->item != NULL)
    {
      switch(menuaction)
        {
        case MN_UP:
          if(pmenu->active_item > 0)
            --pmenu->active_item;
          else
            pmenu->active_item = pmenu->num_items-1;
          break;
        case MN_DOWN:
          if(pmenu->active_item < pmenu->num_items-1)
            ++pmenu->active_item;
          else
            pmenu->active_item = 0;
          break;
        case MN_LEFT:
          if(pmenu->item[pmenu->active_item].kind == MN_STRINGSELECT && pmenu->item[pmenu->active_item].list->num_items != 0)
            {
              if(pmenu->item[pmenu->active_item].list->active_item > 0)
                --pmenu->item[pmenu->active_item].list->active_item;
              else
                pmenu->item[pmenu->active_item].list->active_item = pmenu->item[pmenu->active_item].list->num_items-1;
            }
          break;
        case MN_RIGHT:
          if(pmenu->item[pmenu->active_item].kind == MN_STRINGSELECT && pmenu->item[pmenu->active_item].list->num_items != 0)
            {
              if(pmenu->item[pmenu->active_item].list->active_item < pmenu->item[pmenu->active_item].list->num_items-1)
                ++pmenu->item[pmenu->active_item].list->active_item;
              else
                pmenu->item[pmenu->active_item].list->active_item = 0;
            }
          break;
        case MN_HIT:
          if(pmenu->item[pmenu->active_item].kind == MN_GOTO && pmenu->item[pmenu->active_item].target_menu != NULL)
            menu_set_current((menu_type*)pmenu->item[pmenu->active_item].target_menu);
          else if(pmenu->item[pmenu->active_item].kind == MN_TOGGLE)
            {
              pmenu->item[pmenu->active_item].toggled = !pmenu->item[pmenu->active_item].toggled;
              menu_change = YES;
            }
          else if(pmenu->item[pmenu->active_item].kind == MN_ACTION || pmenu->item[pmenu->active_item].kind == MN_TEXTFIELD || pmenu->item[pmenu->active_item].kind == MN_NUMFIELD)
            {
              pmenu->item[pmenu->active_item].toggled = YES;
            }
          else if(pmenu->item[pmenu->active_item].kind == MN_BACK)
            {
              if(last_menu != NULL)
                menu_set_current(last_menu);
            }
          break;
        case MN_REMOVE:
          if(pmenu->item[pmenu->active_item].kind == MN_TEXTFIELD || pmenu->item[pmenu->active_item].kind == MN_NUMFIELD)
            {
              if(pmenu->item[pmenu->active_item].input != NULL)
                {
                  i = strlen(pmenu->item[pmenu->active_item].input);

                  while(delete_character > 0)	/* remove charactes */
                    {
                      pmenu->item[pmenu->active_item].input[i-1] = '\0';
                      delete_character--;
                    }
                }
            }
          break;
        case MN_INPUT:
          if(pmenu->item[pmenu->active_item].kind == MN_TEXTFIELD || (pmenu->item[pmenu->active_item].kind == MN_NUMFIELD && mn_input_char >= '0' && mn_input_char <= '9'))
            {
              if(pmenu->item[pmenu->active_item].input != NULL)
                {
                  i = strlen(pmenu->item[pmenu->active_item].input);
                  pmenu->item[pmenu->active_item].input = (char*) realloc(pmenu->item[pmenu->active_item].input,sizeof(char)*(i + 2));
                  pmenu->item[pmenu->active_item].input[i] = mn_input_char;
                  pmenu->item[pmenu->active_item].input[i+1] = '\0';
                }
              else
                {
                  pmenu->item[pmenu->active_item].input = (char*) malloc(2*sizeof(char));
                  pmenu->item[pmenu->active_item].input[0] = mn_input_char;
                  pmenu->item[pmenu->active_item].input[1] = '\0';
                }
            }
          break;
        }
    }

  if(pmenu->item[pmenu->active_item].kind == MN_DEACTIVE || pmenu->item[pmenu->active_item].kind == MN_LABEL || pmenu->item[pmenu->active_item].kind == MN_HL)
    {
      if(menuaction != MN_UP && menuaction != MN_DOWN)
        menuaction = MN_DOWN;

      if(pmenu->num_items > 1)
        menu_action(pmenu);
    }

}

/* Check, if the value of the active menu item has changed. */
int menu_check(menu_type* pmenu)
{
  if(pmenu->num_items != 0 && pmenu->item != NULL)
    {
      if((pmenu->item[pmenu->active_item].kind == MN_ACTION || pmenu->item[pmenu->active_item].kind == MN_TEXTFIELD || pmenu->item[pmenu->active_item].kind == MN_NUMFIELD) && pmenu->item[pmenu->active_item].toggled == YES)
        {
          pmenu->item[pmenu->active_item].toggled = NO;
          show_menu = 0;
          return pmenu->active_item;
        }
      else if(pmenu->item[pmenu->active_item].kind == MN_TOGGLE || pmenu->item[pmenu->active_item].kind == MN_GOTO)
        {
          return pmenu->active_item;
        }
      else
        return -1;
    }
  else
    return -1;
}

/* Draw the current menu. */
void menu_draw(menu_type* pmenu)
{
  int i, y, a, b, e, f, menu_height, menu_width;

  /* The width of the menu has to be more than the width of the text with the most characters */
  menu_width = 0;
  for(i = 0; i < pmenu->num_items; ++i)
    {
      y = strlen(pmenu->item[i].text) + (pmenu->item[i].input ? strlen(pmenu->item[i].input) + 1 : 0) + strlen(string_list_active(pmenu->item[i].list));
      if( y > menu_width )
        {
          menu_width = y;
          if( pmenu->item[i].kind == MN_TOGGLE)
            menu_width += 2;
        }
    }
  if(pmenu->arrange_left == YES)
    a = menu_width * 16;
  else
    a = 0;

  menu_width = menu_width * 16 + 48;
  menu_height = (pmenu->num_items) * 24;

  /* Draw a transparent background */
  fillrect(screen->w/2 - menu_width/2,screen->h/2-(((pmenu->num_items)*24)/2),menu_width,menu_height,150,150,150,100);

  if(timer_check(&pmenu->effect))
    {
      e = timer_get_left(&pmenu->effect) / 4;
    }
  else
    {
      e = 0;
    }

  for(i = 0; i < pmenu->num_items; ++i)
    {
      if(pmenu->arrange_left == YES)
        b = (a - ((strlen(pmenu->item[i].text)+strlen(pmenu->item[i].input)+ strlen(string_list_active(pmenu->item[i].list))) * 16)) / 2;
      else
        b = 0;

      if(e != 0)
        {
          if(i % 2)
            f = e;
          else
            f = -e;
        }
      else
        f = 0;

      if(pmenu->item[i].kind == MN_DEACTIVE)
        {
          text_drawf(&black_text,pmenu->item[i].text, - b,(i)*24 - menu_height/2 + 10 + f,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
        }
      else if(pmenu->item[i].kind == MN_HL)
        {
          /* Draw a horizontal line with a little 3d effect */
          fillrect(screen->w/2 - menu_width/2,(i)*24 - menu_height/2 + 6 + screen->h /2,menu_width,4,210,50,50,225);
          fillrect(screen->w/2 - menu_width/2,(i)*24 - menu_height/2 + 10 + screen->h /2,menu_width,2,0,0,0,255);
        }
      else if(pmenu->item[i].kind == MN_LABEL)
        {
          text_drawf(&white_big_text,pmenu->item[i].text, - b,(i)*24 - menu_height/2 + 10,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
        }
      else if(pmenu->item[i].kind == MN_TEXTFIELD || pmenu->item[i].kind == MN_NUMFIELD)
        {
          fillrect(-b +screen->w/2 - ((strlen(pmenu->item[i].input)*16)/2) + ((strlen(pmenu->item[i].text) + 1)*16)/2 - 1,(i)*24 - menu_height/2 + 10 + screen->h /2 - 10 + f,(strlen(pmenu->item[i].input)+1)*16 + 2,20,255,255,255,255);
          fillrect(- b +screen->w/2 - ((strlen(pmenu->item[i].input)*16)/2) + ((strlen(pmenu->item[i].text) + 1)*16)/2,(i)*24 - menu_height/2 + 10 + screen->h /2 - 9 + f,(strlen(pmenu->item[i].input)+1)*16,18,0,0,0,128);
          text_drawf(&gold_text,pmenu->item[i].input, - b + ((strlen(pmenu->item[i].text)+1) * 16)/2,(i)*24 - menu_height/2 + 10 + f,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
          if(i == pmenu->active_item)
            {
              text_drawf(&blue_text,pmenu->item[i].text, - b  -(((strlen(pmenu->item[i].input)+1) * 16)/2),(i)*24 - menu_height/2 + 10 + f,A_HMIDDLE, A_VMIDDLE,3,NO_UPDATE);
            }
          else
            {
              text_drawf(&white_text,pmenu->item[i].text, - b  -(((strlen(pmenu->item[i].input)+1) * 16)/2),(i)*24 - menu_height/2 +10 + f,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
            }
        }
      else if(pmenu->item[i].kind == MN_STRINGSELECT)
        {
          /* Draw arrows */
          texture_draw(&arrow_left,-b +screen->w/2 - ((strlen(string_list_active(pmenu->item[i].list))*16)/2) + ((strlen(pmenu->item[i].text) + 1)*16)/2 - 17,(i)*24 - menu_height/2 + 10 + screen->h / 2 -8 + f,NO_UPDATE);
          texture_draw(&arrow_right,-b +screen->w/2 - ((strlen(string_list_active(pmenu->item[i].list))*16)/2) + ((strlen(pmenu->item[i].text) + 1)*16)/2 - 1 + (strlen(string_list_active(pmenu->item[i].list))+1)*16,(i)*24 - menu_height/2 + 10 + screen->h / 2 -8 + f,NO_UPDATE);
          /* Draw input background */
          fillrect(-b +screen->w/2 - ((strlen(string_list_active(pmenu->item[i].list))*16)/2) + ((strlen(pmenu->item[i].text) + 1)*16)/2 - 1,(i)*24 - menu_height/2 + 10 + screen->h /2 - 10 + f,(strlen(string_list_active(pmenu->item[i].list))+1)*16 + 2,20,255,255,255,255);
          fillrect(- b +screen->w/2 - ((strlen(string_list_active(pmenu->item[i].list))*16)/2) + ((strlen(pmenu->item[i].text) + 1)*16)/2,(i)*24 - menu_height/2 + 10 + screen->h /2 - 9 + f,(strlen(string_list_active(pmenu->item[i].list))+1)*16,18,0,0,0,128);

          text_drawf(&gold_text,string_list_active(pmenu->item[i].list), - b + ((strlen(pmenu->item[i].text)+1) * 16)/2,(i)*24 - menu_height/2 + 10 + f,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
          if(i == pmenu->active_item)
            {
              text_drawf(&blue_text,pmenu->item[i].text, - b  -(((strlen(string_list_active(pmenu->item[i].list))+1) * 16)/2),(i)*24 - menu_height/2 + 10 + f,A_HMIDDLE, A_VMIDDLE,3,NO_UPDATE);
            }
          else
            {
              text_drawf(&white_text,pmenu->item[i].text, - b  -(((strlen(string_list_active(pmenu->item[i].list))+1) * 16)/2),(i)*24 - menu_height/2 +10 + f,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
            }
        }
      else if(i == pmenu->active_item)
        {
          text_drawf(&blue_text,pmenu->item[i].text, - b,(i)*24 - menu_height/2 + 10 + f ,A_HMIDDLE, A_VMIDDLE,3,NO_UPDATE);
        }
      else
        {
          text_drawf(&white_text,pmenu->item[i].text, - b,(i)*24 - menu_height/2 + 10 + f ,A_HMIDDLE, A_VMIDDLE,2,NO_UPDATE);
        }
      if(pmenu->item[i].kind == MN_TOGGLE)
        {

          if(pmenu->item[i].toggled == YES)
            texture_draw(&checkbox_checked, - b + screen->w / 2 + (strlen(pmenu->item[i].text) * 16)/2  + 16,(i)*24 - menu_height/2 + 10 + screen->h / 2 -8 + f,NO_UPDATE);
          else
            texture_draw(&checkbox, - b + screen->w / 2 + (strlen(pmenu->item[i].text) * 16)/2 + 16,(i)*24 - menu_height/2 + 10 + screen->h / 2 - 8 + f,NO_UPDATE);
        }
      else if(pmenu->item[i].kind == MN_BACK)
        {
          texture_draw(&back, - b + screen->w / 2 + (strlen(pmenu->item[i].text) * 16)/2  + 16,(i)*24 - menu_height/2 + 10 + screen->h / 2 -8 + f,NO_UPDATE);
        }
    }
}

/* Reset/Set global defaults */
void menu_reset(void)
{
  menu_change = NO;
  show_menu = NO;
  menuaction = -1;
  current_menu = NULL;
  last_menu = NULL;

  delete_character = 0;
  mn_input_char = '\0';
}

/* --- MENU --- */
/* Draw the current menu and execute the (menu)events */
void menu_process_current(void)
{
  menu_change = NO;

  if(current_menu != NULL)
    {
      menu_action(current_menu);
      menu_draw(current_menu);
    }

  menuaction = -1;
}

/* Check for menu event */
void menu_event(SDL_keysym* keysym)
{
  SDLKey key = keysym->sym;
  SDLMod keymod;
  char ch[2];
  keymod = SDL_GetModState();

  /* If the current unicode character is an ASCII character,
     assign it to ch. */
  if ( (keysym->unicode & 0xFF80) == 0 )
    {
      ch[0] = keysym->unicode & 0x7F;
      ch[1] = '\0';
    }
  else
    {
      /* An International Character. */
    }

  switch(key)
    {
    case SDLK_UP:		/* Menu Up */
      menuaction = MN_UP;
      menu_change = YES;
      break;
    case SDLK_DOWN:		/* Menu Down */
      menuaction = MN_DOWN;
      menu_change = YES;
      break;
    case SDLK_LEFT:		/* Menu Up */
      menuaction = MN_LEFT;
      menu_change = YES;
      break;
    case SDLK_RIGHT:		/* Menu Down */
      menuaction = MN_RIGHT;
      menu_change = YES;
      break;
    case SDLK_SPACE:
      if(current_menu->item[current_menu->active_item].kind == MN_TEXTFIELD)
      {
      menuaction = MN_INPUT;
      menu_change = YES;
      mn_input_char = ' ';
      break;
      }
    case SDLK_RETURN: /* Menu Hit */
      menuaction = MN_HIT;
      menu_change = YES;
      break;
    case SDLK_DELETE:
    case SDLK_BACKSPACE:
      menuaction = MN_REMOVE;
      menu_change = YES;
      delete_character++;
      break;
    default:
      if( (key >= SDLK_0 && key <= SDLK_9) || (key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_SPACE && key <= SDLK_SLASH))
        {
          menuaction = MN_INPUT;
          menu_change = YES;
          mn_input_char = *ch;
        }
      else
        {
          mn_input_char = '\0';
        }
      break;
    }


  /* FIXME: NO JOYSTICK SUPPORT */
  /*#ifdef JOY_YES
  else if (event.type == SDL_JOYBUTTONDOWN)
   {
      Joystick button: Continue:

     done = 1;
   }
  #endif*/
}

