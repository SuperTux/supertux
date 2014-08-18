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
#include <stdexcept>

#include "control/input_manager.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/timer.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

static const float MENU_REPEAT_INITIAL = 0.4f;
static const float MENU_REPEAT_RATE    = 0.1f;

Menu::Menu() :
  pos(),
  delete_character(),
  mn_input_char(),
  menu_repeat_time(),
  items(),
  arrange_left(),
  active_item()
{
  delete_character = 0;
  mn_input_char = '\0';

  pos.x        = SCREEN_WIDTH/2;
  pos.y        = SCREEN_HEIGHT/2;
  arrange_left = 0;
  active_item  = -1;
}

Menu::~Menu()
{
}

void
Menu::set_center_pos(float x, float y)
{
  pos.x = x;
  pos.y = y;
}

/* Add an item to a menu */
MenuItem*
Menu::add_item(std::unique_ptr<MenuItem> new_item)
{
  items.push_back(std::move(new_item));
  MenuItem* item = items.back().get();

  /* If a new menu is being built, the active item shouldn't be set to
   * something that isn't selectable. Set the active_item to the first
   * selectable item added.
   */
  if (active_item == -1
      && item->kind != MN_HL
      && item->kind != MN_LABEL
      && item->kind != MN_INACTIVE)
  {
    active_item = items.size() - 1;
  }

  return item;
}

MenuItem*
Menu::add_hl()
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_HL));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_label(const std::string& text)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_LABEL));
  item->text = text;
  return add_item(std::move(item));
}

MenuItem*
Menu::add_controlfield(int id, const std::string& text,
                       const std::string& mapping)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_CONTROLFIELD, id));
  item->change_text(text);
  item->change_input(mapping);
  return add_item(std::move(item));
}

MenuItem*
Menu::add_entry(int id, const std::string& text)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_ACTION, id));
  item->text = text;
  return add_item(std::move(item));
}

MenuItem*
Menu::add_inactive(int id, const std::string& text)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_INACTIVE, id));
  item->text = text;
  return add_item(std::move(item));
}

MenuItem*
Menu::add_toggle(int id, const std::string& text, bool toogled)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_TOGGLE, id));
  item->text = text;
  item->toggled = toogled;
  return add_item(std::move(item));
}

MenuItem*
Menu::add_string_select(int id, const std::string& text)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_STRINGSELECT, id));
  item->text = text;
  return add_item(std::move(item));
}

MenuItem*
Menu::add_back(const std::string& text)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_BACK));
  item->text = text;
  return add_item(std::move(item));
}

MenuItem*
Menu::add_submenu(const std::string& text, int submenu)
{
  std::unique_ptr<MenuItem> item(new MenuItem(MN_GOTO));
  item->text = text;
  item->target_menu = submenu;
  return add_item(std::move(item));
}

void
Menu::clear()
{
  items.clear();
  active_item = -1;
}

