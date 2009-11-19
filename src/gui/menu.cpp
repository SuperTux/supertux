//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "gui/menu.hpp"

#include <math.h>

#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "supertux/globals.hpp"
#include "supertux/mainloop.hpp"
#include "supertux/resources.hpp"
#include "supertux/timer.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"

static const float MENU_REPEAT_INITIAL = 0.4f;
static const float MENU_REPEAT_RATE    = 0.1f;

extern SDL_Surface* g_screen;

Menu::Menu() :
  hit_item(),
  pos_x(),
  pos_y(),
  menuaction(),
  delete_character(),
  mn_input_char(),
  menu_repeat_time(),
  close(false),
  items(),
  effect_progress(),
  effect_start_time(),
  arrange_left(),
  active_item(),
  checkbox(),
  checkbox_checked(),
  back(),
  arrow_left(),
  arrow_right()
{
  MenuManager2::all_menus.push_back(this);

  hit_item = -1;
  menuaction = MENU_ACTION_NONE;
  delete_character = 0;
  mn_input_char = '\0';

  pos_x        = SCREEN_WIDTH/2;
  pos_y        = SCREEN_HEIGHT/2;
  arrange_left = 0;
  active_item  = -1;

  effect_progress   = 0.0f;
  effect_start_time = 0.0f;

  checkbox.reset(new Surface("images/engine/menu/checkbox-unchecked.png"));
  checkbox_checked.reset(new Surface("images/engine/menu/checkbox-checked.png"));
  back.reset(new Surface("images/engine/menu/arrow-back.png"));
  arrow_left.reset(new Surface("images/engine/menu/arrow-left.png"));
  arrow_right.reset(new Surface("images/engine/menu/arrow-right.png"));
}

Menu::~Menu()
{
  MenuManager2::all_menus.remove(this);

  for(std::vector<MenuItem*>::iterator i = items.begin();
      i != items.end(); ++i) 
  {
    delete *i;
  }

  if (MenuManager2::current_ == this)
    MenuManager2::current_ = NULL;

  if (MenuManager2::previous == this)
    MenuManager2::previous = NULL;
}

void
Menu::set_pos(float x, float y, float rw, float rh)
{
  pos_x = x + get_width()  * rw;
  pos_y = y + get_height() * rh;
}

/* Add an item to a menu */
void
Menu::additem(MenuItem* item)
{
  items.push_back(item);

  /* If a new menu is being built, the active item shouldn't be set to
   * something that isn't selectable. Set the active_item to the first
   * selectable item added.
   */
  if (active_item == -1
      && item->kind != MN_HL
      && item->kind != MN_LABEL
      && item->kind != MN_INACTIVE) {
    active_item = items.size() - 1;
  }
}

MenuItem*
Menu::add_hl()
{
  MenuItem* item = new MenuItem(MN_HL);
  additem(item);
  return item;
}

MenuItem*
Menu::add_label(const std::string& text)
{
  MenuItem* item = new MenuItem(MN_LABEL);
  item->text = text;
  additem(item);
  return item;
}

MenuItem*
Menu::add_controlfield(int id, const std::string& text,
                       const std::string& mapping)
{
  MenuItem* item = new MenuItem(MN_CONTROLFIELD, id);
  item->change_text(text);
  item->change_input(mapping);
  additem(item);
  return item;
}

MenuItem*
Menu::add_entry(int id, const std::string& text)
{
  MenuItem* item = new MenuItem(MN_ACTION, id);
  item->text = text;
  additem(item);
  return item;
}

MenuItem*
Menu::add_inactive(int id, const std::string& text)
{
  MenuItem* item = new MenuItem(MN_INACTIVE, id);
  item->text = text;
  additem(item);
  return item;
}

MenuItem*
Menu::add_toggle(int id, const std::string& text, bool toogled)
{
  MenuItem* item = new MenuItem(MN_TOGGLE, id);
  item->text = text;
  item->toggled = toogled;
  additem(item);
  return item;
}

MenuItem*
Menu::add_string_select(int id, const std::string& text)
{
  MenuItem* item = new MenuItem(MN_STRINGSELECT, id);
  item->text = text;
  additem(item);
  return item;
}

MenuItem*
Menu::add_back(const std::string& text)
{
  MenuItem* item = new MenuItem(MN_BACK);
  item->text = text;
  additem(item);
  return item;
}

