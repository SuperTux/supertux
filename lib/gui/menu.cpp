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
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cassert>

#include "../app/globals.h"
#include "../gui/menu.h"
#include "../video/screen.h"
#include "../video/drawing_context.h"
#include "../app/setup.h"
#include "../special/timer.h"
#include "../app/gettext.h"
#include "../math/vector.h"

using namespace SuperTux;

#define FLICK_CURSOR_TIME 500

Surface* SuperTux::checkbox;
Surface* SuperTux::checkbox_checked;
Surface* SuperTux::back;
Surface* SuperTux::arrow_left;
Surface* SuperTux::arrow_right;

std::vector<Menu*> Menu::last_menus;
Menu* Menu::current_ = 0;
Font* Menu::default_font;
Font* Menu::active_font;
Font* Menu::deactive_font;
Font* Menu::label_font;
Font* Menu::field_font;

/* just displays a Yes/No text that can be used to confirm stuff */
bool SuperTux::confirm_dialog(Surface *background, std::string text)
{
  //Surface* cap_screen = Surface::CaptureScreen();

  Menu* dialog = new Menu;
  dialog->additem(MN_DEACTIVE, text,0,0);
  dialog->additem(MN_HL,"",0,0);
  dialog->additem(MN_ACTION,_("Yes"),0,0,true);
  dialog->additem(MN_ACTION,_("No"),0,0,false);
  dialog->additem(MN_HL,"",0,0);

  Menu::set_current(dialog);

  DrawingContext context;

  while(true)
    {
      SDL_Event event;

      while (SDL_PollEvent(&event))
        {
          dialog->event(event);
        }

      if(background == NULL)
        context.draw_gradient(Color(200,240,220), Color(200,200,220), LAYER_BACKGROUND0);
      else
        context.draw_surface(background, Vector(0,0), LAYER_BACKGROUND0);

      dialog->draw(context);
      dialog->action();

      switch (dialog->check())
        {
        case true:
          //delete cap_screen;
          Menu::set_current(0);
          delete dialog;
          return true;
          break;
        case false:
          //delete cap_screen;
          Menu::set_current(0);
          delete dialog;
          return false;
          break;
        default:
          break;
        }

      mouse_cursor->draw(context);
      context.do_drawing();
      SDL_Delay(25);
    }

  return false;
}

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
MenuItem::create(MenuItemKind kind_, const char *text_, int init_toggle_, Menu* target_menu_, int id, int* int_p_)
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

  pnew_item->id = id;
  pnew_item->int_p = int_p_;

  pnew_item->input_flickering = false;
  pnew_item->input_flickering_timer.init(true);
  pnew_item->input_flickering_timer.start(FLICK_CURSOR_TIME);

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

std::string MenuItem::get_input_with_symbol(bool active_item)
{
  if(!active_item)
    input_flickering = true;
  else
    {
      if(input_flickering_timer.get_left() < 0)
        {
          if(input_flickering)
            input_flickering = false;
          else
            input_flickering = true;
          input_flickering_timer.start(FLICK_CURSOR_TIME);
        }
    }

  char str[1024];
  if(input_flickering)
    sprintf(str,"%s ",input);
  else
    sprintf(str,"%s_",input);

  std::string string = str;

  return string;
}

/* Set ControlField for keyboard key */
void Menu::get_controlfield_key_into_input(MenuItem *item)
{
  switch(*item->int_p)
    {
    case SDLK_UP:
      item->change_input(_("Up cursor"));
      break;
    case SDLK_DOWN:
      item->change_input(_("Down cursor"));
      break;
    case SDLK_LEFT:
      item->change_input(_("Left cursor"));
      break;
    case SDLK_RIGHT:
      item->change_input(_("Right cursor"));
      break;
    case SDLK_RETURN:
      item->change_input(_("Return"));
      break;
    case SDLK_SPACE:
      item->change_input(_("Space"));
      break;
    case SDLK_RSHIFT:
      item->change_input(_("Right Shift"));
      break;
    case SDLK_LSHIFT:
      item->change_input(_("Left Shift"));
      break;
    case SDLK_RCTRL:
      item->change_input(_("Right Control"));
      break;
    case SDLK_LCTRL:
      item->change_input(_("Left Control"));
      break;
    case SDLK_RALT:
      item->change_input(_("Right Alt"));
      break;
    case SDLK_LALT:
      item->change_input(_("Left Alt"));
      break;
    default:
      {
        char tmp[64];
        snprintf(tmp, 64, "%d", *item->int_p);
        item->change_input(tmp);
      }
      break;
    }
}

