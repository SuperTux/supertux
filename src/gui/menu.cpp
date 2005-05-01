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
#include <config.h>

#include <sys/types.h>
#include <ctype.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cassert>
#include <stdexcept>

#include "app/globals.h"
#include "menu.h"
#include "video/screen.h"
#include "video/drawing_context.h"
#include "app/setup.h"
#include "app/gettext.h"
#include "math/vector.h"
#include "main.h"
#include "control/joystickkeyboardcontroller.h"

static const int MENU_REPEAT_INITIAL = 400;
static const int MENU_REPEAT_RATE = 200;
static const int FLICK_CURSOR_TIME=500;

extern SDL_Surface* screen;

using namespace SuperTux;

Surface* checkbox;
Surface* checkbox_checked;
Surface* back;
Surface* arrow_left;
Surface* arrow_right;

std::vector<Menu*> Menu::last_menus;
Menu* Menu::current_ = 0;
Font* Menu::default_font;
Font* Menu::active_font;
Font* Menu::deactive_font;
Font* Menu::label_font;
Font* Menu::field_font;

/* just displays a Yes/No text that can be used to confirm stuff */
bool confirm_dialog(Surface *background, std::string text)
{
  //Surface* cap_screen = Surface::CaptureScreen();
  Menu* dialog = new Menu;
  dialog->add_deactive(-1, text);
  dialog->add_hl();
  dialog->add_entry(true, _("Yes"));
  dialog->add_entry(false, _("No"));
  dialog->add_hl();
  
  Menu::set_current(dialog);

  DrawingContext context;

  while(true)
    {
      SDL_Event event;

      if(event.type == SDL_QUIT)
        throw std::runtime_error("received window close event");
      
      while (SDL_PollEvent(&event)) {
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
  if (last_menus.size() >= 1) {
    current_ = last_menus.back();
    current_->effect.start(500);

    last_menus.pop_back();
  }
}

void
Menu::set_current(Menu* menu)
{
  last_menus.clear();

  if (menu)
    menu->effect.start(500);

  current_ = menu;
  // just to be sure...
  main_controller->reset();
}

MenuItem::MenuItem(MenuItemKind _kind, int _id)
  : kind(_kind) , id(_id)
{
  toggled = false;
  selected = false;
  target_menu = 0;
  input_flickering = false;
  input_flickering_timer.init(true);
  input_flickering_timer.start(FLICK_CURSOR_TIME);
}

void
MenuItem::change_text(const  std::string& text_)
{
  text = text_;
}

void
MenuItem::change_input(const  std::string& text_)
{
  input = text_;
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
    sprintf(str,"%s ",input.c_str());
  else
    sprintf(str,"%s_",input.c_str());

  std::string string = str;

  return string;
}

Menu::~Menu()
{
  for(std::vector<MenuItem*>::iterator i = items.begin();
      i != items.end(); ++i)
    delete *i;
}

Menu::Menu()
{
  hit_item = -1;
  menuaction = MENU_ACTION_NONE;
  delete_character = 0;
  mn_input_char = '\0';

  pos_x        = SCREEN_WIDTH/2;
  pos_y        = SCREEN_HEIGHT/2;
  arrange_left = 0;
  active_item  = -1;
  effect.init(false);

  repeat_timer.init(true);
}

void Menu::set_pos(int x, int y, float rw, float rh)
{
  pos_x = x + (int)((float)get_width() * rw);
  pos_y = y + (int)((float)get_height() * rh);
}

/* Add an item to a menu */
void
Menu::additem(MenuItem* item)
{
  items.push_back(item);

  /* If a new menu is being built, the active item shouldn't be set to
   * something that isnt selectable. Set the active_item to the first
   * selectable item added
   */
  if (active_item == -1
      && item->kind != MN_HL 
      && item->kind != MN_LABEL
      && item->kind != MN_DEACTIVE) {
    active_item = items.size() - 1;
  }
}

void
Menu::add_hl()
{
  additem(new MenuItem(MN_HL));
}

void
Menu::add_label(const std::string& text)
{
  MenuItem* item = new MenuItem(MN_LABEL);
  item->text = text;
  additem(item);
}

void
Menu::add_controlfield(int id, const std::string& text,
		const std::string& mapping)
{
  MenuItem* item = new MenuItem(MN_CONTROLFIELD, id);
  item->change_text(text);
	item->change_input(mapping);
  additem(item);
}

void
Menu::add_entry(int id, const std::string& text)
{
  MenuItem* item = new MenuItem(MN_ACTION, id);
  item->text = text;
  additem(item);
}

void
Menu::add_deactive(int id, const std::string& text)
{
  MenuItem* item = new MenuItem(MN_DEACTIVE, id);
  item->text = text;
  additem(item);
}

void
Menu::add_toggle(int id, const std::string& text, bool toogled)
{
  MenuItem* item = new MenuItem(MN_TOGGLE, id);
  item->text = text;
  item->toggled = toogled;
  additem(item);
}

void
Menu::add_back(const std::string& text)
{
  MenuItem* item = new MenuItem(MN_BACK);
  item->text = text;
  additem(item);
}

void
Menu::add_submenu(const std::string& text, Menu* submenu, int id)
{
  MenuItem* item = new MenuItem(MN_GOTO, id);
  item->text = text;
  item->target_menu = submenu;
  additem(item);
}

void
Menu::clear()
{
  for(std::vector<MenuItem*>::iterator i = items.begin();
      i != items.end(); ++i) {
    delete *i;
  }
  items.clear();
  active_item = -1;
}

/* Process actions done on the menu */
void
Menu::action()
{
  /** check main input controller... */
  if(main_controller->pressed(Controller::UP)) {
    menuaction = MENU_ACTION_UP;
    repeat_timer.start(MENU_REPEAT_INITIAL);
  }
  if(main_controller->hold(Controller::UP) && !repeat_timer.check()) {
    menuaction = MENU_ACTION_UP;
    repeat_timer.start(MENU_REPEAT_RATE);
  } 
  if(main_controller->pressed(Controller::DOWN)) {
    menuaction = MENU_ACTION_DOWN;
    repeat_timer.start(MENU_REPEAT_INITIAL);
  }
  if(main_controller->hold(Controller::DOWN) && !repeat_timer.check()) {
    menuaction = MENU_ACTION_DOWN;
    repeat_timer.start(MENU_REPEAT_RATE);
  }
  if(main_controller->pressed(Controller::JUMP)
     || main_controller->pressed(Controller::ACTION)
     || main_controller->pressed(Controller::MENU_SELECT)) {
    menuaction = MENU_ACTION_HIT;
  }
  if(main_controller->pressed(Controller::PAUSE_MENU)) {
    menuaction = MENU_ACTION_BACK;
  }

  hit_item = -1;
  if(items.size() == 0)
    return;
  
  int last_active_item = active_item;
  switch(menuaction) {
    case MENU_ACTION_UP:
      do {
        if (active_item > 0)
          --active_item;
        else
          active_item = int(items.size())-1;
      } while ((items[active_item]->kind == MN_HL 
                || items[active_item]->kind == MN_LABEL
                || items[active_item]->kind == MN_DEACTIVE)
               && (active_item != last_active_item));
      
      break;
      
    case MENU_ACTION_DOWN:
      do {
        if(active_item < int(items.size())-1 )
          ++active_item;
        else
          active_item = 0;
      } while ((items[active_item]->kind == MN_HL
                || items[active_item]->kind == MN_LABEL
                || items[active_item]->kind == MN_DEACTIVE)
               && (active_item != last_active_item));
      
      break;
      
    case MENU_ACTION_LEFT:
      if(items[active_item]->kind == MN_STRINGSELECT) {
        if(items[active_item]->selected > 0)
          items[active_item]->selected--;
        else
          items[active_item]->selected = items[active_item]->list.size()-1;
      }
      break;
      
    case MENU_ACTION_RIGHT:
      if(items[active_item]->kind == MN_STRINGSELECT) {
        if(items[active_item]->selected+1 < items[active_item]->list.size())
          items[active_item]->selected++;
        else
          items[active_item]->selected = 0;
      }
      break;
      
    case MENU_ACTION_HIT: {
      hit_item = active_item;
      switch (items[active_item]->kind) {
        case MN_GOTO:
          assert(items[active_item]->target_menu != 0);
          Menu::push_current(items[active_item]->target_menu);
          break;
          
        case MN_TOGGLE:
          items[active_item]->toggled = !items[active_item]->toggled;
          menu_action(items[active_item]);
          break;
          
        case MN_CONTROLFIELD:
          menu_action(items[active_item]);
          break;
          
        case MN_ACTION:
          menu_action(items[active_item]);
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
      break;
    }
    
    case MENU_ACTION_REMOVE:
      if(items[active_item]->kind == MN_TEXTFIELD
         || items[active_item]->kind == MN_NUMFIELD)
      {
        if(!items[active_item]->input.empty())
        {
          int i = items[active_item]->input.size();
          
          while(delete_character > 0)	/* remove charactes */
          {
            items[active_item]->input.resize(i-1);
            delete_character--;
          }
        }
      }
      break;
      
    case MENU_ACTION_INPUT:
      if(items[active_item]->kind == MN_TEXTFIELD
         || (items[active_item]->kind == MN_NUMFIELD 
             && mn_input_char >= '0' && mn_input_char <= '9'))
      {
        items[active_item]->input.push_back(mn_input_char);
      }
      break;
      
    case MENU_ACTION_BACK:
      Menu::pop_current();
      break;

    case MENU_ACTION_NONE:
      break;
  }
  menuaction = MENU_ACTION_NONE;

  assert(active_item < int(items.size()));
}

int
Menu::check()
{
  if (hit_item != -1)
    return items[hit_item]->id;
  else
    return -1;
}

void
Menu::menu_action(MenuItem* )
{}

void
Menu::draw_item(DrawingContext& context, int index)
{
  int menu_height = get_height();
  int menu_width = get_width();  

  MenuItem& pitem = *(items[index]);

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
  int list_width = 0;
  if(pitem.list.size() > 0) {
    list_width = (int) text_font->get_text_width(pitem.list[pitem.selected]);
  }
  
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
        context.draw_text(deactive_font, pitem.text,
                          Vector(SCREEN_WIDTH/2, y_pos - int(deactive_font->get_height()/2)),
                          CENTER_ALLIGN, LAYER_GUI);
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
        context.draw_text(label_font, pitem.text,
                          Vector(SCREEN_WIDTH/2, y_pos - int(label_font->get_height()/2)),
                          CENTER_ALLIGN, LAYER_GUI);
        break;
      }
    case MN_TEXTFIELD:
    case MN_NUMFIELD:
    case MN_CONTROLFIELD:
      {
        int width = text_width + input_width + 5;
        int text_pos = SCREEN_WIDTH/2 - width/2;
        int input_pos = text_pos + text_width + 10;

        context.draw_filled_rect(
          Vector(input_pos - 5, y_pos - 10),
          Vector(input_width + 10, 20),
          Color(255,255,255,255), LAYER_GUI-5);
        context.draw_filled_rect(
          Vector(input_pos - 4, y_pos - 9),
          Vector(input_width + 8, 18),
          Color(0,0,0,128), LAYER_GUI-4);

        if(pitem.kind == MN_TEXTFIELD || pitem.kind == MN_NUMFIELD)
          {
            if(active_item == index)
              context.draw_text(field_font,
                                pitem.get_input_with_symbol(true),
                                Vector(input_pos, y_pos - int(field_font->get_height()/2)),
                                LEFT_ALLIGN, LAYER_GUI);
            else
              context.draw_text(field_font,
                                pitem.get_input_with_symbol(false),
                                Vector(input_pos, y_pos - int(field_font->get_height()/2)),
                                LEFT_ALLIGN, LAYER_GUI);
          }
        else
          context.draw_text(field_font, pitem.input,
                            Vector(input_pos, y_pos - int(field_font->get_height()/2)),
                            LEFT_ALLIGN, LAYER_GUI);

        context.draw_text(text_font, pitem.text,
                          Vector(text_pos, y_pos - int(text_font->get_height()/2)),
                          LEFT_ALLIGN, LAYER_GUI);
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

        context.draw_text(text_font, pitem.list[pitem.selected],
                                 Vector(SCREEN_WIDTH/2 + text_pos, y_pos - int(text_font->get_height()/2)),
                                 CENTER_ALLIGN, LAYER_GUI);
        context.draw_text(text_font, pitem.text,
                                 Vector(SCREEN_WIDTH/2  + list_pos_2/2, y_pos - int(text_font->get_height()/2)),
                                 CENTER_ALLIGN, LAYER_GUI);
        break;
      }
    case MN_BACK:
      {
        context.draw_text(text_font, pitem.text,
                          Vector(SCREEN_WIDTH/2, y_pos - int(text_font->get_height()/2)),
                          CENTER_ALLIGN, LAYER_GUI);
        context.draw_surface(back,
                             Vector(x_pos + text_width/2  + 16, y_pos - 8),
                             LAYER_GUI);
        break;
      }

    case MN_TOGGLE:
      {
        context.draw_text(text_font, pitem.text,
                          Vector(SCREEN_WIDTH/2, y_pos - (text_font->get_height()/2)),
                          CENTER_ALLIGN, LAYER_GUI);

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
      context.draw_text(text_font, pitem.text,
                        Vector(SCREEN_WIDTH/2, y_pos - int(text_font->get_height()/2)),
                        CENTER_ALLIGN, LAYER_GUI);
      break;

    case MN_GOTO:
      context.draw_text(text_font, pitem.text,
                        Vector(SCREEN_WIDTH/2, y_pos - int(text_font->get_height()/2)),
                        CENTER_ALLIGN, LAYER_GUI);
      break;
    }
}

int Menu::get_width() const
  {
    /* The width of the menu has to be more than the width of the text
       with the most characters */
    int menu_width = 0;
    for(unsigned int i = 0; i < items.size(); ++i)
      {
        int w = items[i]->text.size() + items[i]->input.size() + 1;
        if(w > menu_width)
          {
            menu_width = w;
            if( items[i]->kind == MN_TOGGLE)
              menu_width += 2;
          }
      }

    return (menu_width * 16 + 24);
  }

int Menu::get_height() const
  {
    return items.size() * 24;
  }

/* Draw the current menu. */
void
Menu::draw(DrawingContext& context)
{
  int menu_height = get_height();
  int menu_width = get_width();  

  /* Draw a transparent background */
  context.draw_filled_rect(
    Vector(pos_x - menu_width/2, pos_y - 24*items.size()/2 - 10),
    Vector(menu_width,menu_height + 20),
    Color(150,180,200,125), LAYER_GUI-10);

  for(unsigned int i = 0; i < items.size(); ++i)
    {
      draw_item(context, i);
    }
}

MenuItem&
Menu::get_item_by_id(int id)
{
  for(std::vector<MenuItem*>::iterator i = items.begin();
      i != items.end(); ++i) {
    MenuItem& item = **i;
    
    if(item.id == id)
      return item;
  }

  throw std::runtime_error("MenuItem not found");
}

const MenuItem&
Menu::get_item_by_id(int id) const
{
  for(std::vector<MenuItem*>::const_iterator i = items.begin();
      i != items.end(); ++i) {
    const MenuItem& item = **i;
    
    if(item.id == id)
      return item;
  }

  throw std::runtime_error("MenuItem not found");
}

int Menu::get_active_item_id()
{
  return items[active_item]->id;
}

bool
Menu::is_toggled(int id) const
{
  return get_item_by_id(id).toggled;
}

/* Check for menu event */
void
Menu::event(const SDL_Event& event)
{
  if(effect.started())
    return;

  switch(event.type) {
    case SDL_MOUSEBUTTONDOWN:
      {
        int x = int(event.motion.x * float(SCREEN_WIDTH)/screen->w);
        int y = int(event.motion.y * float(SCREEN_HEIGHT)/screen->h);

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
        int x = int(event.motion.x * float(SCREEN_WIDTH)/screen->w);
        int y = int(event.motion.y * float(SCREEN_HEIGHT)/screen->h);

        if(x > pos_x - get_width()/2 &&
            x < pos_x + get_width()/2 &&
            y > pos_y - get_height()/2 &&
            y < pos_y + get_height()/2)
          {
            int new_active_item = (y - (pos_y - get_height()/2)) / 24;
          
            /* only change the mouse focus to a selectable item */
            if ((items[new_active_item]->kind != MN_HL)
                && (items[new_active_item]->kind != MN_LABEL)
                && (items[new_active_item]->kind != MN_DEACTIVE))
              active_item = new_active_item;
            
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

void
Menu::set_active_item(int id)
{
	for(int i = 0; i < items.size(); ++i) {
		MenuItem* item = items[i];
		if(item->id == id) {
			active_item = i;
			break;
		}
	}
}