void
Menu::process_input()
{
  int menu_height = (int) get_height();
  if (menu_height > SCREEN_HEIGHT)
  { // Scrolling
    int scroll_offset = (menu_height - SCREEN_HEIGHT) / 2 + 32;
    pos.y = SCREEN_HEIGHT/2 - scroll_offset * ((float(active_item) / (items.size()-1)) - 0.5f) * 2.0f;
  }

  MenuAction menuaction = MENU_ACTION_NONE;
  Controller* controller = InputManager::current()->get_controller();
  /** check main input controller... */
  if(controller->pressed(Controller::UP)) {
    menuaction = MENU_ACTION_UP;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(controller->hold(Controller::UP) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_UP;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(controller->pressed(Controller::DOWN)) {
    menuaction = MENU_ACTION_DOWN;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(controller->hold(Controller::DOWN) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_DOWN;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(controller->pressed(Controller::LEFT)) {
    menuaction = MENU_ACTION_LEFT;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(controller->hold(Controller::LEFT) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_LEFT;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(controller->pressed(Controller::RIGHT)) {
    menuaction = MENU_ACTION_RIGHT;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(controller->hold(Controller::RIGHT) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_RIGHT;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(controller->pressed(Controller::ACTION)
     || controller->pressed(Controller::MENU_SELECT)) {
    menuaction = MENU_ACTION_HIT;
  }
  if(controller->pressed(Controller::PAUSE_MENU)
    || controller->pressed(Controller::MENU_BACK)) {
    menuaction = MENU_ACTION_BACK;
  }

  if(items.size() == 0)
    return;

  // The menu_action() call can pop() the menu from the stack and thus
  // delete it, so it's important that no further member variables are
  // accessed after this call
  process_action(menuaction);
}

void
Menu::process_action(MenuAction menuaction)
{
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

        menu_action(items[active_item].get());
      }
      break;

    case MENU_ACTION_RIGHT:
      if(items[active_item]->kind == MN_STRINGSELECT) {
        if(items[active_item]->selected+1 < items[active_item]->list.size())
          items[active_item]->selected++;
        else
          items[active_item]->selected = 0;

        menu_action(items[active_item].get());
      }
      break;

    case MENU_ACTION_HIT: {
      switch (items[active_item]->kind) {
        case MN_GOTO:
          assert(items[active_item]->target_menu != 0);
          MenuManager::instance().push_menu(items[active_item]->target_menu);
          break;

        case MN_TOGGLE:
          items[active_item]->toggled = !items[active_item]->toggled;
          menu_action(items[active_item].get());
          break;

        case MN_CONTROLFIELD:
          menu_action(items[active_item].get());
          break;

        case MN_ACTION:
          menu_action(items[active_item].get());
          break;

        case MN_STRINGSELECT:
          if(items[active_item]->selected+1 < items[active_item]->list.size())
            items[active_item]->selected++;
          else
            items[active_item]->selected = 0;

          menu_action(items[active_item].get());
          break;

        case MN_TEXTFIELD:
        case MN_NUMFIELD:
          menuaction = MENU_ACTION_DOWN;
          process_input();
          break;

        case MN_BACK:
          MenuManager::instance().pop_menu();
          return;

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
      MenuManager::instance().pop_menu();
      return;

    case MENU_ACTION_NONE:
      break;
  }
}

void
Menu::draw_item(DrawingContext& context, int index)
{
  float menu_height = get_height();
  float menu_width  = get_width();

  MenuItem& pitem = *(items[index]);

  Color text_color = default_color;
  float x_pos       = pos.x;
  float y_pos       = pos.y + 24*index - menu_height/2 + 12;
  int text_width  = int(Resources::normal_font->get_text_width(pitem.text));
  int input_width = int(Resources::normal_font->get_text_width(pitem.input) + 10);
  int list_width = 0;

  float left  = pos.x - menu_width/2 + 16;
  float right = pos.x + menu_width/2 - 16;

  if(pitem.list.size() > 0) {
    list_width = (int) Resources::normal_font->get_text_width(pitem.list[pitem.selected]);
  }

  if (arrange_left)
    x_pos += 24 - menu_width/2 + (text_width + input_width + list_width)/2;

  if(index == active_item)
  {
    text_color = active_color;
  }

  if(active_item == index)
  {
    float blink = (sinf(real_time * M_PI * 1.0f)/2.0f + 0.5f) * 0.5f + 0.25f;
    context.draw_filled_rect(Rectf(Vector(pos.x - menu_width/2 + 10 - 2, y_pos - 12 - 2),
                                   Vector(pos.x + menu_width/2 - 10 + 2, y_pos + 12 + 2)),
                             Color(1.0f, 1.0f, 1.0f, blink),
                             14.0f,
                             LAYER_GUI-10);
    context.draw_filled_rect(Rectf(Vector(pos.x - menu_width/2 + 10, y_pos - 12),
                                   Vector(pos.x + menu_width/2 - 10, y_pos + 12)),
                             Color(1.0f, 1.0f, 1.0f, 0.5f),
                             12.0f,
                             LAYER_GUI-10);
  }

  switch (pitem.kind)
  {
    case MN_INACTIVE:
    {
      context.draw_text(Resources::normal_font, pitem.text,
                        Vector(pos.x, y_pos - int(Resources::normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, inactive_color);
      break;
    }

    case MN_HL:
    {
      // TODO
      float x = pos.x - menu_width/2;
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
      context.draw_text(Resources::big_font, pitem.text,
                        Vector(pos.x, y_pos - int(Resources::big_font->get_height()/2)),
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
          context.draw_text(Resources::normal_font,
                            pitem.get_input_with_symbol(true),
                            Vector(right, y_pos - int(Resources::normal_font->get_height()/2)),
                            ALIGN_RIGHT, LAYER_GUI, field_color);
        else
          context.draw_text(Resources::normal_font,
                            pitem.get_input_with_symbol(false),
                            Vector(right, y_pos - int(Resources::normal_font->get_height()/2)),
                            ALIGN_RIGHT, LAYER_GUI, field_color);
      }
      else
        context.draw_text(Resources::normal_font, pitem.input,
                          Vector(right, y_pos - int(Resources::normal_font->get_height()/2)),
                          ALIGN_RIGHT, LAYER_GUI, field_color);

      context.draw_text(Resources::normal_font, pitem.text,
                        Vector(left, y_pos - int(Resources::normal_font->get_height()/2)),
                        ALIGN_LEFT, LAYER_GUI, text_color);
      break;
    }
    case MN_STRINGSELECT:
    {
      float roff = Resources::arrow_left->get_width();
      // Draw left side
      context.draw_text(Resources::normal_font, pitem.text,
                        Vector(left, y_pos - int(Resources::normal_font->get_height()/2)),
                        ALIGN_LEFT, LAYER_GUI, text_color);

      // Draw right side
      context.draw_surface(Resources::arrow_left,
                           Vector(right - list_width - roff - roff, y_pos - 8),
                           LAYER_GUI);
      context.draw_surface(Resources::arrow_right,
                           Vector(right - roff, y_pos - 8),
                           LAYER_GUI);
      context.draw_text(Resources::normal_font, pitem.list[pitem.selected],
                        Vector(right - roff, y_pos - int(Resources::normal_font->get_height()/2)),
                        ALIGN_RIGHT, LAYER_GUI, text_color);
      break;
    }
    case MN_BACK:
    {
      context.draw_text(Resources::Resources::normal_font, pitem.text,
                        Vector(pos.x, y_pos - int(Resources::normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, text_color);
      context.draw_surface(Resources::back,
                           Vector(x_pos + text_width/2  + 16, y_pos - 8),
                           LAYER_GUI);
      break;
    }

    case MN_TOGGLE:
    {
      context.draw_text(Resources::normal_font, pitem.text,
                        Vector(pos.x - menu_width/2 + 16, y_pos - (Resources::normal_font->get_height()/2)),
                        ALIGN_LEFT, LAYER_GUI, text_color);

      if(pitem.toggled)
        context.draw_surface(Resources::checkbox_checked,
                             Vector(x_pos + (menu_width/2-16) - Resources::checkbox->get_width(), y_pos - 8),
                             LAYER_GUI + 1);
      else
        context.draw_surface(Resources::checkbox,
                             Vector(x_pos + (menu_width/2-16) - Resources::checkbox->get_width(), y_pos - 8),
                             LAYER_GUI + 1);
      break;
    }
    case MN_ACTION:
      context.draw_text(Resources::normal_font, pitem.text,
                        Vector(pos.x, y_pos - int(Resources::normal_font->get_height()/2)),
                        ALIGN_CENTER, LAYER_GUI, text_color);
      break;

    case MN_GOTO:
      context.draw_text(Resources::normal_font, pitem.text,
                        Vector(pos.x, y_pos - int(Resources::normal_font->get_height()/2)),
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
    FontPtr font = Resources::Resources::normal_font;
    if(items[i]->kind == MN_LABEL)
      font = Resources::big_font;

    float w = font->get_text_width(items[i]->text) +
      Resources::big_font->get_text_width(items[i]->input) + 16;
    if(items[i]->kind == MN_TOGGLE)
      w += 32;
    if (items[i]->kind == MN_STRINGSELECT)
      w += font->get_text_width(items[i]->list[items[i]->selected]) + 32;


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

void
Menu::on_window_resize()
{
  pos.x = SCREEN_WIDTH / 2;
  pos.y = SCREEN_HEIGHT / 2;
}

void
Menu::draw(DrawingContext& context)
{
  if (!items[active_item]->help.empty())
  {
    int text_width  = (int) Resources::normal_font->get_text_width(items[active_item]->help);
    int text_height = (int) Resources::normal_font->get_text_height(items[active_item]->help);

    Rectf text_rect(pos.x - text_width/2 - 8,
                   SCREEN_HEIGHT - 48 - text_height/2 - 4,
                   pos.x + text_width/2 + 8,
                   SCREEN_HEIGHT - 48 + text_height/2 + 4);

    context.draw_filled_rect(Rectf(text_rect.p1 - Vector(4,4),
                                   text_rect.p2 + Vector(4,4)),
                             Color(0.2f, 0.3f, 0.4f, 0.8f),
                             16.0f,
                             LAYER_GUI-10);

    context.draw_filled_rect(text_rect,
                             Color(0.6f, 0.7f, 0.8f, 0.5f),
                             16.0f,
                             LAYER_GUI-10);

    context.draw_text(Resources::normal_font, items[active_item]->help,
                      Vector(pos.x, SCREEN_HEIGHT - 48 - text_height/2),
                      ALIGN_CENTER, LAYER_GUI);
  }

  for(unsigned int i = 0; i < items.size(); ++i)
  {
    draw_item(context, i);
  }
}

MenuItem&
Menu::get_item_by_id(int id)
{
  for (const auto& item : items)
  {
    if (item->id == id)
    {
      return *item;
    }
  }

  throw std::runtime_error("MenuItem not found");
}

const MenuItem&
Menu::get_item_by_id(int id) const
{
  for (const auto& item : items)
  {
    if (item->id == id)
    {
      return *item;
    }
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

void
Menu::event(const SDL_Event& ev)
{
  switch(ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    if(ev.button.button == SDL_BUTTON_LEFT)
    {
      Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      int x = int(mouse_pos.x);
      int y = int(mouse_pos.y);

      if(x > pos.x - get_width()/2 &&
         x < pos.x + get_width()/2 &&
         y > pos.y - get_height()/2 &&
         y < pos.y + get_height()/2)
      {
        process_action(MENU_ACTION_HIT);
      }
    }
    break;

    case SDL_MOUSEMOTION:
    {
      Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      float x = mouse_pos.x;
      float y = mouse_pos.y;

      if(x > pos.x - get_width()/2 &&
         x < pos.x + get_width()/2 &&
         y > pos.y - get_height()/2 &&
         y < pos.y + get_height()/2)
      {
        int new_active_item
          = static_cast<int> ((y - (pos.y - get_height()/2)) / 24);

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
    if(items[i]->id == id) {
      active_item = i;
      break;
    }
  }
}

/* EOF */