/* Set ControlField for joystick button */
void Menu::get_controlfield_js_into_input(MenuItem *item)
{
  std::ostringstream oss;
  oss << "Button " << *item->int_p;
  item->change_input(oss.str().c_str());
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
  arrange_left = 0;
  active_item  = 0;
  effect.init(false);

  joystick_timer.init(true);
}

void Menu::set_pos(int x, int y, float rw, float rh)
{
  pos_x = x + (int)((float)get_width() * rw);
  pos_y = y + (int)((float)get_height() * rh);
}

void
Menu::additem(MenuItemKind kind_, const std::string& text_, int toggle_, Menu* menu_, int id, int* int_p)
{
  additem(MenuItem::create(kind_, text_.c_str(), toggle_, menu_, id, int_p));
}

/* Add an item to a menu */
void
Menu::additem(MenuItem* pmenu_item)
{
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
                Menu::set_current(0);
                item[active_item].toggled = true;
                break;
              case MN_TEXTFIELD:
              case MN_NUMFIELD:
                menuaction = MENU_ACTION_DOWN;
                action();
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

  if (active_item >= int(item.size()))
    active_item = int(item.size()) - 1;
}

int
Menu::check()
{
  if (hit_item != -1)
    return item[hit_item].id;
  else
    return -1;
}

