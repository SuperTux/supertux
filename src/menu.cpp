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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef WIN32
#include <sys/types.h>
#include <ctype.h>
#endif

#include <iostream>
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

Surface* checkbox;
Surface* checkbox_checked;
Surface* back;
Surface* arrow_left;
Surface* arrow_right;

Menu* main_menu      = 0;
Menu* game_menu      = 0;
Menu* worldmap_menu  = 0;
Menu* options_menu   = 0;
Menu* options_controls_menu   = 0;
Menu* highscore_menu = 0;
Menu* load_game_menu = 0;
Menu* save_game_menu = 0;
Menu* contrib_menu   = 0;
Menu* contrib_subset_menu   = 0;

std::vector<Menu*> Menu::last_menus;
Menu* Menu::current_ = 0;

void
Menu::push_current(Menu* pmenu)
{
  if (current_)
    last_menus.push_back(current_);
  
  current_ = pmenu;
  current_->effect.start(500);
}

void
Menu::pop_current()
{
  if (!last_menus.empty())
    {
      current_ = last_menus.back();
      current_->effect.start(500);

      last_menus.pop_back();
    }
  else
    {
      current_ = 0;
    }
}

void
Menu::set_current(Menu* menu)
{
  last_menus.clear();

  if (menu)
    menu->effect.start(500);
  
  current_ = menu;
}

/* Return a pointer to a new menu item */
MenuItem*
MenuItem::create(MenuItemKind kind_, const char *text_, int init_toggle_, Menu* target_menu_, int* int_p_)
{
  MenuItem *pnew_item = new MenuItem;
  
  pnew_item->kind = kind_;
  pnew_item->text = (char*) malloc(sizeof(char) * (strlen(text_) + 1));
  strcpy(pnew_item->text, text_);

  if(kind_ == MN_TOGGLE)
    pnew_item->toggled = init_toggle_;
  else
    pnew_item->toggled = false;

  pnew_item->target_menu = target_menu_;
  pnew_item->input = (char*) malloc(sizeof(char));
  pnew_item->input[0] = '\0';

  if(kind_ == MN_STRINGSELECT)
    {
      pnew_item->list = (string_list_type*) malloc(sizeof(string_list_type));
      string_list_init(pnew_item->list);
    }
  else
    pnew_item->list = NULL;

  pnew_item->int_p = int_p_;

  return pnew_item;
}

void
MenuItem::change_text(const  char *text_)
{
  if (text_)
    {
      free(text);
      text = (char*) malloc(sizeof(char )*(strlen(text_)+1));
      strcpy(text, text_);
    }
}

void
MenuItem::change_input(const  char *text_)
{
  if(text)
    {
      free(input);
      input = (char*) malloc(sizeof(char )*(strlen(text_)+1));
      strcpy(input, text_);
    }
}

/* Free a menu and all its items */
Menu::~Menu()
{
  if(item.size() != 0)
    {
      for(unsigned int i = 0; i < item.size(); ++i)
        {
          free(item[i].text);
          free(item[i].input);
          string_list_free(item[i].list);
        }
    }
}


Menu::Menu()
{
  hit_item = -1;
  menuaction = MENU_ACTION_NONE;
  delete_character = 0;
  mn_input_char = '\0';
  
  pos_x        = screen->w/2;
  pos_y        = screen->h/2;
  has_backitem = false;
  arrange_left = 0;
  active_item  = 0;
  effect.init(false);
}

void Menu::set_pos(int x, int y, float rw, float rh)
{
  pos_x = x + (int)((float)get_width() * rw);
  pos_y = y + (int)((float)get_height() * rh);
}

void
Menu::additem(MenuItemKind kind_, const std::string& text_, int toggle_, Menu* menu_, int* int_p)
{
  if(kind_ == MN_BACK)
    has_backitem = true;

  additem(MenuItem::create(kind_, text_.c_str(), toggle_, menu_, int_p));
}