MenuItem*
Menu::add_submenu(const std::string& text, Menu* submenu, int id)
{
  MenuItem* item = new MenuItem(MN_GOTO, id);
  item->text = text;
  item->target_menu = submenu;
  additem(item);
  return item;
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
Menu::update()
{
  int menu_height = (int) get_height();
  if (menu_height > SCREEN_HEIGHT)
  { // Scrolling
    int scroll_offset = (menu_height - SCREEN_HEIGHT) / 2 + 32;
    pos_y = SCREEN_HEIGHT/2 - scroll_offset * ((float(active_item) / (items.size()-1)) - 0.5f) * 2.0f;
  }

  effect_progress = (real_time - effect_start_time) * 6.0f;

  if(effect_progress >= 1.0f) {
    effect_progress = 1.0f;

    if (close) {
      MenuManager2::current_ = 0;
      close = false;
    }
  }
  else if (effect_progress <= 0.0f) {
    effect_progress = 0.0f;
  }

  /** check main input controller... */
  if(g_main_controller->pressed(Controller::UP)) {
    menuaction = MENU_ACTION_UP;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(g_main_controller->hold(Controller::UP) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_UP;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(g_main_controller->pressed(Controller::DOWN)) {
    menuaction = MENU_ACTION_DOWN;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(g_main_controller->hold(Controller::DOWN) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_DOWN;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(g_main_controller->pressed(Controller::LEFT)) {
    menuaction = MENU_ACTION_LEFT;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(g_main_controller->hold(Controller::LEFT) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_LEFT;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(g_main_controller->pressed(Controller::RIGHT)) {
    menuaction = MENU_ACTION_RIGHT;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(g_main_controller->hold(Controller::RIGHT) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_RIGHT;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(g_main_controller->pressed(Controller::ACTION)
     || g_main_controller->pressed(Controller::MENU_SELECT)) {
    menuaction = MENU_ACTION_HIT;
  }
  if(g_main_controller->pressed(Controller::PAUSE_MENU)) {
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
                || items[active_item]->kind == MN_INACTIVE)
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
                || items[active_item]->kind == MN_INACTIVE)
               && (active_item != last_active_item));

      break;

    case MENU_ACTION_LEFT:
      if(items[active_item]->kind == MN_STRINGSELECT) {
        if(items[active_item]->selected > 0)
          items[active_item]->selected--;
        else
          items[active_item]->selected = items[active_item]->list.size()-1;
        
        menu_action(items[active_item]);
      }
      break;

    case MENU_ACTION_RIGHT:
      if(items[active_item]->kind == MN_STRINGSELECT) {
        if(items[active_item]->selected+1 < items[active_item]->list.size())
          items[active_item]->selected++;
        else
          items[active_item]->selected = 0;
        
        menu_action(items[active_item]);
      }
      break;

    case MENU_ACTION_HIT: {
      hit_item = active_item;
      switch (items[active_item]->kind) {
        case MN_GOTO:
          assert(items[active_item]->target_menu != 0);
          MenuManager2::push_current(items[active_item]->target_menu);
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

        case MN_STRINGSELECT:
          if(items[active_item]->selected+1 < items[active_item]->list.size())
            items[active_item]->selected++;
          else
            items[active_item]->selected = 0;

          menu_action(items[active_item]);
          break;

        case MN_TEXTFIELD:
        case MN_NUMFIELD:
          menuaction = MENU_ACTION_DOWN;
          update();
          break;

        case MN_BACK:
          MenuManager2::pop_current();
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

          while(delete_character > 0)        /* remove characters */
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
      MenuManager2::pop_current();
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
  if (hit_item != -1) {
    int id = items[hit_item]->id;
    hit_item = -1;                      //Clear event when checked out.. (we would end up in a loop when we try to leave "fake" submenu like Addons or Contrib)
    return id;
  }
  else
    return -1;
}

void
Menu::menu_action(MenuItem* )
{}

void
Menu::draw_item(DrawingContext& context, int index)
{
  float menu_height = get_height();
  float menu_width  = get_width();

  MenuItem& pitem = *(items[index]);

  Color text_color = default_color;
  float x_pos       = pos_x;
  float y_pos       = pos_y + 24*index - menu_height/2 + 12;
  int shadow_size = 2;
  int text_width  = int(normal_font->get_text_width(pitem.text));
  int input_width = int(normal_font->get_text_width(pitem.input) + 10);
  int list_width = 0;

  float left  = pos_x - menu_width/2 + 16;
  float right = pos_x + menu_width/2 - 16;

  if(pitem.list.size() > 0) {
    list_width = (int) normal_font->get_text_width(pitem.list[pitem.selected]);
  }

  if (arrange_left)
    x_pos += 24 - menu_width/2 + (text_width + input_width + list_width)/2;

  if(index == active_item)
  {
    shadow_size = 3;
    text_color = active_color;
  }

  if(active_item == index)
  {
    float blink = (sinf(real_time * M_PI * 1.0f)/2.0f + 0.5f) * 0.5f + 0.25f;
    context.draw_filled_rect(Rect(Vector(pos_x - menu_width/2 + 10 - 2, y_pos - 12 - 2),
                                  Vector(pos_x + menu_width/2 - 10 + 2, y_pos + 12 + 2)),
                             Color(1.0f, 1.0f, 1.0f, blink),
                             14.0f,
                             LAYER_GUI-10);
    context.draw_filled_rect(Rect(Vector(pos_x - menu_width/2 + 10, y_pos - 12),
                                  Vector(pos_x + menu_width/2 - 10, y_pos + 12)),
                             Color(1.0f, 1.0f, 1.0f, 0.5f),
                             12.0f,
                             LAYER_GUI-10);
  }

  switch (pitem.kind)
  {
    case MN_INACTIVE:
    {
      context.draw_text(normal_font, pitem.text,
                        Vector(pos_x, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, inactive_color);
      break;
    }

    case MN_HL:
    {
      // TODO
      float x = pos_x - menu_width/2;
      float y = y_pos - 12;
      /* Draw a horizontal line with a little 3d effect */
      context.draw_filled_rect(Vector(x, y + 6),
                               Vector(menu_width, 4),
                               Color(0.6f, 0.7f, 1.0f, 1.0f), LAYER_GUI);
      context.draw_filled_rect(Vector(x, y + 6),
                               Vector(menu_width, 2),
                               Color(1.0f, 1.0f, 1.0f, 1.0f), LAYER_GUI);
      break;
    }
    case MN_LABEL:
    {
      context.draw_text(big_font, pitem.text,
                        Vector(pos_x, y_pos - int(big_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, label_color);
      break;
    }
    case MN_TEXTFIELD:
    case MN_NUMFIELD:
    case MN_CONTROLFIELD:
    {
      if(pitem.kind == MN_TEXTFIELD || pitem.kind == MN_NUMFIELD)
      {
        if(active_item == index)
          context.draw_text(normal_font,
                            pitem.get_input_with_symbol(true),
                            Vector(right, y_pos - int(normal_font->get_height()/2)),
                            ALIGN_RIGHT, LAYER_GUI, field_color);
        else
          context.draw_text(normal_font,
                            pitem.get_input_with_symbol(false),
                            Vector(right, y_pos - int(normal_font->get_height()/2)),
                            ALIGN_RIGHT, LAYER_GUI, field_color);
      }
      else
        context.draw_text(normal_font, pitem.input,
                          Vector(right, y_pos - int(normal_font->get_height()/2)),
                          ALIGN_RIGHT, LAYER_GUI, field_color);

      context.draw_text(normal_font, pitem.text,
                        Vector(left, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_LEFT, LAYER_GUI, text_color);
      break;
    }
    case MN_STRINGSELECT:
    {
      float roff = arrow_left->get_width();
      // Draw left side
      context.draw_text(normal_font, pitem.text,
                        Vector(left, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_LEFT, LAYER_GUI, text_color);

      // Draw right side
      context.draw_surface(arrow_left.get(),
                           Vector(right - list_width - roff - roff, y_pos - 8),
                           LAYER_GUI);
      context.draw_surface(arrow_right.get(),
                           Vector(right - roff, y_pos - 8),
                           LAYER_GUI);
      context.draw_text(normal_font, pitem.list[pitem.selected],
                        Vector(right - roff, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_RIGHT, LAYER_GUI, text_color);
      break;
    }
    case MN_BACK:
    {
      context.draw_text(normal_font, pitem.text,
                        Vector(pos_x, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, text_color);
      context.draw_surface(back.get(),
                           Vector(x_pos + text_width/2  + 16, y_pos - 8),
                           LAYER_GUI);
      break;
    }

    case MN_TOGGLE:
    {
      context.draw_text(normal_font, pitem.text,
                        Vector(pos_x - menu_width/2 + 16, y_pos - (normal_font->get_height()/2)),
                        ALIGN_LEFT, LAYER_GUI, text_color);

      if(pitem.toggled)
        context.draw_surface(checkbox_checked.get(),
                             Vector(x_pos + (menu_width/2-16) - checkbox->get_width(), y_pos - 8),
                             LAYER_GUI + 1);
      else
        context.draw_surface(checkbox.get(),
                             Vector(x_pos + (menu_width/2-16) - checkbox->get_width(), y_pos - 8),
                             LAYER_GUI + 1);
      break;
    }
    case MN_ACTION:
      context.draw_text(normal_font, pitem.text,
                        Vector(pos_x, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, text_color);
      break;

    case MN_GOTO:
      context.draw_text(normal_font, pitem.text,
                        Vector(pos_x, y_pos - int(normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, text_color);
      break;
  }
}

float
Menu::get_width() const
{
  /* The width of the menu has to be more than the width of the text
     with the most characters */
  float menu_width = 0;
  for(unsigned int i = 0; i < items.size(); ++i)
  {
    Font* font = normal_font;
    if(items[i]->kind == MN_LABEL)
      font = big_font;

    float w = font->get_text_width(items[i]->text) +
      big_font->get_text_width(items[i]->input) + 16;
    if(items[i]->kind == MN_TOGGLE)
      w += 32;

    if(w > menu_width)
      menu_width = w;
  }

  return menu_width + 24;
}

float
Menu::get_height() const
{
  return items.size() * 24;
}

/* Draw the current menu. */
void
Menu::draw(DrawingContext& context)
{
  if(MouseCursor::current()) {
    MouseCursor::current()->draw(context);
  }

  float menu_width  = get_width();
  float menu_height = get_height();

  if (effect_progress != 1.0f)
  {
    if (close)
    {
      menu_width  = (MenuManager2::current_->get_width()  * (1.0f - effect_progress));
      menu_height = (MenuManager2::current_->get_height() * (1.0f - effect_progress));
    }
    else if (MenuManager2::previous)
    {
      menu_width  = (menu_width  * effect_progress) + (MenuManager2::previous->get_width()  * (1.0f - effect_progress));
      menu_height = (menu_height * effect_progress) + (MenuManager2::previous->get_height() * (1.0f - effect_progress));
      //std::cout << effect_progress << " " << this << " " << last_menus.back() << std::endl;
    }
    else
    {
      menu_width  *= effect_progress;
      menu_height *= effect_progress;
    }
  }

  /* Draw a transparent background */
  context.draw_filled_rect(Rect(Vector(pos_x - menu_width/2-4, pos_y - menu_height/2 - 10-4),
                                Vector(pos_x + menu_width/2+4, pos_y - menu_height/2 + 10 + menu_height+4)),
                           Color(0.2f, 0.3f, 0.4f, 0.8f), 
                           20.0f,
                           LAYER_GUI-10);

  context.draw_filled_rect(Rect(Vector(pos_x - menu_width/2, pos_y - menu_height/2 - 10),
                                Vector(pos_x + menu_width/2, pos_y - menu_height/2 + 10 + menu_height)),
                           Color(0.6f, 0.7f, 0.8f, 0.5f), 
                           16.0f,
                           LAYER_GUI-10);

  if (!items[active_item]->help.empty())
  {
    int text_width  = (int) normal_font->get_text_width(items[active_item]->help);
    int text_height = (int) normal_font->get_text_height(items[active_item]->help);
      
    Rect text_rect(pos_x - text_width/2 - 8, 
                   SCREEN_HEIGHT - 48 - text_height/2 - 4,
                   pos_x + text_width/2 + 8, 
                   SCREEN_HEIGHT - 48 + text_height/2 + 4);
        
    context.draw_filled_rect(Rect(text_rect.p1 - Vector(4,4),
                                  text_rect.p2 + Vector(4,4)),
                             Color(0.2f, 0.3f, 0.4f, 0.8f), 
                             16.0f,
                             LAYER_GUI-10);
      
    context.draw_filled_rect(text_rect,
                             Color(0.6f, 0.7f, 0.8f, 0.5f), 
                             16.0f,
                             LAYER_GUI-10);

    context.draw_text(normal_font, items[active_item]->help,
                      Vector(pos_x, SCREEN_HEIGHT - 48 - text_height/2),
                      ALIGN_CENTER, LAYER_GUI);
  }

  if (effect_progress == 1.0f)
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

void
Menu::set_toggled(int id, bool toggled)
{
  get_item_by_id(id).toggled = toggled;
}

Menu*
Menu::get_parent() const
{
  if (MenuManager2::last_menus.empty())
    return 0;
  else
    return MenuManager2::last_menus.back();
}

/* Check for menu event */
void
Menu::event(const SDL_Event& event)
{
  if(effect_progress != 1.0f)
    return;

  switch(event.type) {
    case SDL_MOUSEBUTTONDOWN:
    {
      int x = int(event.motion.x * float(SCREEN_WIDTH)/g_screen->w);
      int y = int(event.motion.y * float(SCREEN_HEIGHT)/g_screen->h);

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
      float x = event.motion.x * SCREEN_WIDTH/g_screen->w;
      float y = event.motion.y * SCREEN_HEIGHT/g_screen->h;

      if(x > pos_x - get_width()/2 &&
         x < pos_x + get_width()/2 &&
         y > pos_y - get_height()/2 &&
         y < pos_y + get_height()/2)
      {
        int new_active_item
          = static_cast<int> ((y - (pos_y - get_height()/2)) / 24);

        /* only change the mouse focus to a selectable item */
        if ((items[new_active_item]->kind != MN_HL)
            && (items[new_active_item]->kind != MN_LABEL)
            && (items[new_active_item]->kind != MN_INACTIVE))
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
  for(size_t i = 0; i < items.size(); ++i) {
    MenuItem* item = items[i];
    if(item->id == id) {
      active_item = i;
      break;
    }
  }
}

/* EOF */