void
Menu::draw_item(DrawingContext& context,
                int index, // Position of the current item in the menu
                int menu_width, int menu_height)
{
  MenuItem& pitem = item[index];

  int effect_offset = 0;
  {
    int effect_time = 0;

    if(effect.check())
      effect_time = effect.get_left() / 4;

    effect_offset = (index % 2) ? effect_time : -effect_time;
  }

  Font* text_font = default_font;
  int x_pos       = pos_x;
  int y_pos       = pos_y + 24*index - menu_height/2 + 12 + effect_offset;
  int shadow_size = 2;
  int text_width  = int(text_font->get_text_width(pitem.text));
  int input_width = int(text_font->get_text_width(pitem.input) + 10);
  int list_width  =
    int(text_font->get_text_width(string_list_active(pitem.list)));

  if (arrange_left)
    x_pos += 24 - menu_width/2 + (text_width + input_width + list_width)/2;

  if(index == active_item)
    {
      shadow_size = 3;
      text_font = active_font;
    }

  switch (pitem.kind)
    {
    case MN_DEACTIVE:
      {
        context.draw_text_center(deactive_font, pitem.text,
                                 Vector(0, y_pos - int(deactive_font->get_height()/2)),
                                 LAYER_GUI);
        break;
      }

    case MN_HL:
      {
        // TODO
        int x = pos_x - menu_width/2;
        int y = y_pos - 12 - effect_offset;
        /* Draw a horizontal line with a little 3d effect */
        context.draw_filled_rect(Vector(x, y + 6),
                                 Vector(menu_width, 4), Color(150,200,255,225), LAYER_GUI);
        context.draw_filled_rect(Vector(x, y + 6),
                                 Vector(menu_width, 2), Color(255,255,255,255), LAYER_GUI);
        break;
      }
    case MN_LABEL:
      {
        context.draw_text_center(label_font,
                                 pitem.text, Vector(0, y_pos - int(label_font->get_height()/2)),
                                 LAYER_GUI);
        break;
      }
    case MN_TEXTFIELD:
    case MN_NUMFIELD:
    case MN_CONTROLFIELD_KB:
    case MN_CONTROLFIELD_JS:
      {
        int width = text_width + input_width + 5;
        int text_pos = screen->w/2 - width/2;
        int input_pos = text_pos + text_width + 10;

        context.draw_filled_rect(
          Vector(input_pos - 5, y_pos - 10),
          Vector(input_width + 10, 20),
          Color(255,255,255,255), LAYER_GUI-5);
        context.draw_filled_rect(
          Vector(input_pos - 4, y_pos - 9),
          Vector(input_width + 8, 18),
          Color(0,0,0,128), LAYER_GUI-4);

        if(pitem.kind == MN_CONTROLFIELD_KB)
          get_controlfield_key_into_input(&pitem);
        else if (pitem.kind == MN_CONTROLFIELD_JS)
          get_controlfield_js_into_input(&pitem);

        if(pitem.kind == MN_TEXTFIELD || pitem.kind == MN_NUMFIELD)
          {
            if(active_item == index)
              context.draw_text(field_font,
                                pitem.get_input_with_symbol(true),
                                Vector(input_pos, y_pos - int(field_font->get_height()/2)),
                                LAYER_GUI);
            else
              context.draw_text(field_font,
                                pitem.get_input_with_symbol(false),
                                Vector(input_pos, y_pos - int(field_font->get_height()/2)),
                                LAYER_GUI);
          }
        else
          context.draw_text(field_font, pitem.input,
                            Vector(input_pos, y_pos - int(field_font->get_height()/2)),
                            LAYER_GUI);

        context.draw_text(text_font, pitem.text,
                          Vector(text_pos, y_pos - int(text_font->get_height()/2)),
                          LAYER_GUI);
        break;
      }
    case MN_STRINGSELECT:
      {
        int list_pos_2 = list_width + 16;
        int list_pos   = list_width/2;
        int text_pos   = (text_width + 16)/2;

        /* Draw arrows */
        context.draw_surface(arrow_left,
                             Vector(x_pos - list_pos + text_pos - 17, y_pos - 8),
                             LAYER_GUI);
        context.draw_surface(arrow_right,
                             Vector(x_pos - list_pos + text_pos - 1 + list_pos_2, y_pos - 8),
                             LAYER_GUI);

        /* Draw input background */
        context.draw_filled_rect(
          Vector(x_pos - list_pos + text_pos - 1, y_pos - 10),
          Vector(list_pos_2 + 2, 20),
          Color(255,255,255,255), LAYER_GUI - 4);
        context.draw_filled_rect(
          Vector(x_pos - list_pos + text_pos, y_pos - 9),
          Vector(list_pos_2, 18),
          Color(0,0,0,128), LAYER_GUI - 5);

        context.draw_text_center(text_font, string_list_active(pitem.list),
                                 Vector(text_pos, y_pos - int(text_font->get_height()/2)),
                                 LAYER_GUI);
        context.draw_text_center(text_font, pitem.text,
                                 Vector(list_pos_2/2, y_pos - int(text_font->get_height()/2)),
                                 LAYER_GUI);
        break;
      }
    case MN_BACK:
      {
        context.draw_text_center(text_font, pitem.text,
                                 Vector(0, y_pos - int(text_font->get_height()/2)),
                                 LAYER_GUI);
        context.draw_surface(back,
                             Vector(x_pos + text_width/2  + 16, y_pos - 8),
                             LAYER_GUI);
        break;
      }

    case MN_TOGGLE:
      {
        context.draw_text_center(text_font, pitem.text,
                                 Vector(0, y_pos - (text_font->get_height()/2)),
                                 LAYER_GUI);

        if(pitem.toggled)
          context.draw_surface(checkbox_checked,
                               Vector(x_pos + (text_width+16)/2, y_pos - 8),
                               LAYER_GUI + 1);
        else
          context.draw_surface(checkbox,
                               Vector(x_pos + (text_width+16)/2, y_pos - 8),
                               LAYER_GUI + 1);
        break;
      }
    case MN_ACTION:
      context.draw_text_center(text_font, pitem.text,
                               Vector(0, y_pos - int(text_font->get_height()/2)),
                               LAYER_GUI);
      break;

    case MN_GOTO:
      context.draw_text_center(text_font, pitem.text,
                               Vector(0, y_pos - int(text_font->get_height()/2)),
                               LAYER_GUI);
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
Menu::draw(DrawingContext& context)
{
  int menu_height = get_height();
  int menu_width  = get_width();

  /* Draw a transparent background */
  context.draw_filled_rect(
    Vector(pos_x - menu_width/2, pos_y - 24*item.size()/2 - 10),
    Vector(menu_width,menu_height + 20),
    Color(150,180,200,125), LAYER_GUI-10);

  for(unsigned int i = 0; i < item.size(); ++i)
    {
      draw_item(context, i, menu_width, menu_height);
    }
}

MenuItem&
Menu::get_item_by_id(int id)
{
  for(std::vector<MenuItem>::iterator i = item.begin(); i != item.end(); ++i)
    {
      if(i->id == id)
        return *i;
    }

  assert(false);
  static MenuItem dummyitem;
  return dummyitem;
}

int Menu::get_active_item_id()
{
  return item[active_item].id;
}

bool
Menu::isToggled(int id)
{
  return get_item_by_id(id).toggled;
}

/* Check for menu event */
void
Menu::event(SDL_Event& event)
{
  switch(event.type)
    {
    case SDL_KEYDOWN:
      {
        SDLKey key = key = event.key.keysym.sym;
        SDLMod keymod;
        char ch[2];
        keymod = SDL_GetModState();

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

        if(item[active_item].kind == MN_CONTROLFIELD_KB)
          {
            if(key == SDLK_ESCAPE)
              {
                Menu::pop_current();
                return;
              }
            *item[active_item].int_p = key;
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
      }
      break;

    case  SDL_JOYAXISMOTION:
      if(event.jaxis.axis == joystick_keymap.y_axis)
        {
          if (event.jaxis.value > joystick_keymap.dead_zone && !joystick_timer.started())
            {
              menuaction = MENU_ACTION_DOWN;
              joystick_timer.start(JOYSTICK_MENU_DELAY);
            }
          else if (event.jaxis.value < -joystick_keymap.dead_zone && !joystick_timer.started())
            {
              menuaction = MENU_ACTION_UP;
              joystick_timer.start(JOYSTICK_MENU_DELAY);
            }
          else
            joystick_timer.stop();
        }
      break;
    case  SDL_JOYBUTTONDOWN:
      if (item[active_item].kind == MN_CONTROLFIELD_JS)
        {
          // FIXME: This next line does nothing useable, right?
          // *item[active_item].int_p = key;
          menuaction = MENU_ACTION_DOWN;
        }
      menuaction = MENU_ACTION_HIT;
      break;

    case SDL_MOUSEBUTTONDOWN:
      {
        int x = event.motion.x;
        int y = event.motion.y;

        if(x > pos_x - get_width()/2 &&
            x < pos_x + get_width()/2 &&
            y > pos_y - get_height()/2 &&
            y < pos_y + get_height()/2)
          {
            menuaction = MENU_ACTION_HIT;
          }
      }
      break;

    case SDL_MOUSEMOTION:
      {
        int x = event.motion.x;
        int y = event.motion.y;

        if(x > pos_x - get_width()/2 &&
            x < pos_x + get_width()/2 &&
            y > pos_y - get_height()/2 &&
            y < pos_y + get_height()/2)
          {
            active_item = (y - (pos_y - get_height()/2)) / 24;
	    if(MouseCursor::current())
            MouseCursor::current()->set_state(MC_LINK);
          }
        else
          {
	    if(MouseCursor::current())
            MouseCursor::current()->set_state(MC_NORMAL);
          }
      }
      break;

    default:
      break;
    }
}


// EOF //
