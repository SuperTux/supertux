/*
  menu.c
  
  Super Tux - Menu
  
  by Tobias Glaesser
  tobi.web@gmx.de
  http://www.newbreedsoftware.com/supertux/
  
  December 20, 2003 - March 15, 2004
*/

#ifndef WIN32
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
MenuAction menuaction;
bool show_menu;
bool menu_change;
texture_type checkbox, checkbox_checked, back, arrow_left, arrow_right;

Menu* main_menu      = 0;
Menu* game_menu      = 0;
Menu* options_menu   = 0;
Menu* options_controls_menu   = 0;
Menu* highscore_menu = 0;
Menu* load_game_menu = 0;
Menu* save_game_menu = 0;

Menu* current_menu = 0;

/* input implementation variables */
int delete_character;
char mn_input_char;

/* Set the current menu */
void
Menu::set_current(Menu* pmenu)
{
  if(pmenu != current_menu)
    {
      menu_change  = true;
      Menu* tmp = current_menu;
      current_menu = pmenu;
      if(tmp)
        if(tmp->last_menu != pmenu)
          current_menu->last_menu = tmp;

      timer_start(&pmenu->effect, 500);
    }
}

/* Return a pointer to a new menu item */
menu_item_type* menu_item_create(MenuItemKind kind, char *text, int init_toggle, Menu* target_menu)
{
  menu_item_type *pnew_item = (menu_item_type*) malloc(sizeof(menu_item_type));
  pnew_item->kind = kind;
  pnew_item->text = (char*) malloc(sizeof(char) * (strlen(text) + 1));
  strcpy(pnew_item->text,text);
  if(kind == MN_TOGGLE)
    pnew_item->toggled = init_toggle;
  else
    pnew_item->toggled = false;
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

void menu_item_change_text(menu_item_type* pmenu_item,const  char *text)
{
  if(text)
    {
      free(pmenu_item->text);
      pmenu_item->text = (char*) malloc(sizeof(char )*(strlen(text)+1));
      strcpy(pmenu_item->text,text);
    }
}
void menu_item_change_input(menu_item_type* pmenu_item,const  char *text)
{
  if(text)
    {
      free(pmenu_item->input);
      pmenu_item->input = (char*) malloc(sizeof(char )*(strlen(text)+1));
      strcpy(pmenu_item->input,text);
    }
}

/* Free a menu and all its items */
Menu::~Menu()
{
  if(num_items != 0 && item != NULL)
    {
      for(int i = 0; i < num_items; ++i)
        {
          free(item[i].text);
          free(item[i].input);
          string_list_free(item[i].list);
        }
      free(item);
    }
}

Menu::Menu()
{
  pos_x        = screen->w/2;
  pos_y        = screen->h/2;
  arrange_left = 0;
  num_items    = 0;
  active_item  = 0;
  item         = NULL;
  timer_init(&effect,false);
}

void
Menu::additem(MenuItemKind kind, char *text, int toggle, Menu* menu)
{
  additem(menu_item_create(kind, text, toggle, menu));
}

/* Add an item to a menu */
void
Menu::additem(menu_item_type* pmenu_item)
{
  ++num_items;
  item = (menu_item_type*)realloc(item, sizeof(menu_item_type) * num_items);
  memcpy(&item[num_items-1],pmenu_item,sizeof(menu_item_type));
  free(pmenu_item);
}

/* Process actions done on the menu */
void
Menu::action()
{
  if(num_items != 0 && item != NULL)
    {
      switch(menuaction)
        {
        case MENU_ACTION_UP:
          if (active_item > 0)
            --active_item;
          else
            active_item = num_items-1;
          break;

        case MENU_ACTION_DOWN:
          if(active_item < num_items-1)
            ++active_item;
          else
            active_item = 0;
          break;

        case MENU_ACTION_LEFT:
          if(item[active_item].kind == MN_STRINGSELECT
              && item[active_item].list->num_items != 0)
            {
              if(item[active_item].list->active_item > 0)
                --item[active_item].list->active_item;
              else
                item[active_item].list->active_item = item[active_item].list->num_items-1;
            }
          break;
        case MENU_ACTION_RIGHT:
          if(item[active_item].kind == MN_STRINGSELECT
              && item[active_item].list->num_items != 0)
            {
              if(item[active_item].list->active_item < item[active_item].list->num_items-1)
                ++item[active_item].list->active_item;
              else
                item[active_item].list->active_item = 0;
            }
          break;

        case MENU_ACTION_HIT:
          {
            switch (item[active_item].kind)
              {
              case MN_GOTO:
                if (item[active_item].target_menu != NULL)
                  Menu::set_current(item[active_item].target_menu);
                else
                  puts("NULLL");
                break;

              case MN_TOGGLE:
                item[active_item].toggled = !item[active_item].toggled;
                menu_change = true;
                break;

              case MN_ACTION:
              case MN_TEXTFIELD:
              case MN_NUMFIELD:
              case MN_CONTROLFIELD:
                item[active_item].toggled = true;
                break;

              case MN_BACK:
                if(last_menu != NULL)
                  Menu::set_current(last_menu);
                break;
              default:
                break;
              }
          }
          break;

        case MENU_ACTION_REMOVE:
          if(item[active_item].kind == MN_TEXTFIELD
              || item[active_item].kind == MN_NUMFIELD)
            {
              if(item[active_item].input != NULL)
                {
                  int i = strlen(item[active_item].input);

                  while(delete_character > 0)	/* remove charactes */
                    {
                      item[active_item].input[i-1] = '\0';
                      delete_character--;
                    }
                }
            }
          break;

        case MENU_ACTION_INPUT:
          if(item[active_item].kind == MN_TEXTFIELD
              || (item[active_item].kind == MN_NUMFIELD && mn_input_char >= '0' && mn_input_char <= '9'))
            {
              if(item[active_item].input != NULL)
                {
                  int i = strlen(item[active_item].input);
                  item[active_item].input = (char*) realloc(item[active_item].input,sizeof(char)*(i + 2));
                  item[active_item].input[i] = mn_input_char;
                  item[active_item].input[i+1] = '\0';
                }
              else
                {
                  item[active_item].input = (char*) malloc(2*sizeof(char));
                  item[active_item].input[0] = mn_input_char;
                  item[active_item].input[1] = '\0';
                }
            }
          break;

        case MENU_ACTION_NONE:
          break;
        }
    }

  menu_item_type& new_item = item[active_item];
  if(new_item.kind == MN_DEACTIVE
      || new_item.kind == MN_LABEL
      || new_item.kind == MN_HL)
    {
      // Skip the horzontal line item
      if(menuaction != MENU_ACTION_UP && menuaction != MENU_ACTION_DOWN)
        menuaction = MENU_ACTION_DOWN;

      if(num_items > 1)
        action();
    }
}

/* Check, if the value of the active menu item has changed. */
int
Menu::check()
{
  if(num_items != 0 && item != NULL)
    {
      if((item[active_item].kind == MN_ACTION
          || item[active_item].kind == MN_TEXTFIELD
          || item[active_item].kind == MN_NUMFIELD)
          && item[active_item].toggled)
        {
          item[active_item].toggled = false;
          show_menu = 0;
          return active_item;
        }
      else if(item[active_item].kind == MN_TOGGLE || item[active_item].kind == MN_GOTO)
        {
          return active_item;
        }
      else
        return -1;
    }
  else
    return -1;
}

void
Menu::draw_item(int index, // Position of the current item in the menu
                int menu_width,
                int menu_height)
{
  int font_width  = 16;

  const menu_item_type& pitem =  item[index];

  int effect_offset = 0;
  {
    int effect_time = 0;
    if(timer_check(&effect))
      effect_time = timer_get_left(&effect) / 4;

    effect_offset = (index % 2) ? effect_time : -effect_time;
  }

  int x_pos       = pos_x;
  int y_pos       = pos_y + 24*index - menu_height/2 + 12 + effect_offset;
  int shadow_size = 2;
  int text_width  = strlen(pitem.text) * font_width;
  int input_width = strlen(pitem.input) * font_width;
  int list_width  = strlen(string_list_active(pitem.list)) * font_width;
  text_type* text_font = &white_text;

  if (arrange_left)
    x_pos += 24 - menu_width/2 + (text_width + input_width + list_width)/2;

  if(index == active_item)
    {
      shadow_size = 3;
      text_font = &blue_text;
    }

  switch (pitem.kind)
    {
    case MN_DEACTIVE:
      {
        text_draw_align(&black_text, pitem.text,
                        x_pos, y_pos,
                        A_HMIDDLE, A_VMIDDLE, 2);
        break;
      }

    case MN_HL:
      {
        int x = pos_x - menu_width/2;
        int y = y_pos - 12 - effect_offset;
        /* Draw a horizontal line with a little 3d effect */
        fillrect(x, y + 6,
                 menu_width, 4,
                 210,50,50,225);
        fillrect(x, y + 6,
                 menu_width, 2,
                 0,0,0,255);
        break;
      }
    case MN_LABEL:
      {
        text_draw_align(&white_big_text, pitem.text,
                        x_pos, y_pos,
                        A_HMIDDLE, A_VMIDDLE, 2);
        break;
      }
    case MN_TEXTFIELD:
    case MN_NUMFIELD:
    case MN_CONTROLFIELD:
      {
        int input_pos = input_width/2;
        int text_pos  = (text_width + font_width)/2;

        fillrect(x_pos - input_pos + text_pos - 1, y_pos - 10,
                 input_width + font_width + 2, 20,
                 255,255,255,255);
        fillrect(x_pos - input_pos + text_pos, y_pos - 9,
                 input_width + font_width, 18,
                 0,0,0,128);

        text_draw_align(&gold_text, pitem.input,
                        x_pos + text_pos, y_pos,
                        A_HMIDDLE, A_VMIDDLE, 2);

        text_draw_align(text_font, pitem.text,
                        x_pos - (input_width + font_width)/2, y_pos,
                        A_HMIDDLE, A_VMIDDLE, shadow_size);
        break;
      }
    case MN_STRINGSELECT:
      {
        int list_pos_2 = list_width + font_width;
        int list_pos   = list_width/2;
        int text_pos   = (text_width + font_width)/2;

        /* Draw arrows */
        texture_draw(&arrow_left,  x_pos - list_pos + text_pos - 17, y_pos - 8);
        texture_draw(&arrow_right, x_pos - list_pos + text_pos - 1 + list_pos_2, y_pos - 8);

        /* Draw input background */
        fillrect(x_pos - list_pos + text_pos - 1, y_pos - 10,
                 list_pos_2 + 2, 20,
                 255,255,255,255);
        fillrect(x_pos - list_pos + text_pos, y_pos - 9,
                 list_pos_2, 18,
                 0,0,0,128);

        text_draw_align(&gold_text, string_list_active(pitem.list),
                        x_pos + text_pos, y_pos,
                        A_HMIDDLE, A_VMIDDLE,2);

        text_draw_align(text_font, pitem.text,
                        x_pos - list_pos_2/2, y_pos,
                        A_HMIDDLE, A_VMIDDLE, shadow_size);
        break;
      }
    case MN_BACK:
      {
        text_draw_align(text_font, pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);
        texture_draw(&back, x_pos + text_width/2  + font_width, y_pos - 8);
        break;
      }

    case MN_TOGGLE:
      {
        text_draw_align(text_font, pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);

        if(pitem.toggled)
          texture_draw(&checkbox_checked,
                       x_pos + (text_width+font_width)/2,
                       y_pos - 8);
        else
          texture_draw(&checkbox,
                       x_pos + (text_width+font_width)/2,
                       y_pos - 8);
        break;
      }
    case MN_ACTION:
      text_draw_align(text_font, pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);
      break;

    case MN_GOTO:
      text_draw_align(text_font, pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);
      break;
    }
}

int Menu::width()
{
  /* The width of the menu has to be more than the width of the text
     with the most characters */
  int menu_width = 0;
  for(int i = 0; i < num_items; ++i)
    {
      int w = strlen(item[i].text) + (item[i].input ? strlen(item[i].input) + 1 : 0) + strlen(string_list_active(item[i].list));
      if( w > menu_width )
        {
          menu_width = w;
          if( item[i].kind == MN_TOGGLE)
            menu_width += 2;
        }
    }

  return (menu_width * 16 + 48);
}

int Menu::height()
{
  return ((num_items) * 24);
}

/* Draw the current menu. */
void
Menu::draw()
{
  int menu_height = height();
  int menu_width = width();

  /* Draw a transparent background */
  fillrect(pos_x - menu_width/2,
           pos_y - 24*num_items/2,
           menu_width,menu_height,150,150,150,100);

  for(int i = 0; i < num_items; ++i)
    {
      draw_item(i, menu_width, menu_height);
    }
}

/* Reset/Set global defaults */
void menu_reset(void)
{
  menu_change  = false;
  show_menu    = false;
  menuaction   = MENU_ACTION_NONE;
  current_menu = NULL;

  delete_character = 0;
  mn_input_char    = '\0';
}

/* --- MENU --- */
/* Draw the current menu and execute the (menu)events */
void menu_process_current(void)
{
  menu_change = false;

  if(current_menu != NULL)
    {
      current_menu->action();
      current_menu->draw();
    }

  menuaction = MENU_ACTION_NONE;
}

/* Check for menu event */
void menu_event(SDL_Event& event)
{
  SDLKey key;
  switch(event.type)
    {
    case SDL_KEYDOWN:
      key = event.key.keysym.sym;
      SDLMod keymod;
      char ch[2];
      keymod = SDL_GetModState();
      int x,y;

      /* If the current unicode character is an ASCII character,
         assign it to ch. */
      if ( (event.key.keysym.unicode & 0xFF80) == 0 )
        {
          ch[0] = event.key.keysym.unicode & 0x7F;
          ch[1] = '\0';
        }
      else
        {
          /* An International Character. */
        }

      switch(key)
        {
        case SDLK_UP:		/* Menu Up */
          menuaction = MENU_ACTION_UP;
          menu_change = true;
          break;
        case SDLK_DOWN:		/* Menu Down */
          menuaction = MENU_ACTION_DOWN;
          menu_change = true;
          break;
        case SDLK_LEFT:		/* Menu Up */
          menuaction = MENU_ACTION_LEFT;
          menu_change = true;
          break;
        case SDLK_RIGHT:		/* Menu Down */
          menuaction = MENU_ACTION_RIGHT;
          menu_change = true;
          break;
        case SDLK_SPACE:
          if(current_menu->item[current_menu->active_item].kind == MN_TEXTFIELD)
            {
              menuaction = MENU_ACTION_INPUT;
              menu_change = true;
              mn_input_char = ' ';
              break;
            }
        case SDLK_RETURN: /* Menu Hit */
          menuaction = MENU_ACTION_HIT;
          menu_change = true;
          break;
        case SDLK_DELETE:
        case SDLK_BACKSPACE:
          menuaction = MENU_ACTION_REMOVE;
          menu_change = true;
          delete_character++;
          break;
        default:
          if( (key >= SDLK_0 && key <= SDLK_9) || (key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_SPACE && key <= SDLK_SLASH))
            {
              menuaction = MENU_ACTION_INPUT;
              menu_change = true;
              mn_input_char = *ch;
            }
          else
            {
              mn_input_char = '\0';
            }
          break;
        }
      break;
    case  SDL_JOYAXISMOTION:
      if(event.jaxis.axis == JOY_Y)
        {
          if (event.jaxis.value > 1024)
            menuaction = MENU_ACTION_DOWN;
          else if (event.jaxis.value < -1024)
            menuaction = MENU_ACTION_UP;
        }
      break;
    case  SDL_JOYBUTTONDOWN:
      menuaction = MENU_ACTION_HIT;
      break;
    case SDL_MOUSEBUTTONDOWN:
      x = event.motion.x;
      y = event.motion.y;
      if(x > current_menu->pos_x - current_menu->width()/2 &&
          x < current_menu->pos_x + current_menu->width()/2 &&
          y > current_menu->pos_y - current_menu->height()/2 &&
          y < current_menu->pos_y + current_menu->height()/2)
        {
          menuaction = MENU_ACTION_HIT;
        }
      break;
    case SDL_MOUSEMOTION:
      x = event.motion.x;
      y = event.motion.y;
      if(x > current_menu->pos_x - current_menu->width()/2 &&
          x < current_menu->pos_x + current_menu->width()/2 &&
          y > current_menu->pos_y - current_menu->height()/2 &&
          y < current_menu->pos_y + current_menu->height()/2)
        {
          current_menu->active_item = (y - (current_menu->pos_y - current_menu->height()/2)) / 24;
          menu_change = true;
	  mouse_cursor->set_state(MC_LINK);
        }
	else
	{
	  mouse_cursor->set_state(MC_NORMAL);
	}
      break;
    default:
      break;
    }
}


// EOF //