/* Add an item to a menu */
void
Menu::additem(MenuItem* pmenu_item)
{
  if(pmenu_item->kind == MN_BACK)
    has_backitem = true;

  item.push_back(*pmenu_item);
  delete pmenu_item;
}

void
Menu::clear()
{
  item.clear();
}

/* Process actions done on the menu */
void
Menu::action()
{
  hit_item = -1;
  if(item.size() != 0)
    {
      switch(menuaction)
        {
        case MENU_ACTION_UP:
          if (active_item > 0)
            --active_item;
          else
            active_item = int(item.size())-1;
          break;

        case MENU_ACTION_DOWN:
          if(active_item < int(item.size())-1)
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
            hit_item = active_item;
            switch (item[active_item].kind)
              {
              case MN_GOTO:
                if (item[active_item].target_menu != NULL)
                  Menu::push_current(item[active_item].target_menu);
                else
                  puts("NULLL");
                break;

              case MN_TOGGLE:
                item[active_item].toggled = !item[active_item].toggled;
                break;

              case MN_ACTION:
              case MN_TEXTFIELD:
              case MN_NUMFIELD:
                Menu::set_current(0); 
                item[active_item].toggled = true;
                break;
              case MN_CONTROLFIELD:
                break;

              case MN_BACK:
                Menu::pop_current();
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

        case MENU_ACTION_NONE:
          break;
        }
    }

  MenuItem& new_item = item[active_item];
  if(new_item.kind == MN_DEACTIVE
      || new_item.kind == MN_LABEL
      || new_item.kind == MN_HL)
    {
      // Skip the horzontal line item
      if (menuaction != MENU_ACTION_UP && menuaction != MENU_ACTION_DOWN)
        menuaction = MENU_ACTION_DOWN;

      if (item.size() > 1)
        action();
    }

  menuaction = MENU_ACTION_NONE;
}

int
Menu::check()
{
  return hit_item;
  /*
  if (item.size() != 0)
    {
      if((item[active_item].kind == MN_ACTION
          || item[active_item].kind == MN_TEXTFIELD
          || item[active_item].kind == MN_NUMFIELD)
          && item[active_item].toggled)
        { 
          item[active_item].toggled = false;
          Menu::set_current(0);
          return active_item;
        }
      else if(item[active_item].kind == MN_TOGGLE 
              || item[active_item].kind == MN_GOTO)
        {
          return active_item;
        }
      else
        return -1;
    }
  else
    return -1;
  */
}

void
Menu::draw_item(int index, // Position of the current item in the menu
                int menu_width,
                int menu_height)
{
  const MenuItem& pitem = item[index];

  int font_width  = 16;
  int effect_offset = 0;
  {
    int effect_time = 0;

    if(effect.check())
      effect_time = effect.get_left() / 4;

    effect_offset = (index % 2) ? effect_time : -effect_time;
  }

  int x_pos       = pos_x;
  int y_pos       = pos_y + 24*index - menu_height/2 + 12 + effect_offset;
  int shadow_size = 2;
  int text_width  = strlen(pitem.text) * font_width;
  int input_width = strlen(pitem.input) * font_width;
  int list_width  = strlen(string_list_active(pitem.list)) * font_width;
  Text* text_font = white_text;

  if (arrange_left)
    x_pos += 24 - menu_width/2 + (text_width + input_width + list_width)/2;

  if(index == active_item)
    {
      shadow_size = 3;
      text_font = blue_text;
    }

  switch (pitem.kind)
    {
    case MN_DEACTIVE:
      {
        black_text->draw_align(pitem.text,
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
                 150,200,255,225);
        fillrect(x, y + 6,
                 menu_width, 2,
                 255,255,255,255);
        break;
      }
    case MN_LABEL:
      {
        white_big_text->draw_align(pitem.text,
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

        gold_text->draw_align(pitem.input,
                              x_pos + text_pos, y_pos,
                              A_HMIDDLE, A_VMIDDLE, 2);

        text_font->draw_align(pitem.text,
                              x_pos - (input_width + font_width)/2, y_pos,
                              A_HMIDDLE, A_VMIDDLE, shadow_size);
        break;
      }
//    case MN_CONTROLFIELD:
//      {
        /* display key */  // FIXME: the key number is not that obvious to the user :P
/*        char str[12];
        sprintf(str, "%i", *pitem.int_p);
        input_width = strlen(str) * font_width;

        int input_pos = input_width/2;
        int text_pos  = (text_width + font_width)/2;

        fillrect(x_pos - input_pos + text_pos - 1, y_pos - 10,
                 input_width + font_width + 2, 20,
                 255,255,255,255);
        fillrect(x_pos - input_pos + text_pos, y_pos - 9,
                 input_width + font_width, 18,
                 0,0,0,128);

        gold_text->draw_align(str,
                              x_pos + text_pos, y_pos,
                              A_HMIDDLE, A_VMIDDLE, 2);

        text_font->draw_align(pitem.text,
                              x_pos - (input_width + font_width)/2, y_pos,
                              A_HMIDDLE, A_VMIDDLE, shadow_size);
        break;
      }*/
    case MN_STRINGSELECT:
      {
        int list_pos_2 = list_width + font_width;
        int list_pos   = list_width/2;
        int text_pos   = (text_width + font_width)/2;

        /* Draw arrows */
        arrow_left->draw(  x_pos - list_pos + text_pos - 17, y_pos - 8);
        arrow_right->draw( x_pos - list_pos + text_pos - 1 + list_pos_2, y_pos - 8);

        /* Draw input background */
        fillrect(x_pos - list_pos + text_pos - 1, y_pos - 10,
                 list_pos_2 + 2, 20,
                 255,255,255,255);
        fillrect(x_pos - list_pos + text_pos, y_pos - 9,
                 list_pos_2, 18,
                 0,0,0,128);

        gold_text->draw_align(string_list_active(pitem.list),
                        x_pos + text_pos, y_pos,
                        A_HMIDDLE, A_VMIDDLE,2);

        text_font->draw_align(pitem.text,
                        x_pos - list_pos_2/2, y_pos,
                        A_HMIDDLE, A_VMIDDLE, shadow_size);
        break;
      }
    case MN_BACK:
      {
        text_font->draw_align(pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);
        back->draw( x_pos + text_width/2  + font_width, y_pos - 8);
        break;
      }

    case MN_TOGGLE:
      {
        text_font->draw_align(pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);

        if(pitem.toggled)
          checkbox_checked->draw(
                       x_pos + (text_width+font_width)/2,
                       y_pos - 8);
        else
          checkbox->draw(
                       x_pos + (text_width+font_width)/2,
                       y_pos - 8);
        break;
      }
    case MN_ACTION:
      text_font->draw_align(pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);
      break;

    case MN_GOTO:
      text_font->draw_align(pitem.text, x_pos, y_pos, A_HMIDDLE, A_VMIDDLE, shadow_size);
      break;
    }
}

int Menu::get_width() const
{
  /* The width of the menu has to be more than the width of the text
     with the most characters */
  int menu_width = 0;
  for(unsigned int i = 0; i < item.size(); ++i)
    {
      int w = strlen(item[i].text) + (item[i].input ? strlen(item[i].input) + 1 : 0) + strlen(string_list_active(item[i].list));
      if( w > menu_width )
        {
          menu_width = w;
          if( item[i].kind == MN_TOGGLE)
            menu_width += 2;
        }
    }

  return (menu_width * 16 + 24);
}

int Menu::get_height() const
{
  return item.size() * 24;
}

/* Draw the current menu. */
void
Menu::draw()
{
  int menu_height = get_height();
  int menu_width  = get_width();

  /* Draw a transparent background */
  fillrect(pos_x - menu_width/2,
           pos_y - 24*item.size()/2 - 10,
           menu_width,menu_height + 20,
           150,180,200,125);

  for(unsigned int i = 0; i < item.size(); ++i)
    {
      draw_item(i, menu_width, menu_height);
    }
}

/* Check for menu event */
void
Menu::event(SDL_Event& event)
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

      if(item[active_item].kind == MN_CONTROLFIELD)
        {
        *item[active_item].int_p = event.key.keysym.sym;
        if(ch[0] != '\0')
          strcpy(item[active_item].input, ch);
        else
          switch(key)
           {
           case SDLK_UP:
             strcpy(item[active_item].input, "Up cursor");
             break;
           case SDLK_DOWN:
             strcpy(item[active_item].input, "Down cursor");
             break;
           case SDLK_LEFT:
             strcpy(item[active_item].input, "Left cursor");
             break;
           case SDLK_RIGHT:
             strcpy(item[active_item].input, "Right cursor");
             break;
           case SDLK_RETURN:
             strcpy(item[active_item].input, "Return");
             break;
           case SDLK_SPACE:
             strcpy(item[active_item].input, "Space");
             break;
           case SDLK_RSHIFT:
             strcpy(item[active_item].input, "Right Shift");
             break;
           case SDLK_LSHIFT:
             strcpy(item[active_item].input, "Left Shift");
             break;
           case SDLK_RCTRL:
             strcpy(item[active_item].input, "Right Control");
             break;
           case SDLK_LCTRL:
             strcpy(item[active_item].input, "Left Control");
             break;
           case SDLK_RALT:
             strcpy(item[active_item].input, "Right Alt");
             break;
           case SDLK_LALT:
             strcpy(item[active_item].input, "Left Alt");
             break;
           default:
             strcpy(item[active_item].input, "?");
             break;
           }
        
        menuaction = MENU_ACTION_DOWN;
        return;
        }


      switch(key)
        {
        case SDLK_UP:		/* Menu Up */
          menuaction = MENU_ACTION_UP;
          break;
        case SDLK_DOWN:		/* Menu Down */
          menuaction = MENU_ACTION_DOWN;
          break;
        case SDLK_LEFT:		/* Menu Up */
          menuaction = MENU_ACTION_LEFT;
          break;
        case SDLK_RIGHT:		/* Menu Down */
          menuaction = MENU_ACTION_RIGHT;
          break;
        case SDLK_SPACE:
          if(item[active_item].kind == MN_TEXTFIELD)
            {
              menuaction = MENU_ACTION_INPUT;
              mn_input_char = ' ';
              break;
            }
        case SDLK_RETURN: /* Menu Hit */
          menuaction = MENU_ACTION_HIT;
          break;
        case SDLK_DELETE:
        case SDLK_BACKSPACE:
          menuaction = MENU_ACTION_REMOVE;
          delete_character++;
          break;
        case SDLK_ESCAPE:
          Menu::pop_current();
          break;
        default:
          if( (key >= SDLK_0 && key <= SDLK_9) || (key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_SPACE && key <= SDLK_SLASH))
            {
              menuaction = MENU_ACTION_INPUT;
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
      if(event.jaxis.axis == joystick_keymap.y_axis)
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
      if(x > pos_x - get_width()/2 &&
         x < pos_x + get_width()/2 &&
         y > pos_y - get_height()/2 &&
         y < pos_y + get_height()/2)
        {
          menuaction = MENU_ACTION_HIT;
        }
      break;
    case SDL_MOUSEMOTION:
      x = event.motion.x;
      y = event.motion.y;
      if(x > pos_x - get_width()/2 &&
         x < pos_x + get_width()/2 &&
         y > pos_y - get_height()/2 &&
         y < pos_y + get_height()/2)
        {
          active_item = (y - (pos_y - get_height()/2)) / 24;
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
