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

#define INCLUDE_MENU_ITEMS
// This causes the #include "gui/menu_item.hpp" to include all menu items too.

#include "control/input_manager.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/timer.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

static const float MENU_REPEAT_INITIAL = 0.4f;
static const float MENU_REPEAT_RATE    = 0.1f;

Menu::Menu() :
  pos(Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT/2)),
  delete_character(0),
  mn_input_char('\0'),
  menu_repeat_time(),
  menu_width(),
  items(),
  arrange_left(0),
  active_item(-1)
{
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
  auto item = items.back().get();

  /* If a new menu is being built, the active item shouldn't be set to
   * something that isn't selectable. Set the active_item to the first
   * selectable item added.
   */

  if (active_item == -1 && !item->skippable())
  {
    active_item = items.size() - 1;
  }

  calculate_width();

  return item;
}

MenuItem*
Menu::add_item(std::unique_ptr<MenuItem> new_item, int pos_)
{
  items.insert(items.begin()+pos_,std::move(new_item));
  MenuItem* item = items[pos_].get();

  /* When the item is inserted before the selected item, the
   * same menu item should be still selected.
   */

  if (active_item >= pos_)
  {
    active_item++;
  }

  calculate_width();

  return item;
}

void
Menu::delete_item(int pos_)
{
  items.erase(items.begin()+pos_);

  /* When the item is deleted before the selected item, the
   * same menu item should be still selected.
   */

  if (active_item >= pos_)
  {
    do {
      if (active_item > 0)
        --active_item;
      else
        active_item = int(items.size())-1;
    } while (items[active_item]->skippable());
  }
}

MenuItem*
Menu::add_hl()
{
  std::unique_ptr<ItemHorizontalLine> item(new ItemHorizontalLine());
  return add_item(std::move(item));
}

MenuItem*
Menu::add_label(const std::string& text)
{
  std::unique_ptr<ItemLabel> item(new ItemLabel(text));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_controlfield(int id, const std::string& text,
                       const std::string& mapping)
{
  std::unique_ptr<ItemControlField> item(new ItemControlField(text, mapping, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_textfield(const std::string& text, std::string* input, int id)
{
  std::unique_ptr<ItemTextField> item(new ItemTextField(text, input, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_script(const std::string& text, std::string* script, int id)
{
  std::unique_ptr<ItemScript> item(new ItemScript(text, script, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_script_line(std::string* input, int id)
{
  std::unique_ptr<ItemScriptLine> item(new ItemScriptLine(input, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_intfield(const std::string& text, int* input, int id)
{
  std::unique_ptr<ItemIntField> item(new ItemIntField(text, input, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_numfield(const std::string& text, float* input, int id)
{
  std::unique_ptr<ItemNumField> item(new ItemNumField(text, input, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_entry(int id, const std::string& text)
{
  std::unique_ptr<ItemAction> item(new ItemAction(text, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_inactive(const std::string& text)
{
  std::unique_ptr<ItemInactive> item(new ItemInactive(text));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_toggle(int id, const std::string& text, bool* toggled)
{
  std::unique_ptr<ItemToggle> item(new ItemToggle(text, toggled, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_string_select(int id, const std::string& text, int* selected, const std::vector<std::string>& strings)
{
  std::unique_ptr<ItemStringSelect> item(new ItemStringSelect(text, strings, selected, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_file(const std::string& text, std::string* input, const std::vector<std::string>& extensions, int id)
{
  std::unique_ptr<ItemFile> item(new ItemFile(text, input, extensions, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_back(const std::string& text, int id)
{
  std::unique_ptr<ItemBack> item(new ItemBack(text, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_submenu(const std::string& text, int submenu, int id)
{
  std::unique_ptr<ItemGoTo> item(new ItemGoTo(text, submenu, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_color_channel(float* input, Color channel, int id) {
  std::unique_ptr<ItemColorChannel> item(new ItemColorChannel(input, channel, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_color_display(Color* color, int id) {
  std::unique_ptr<ItemColorDisplay> item(new ItemColorDisplay(color, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_color(const std::string& text, Color* color, int id) {
  std::unique_ptr<ItemColor> item(new ItemColor(text, color, id));
  return add_item(std::move(item));
}

MenuItem*
Menu::add_badguy_select(const std::string& text, std::vector<std::string>* badguys, int id) {
  std::unique_ptr<ItemBadguySelect> item(new ItemBadguySelect(text, badguys, id));
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
  auto controller = InputManager::current()->get_controller();
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
     || controller->pressed(Controller::MENU_SELECT)
     || (!is_sensitive() && controller->pressed(Controller::MENU_SELECT_SPACE))) {
    menuaction = MENU_ACTION_HIT;
  }
  if(controller->pressed(Controller::ESCAPE) ||
     controller->pressed(Controller::CHEAT_MENU) ||
     controller->pressed(Controller::MENU_BACK)) {
    menuaction = MENU_ACTION_BACK;
  }

  if(controller->pressed(Controller::REMOVE)) {
    menuaction = MENU_ACTION_REMOVE;
    menu_repeat_time = real_time + MENU_REPEAT_INITIAL;
  }
  if(controller->hold(Controller::REMOVE) &&
     menu_repeat_time != 0 && real_time > menu_repeat_time) {
    menuaction = MENU_ACTION_REMOVE;
    menu_repeat_time = real_time + MENU_REPEAT_RATE;
  }

  if(items.size() == 0)
    return;

  // The menu_action() call can pop() the menu from the stack and thus
  // delete it, so it's important that no further member variables are
  // accessed after this call
  process_action(menuaction);
}

void
Menu::process_action(const MenuAction& menuaction)
{
  int last_active_item = active_item;

  switch(menuaction) {
    case MENU_ACTION_UP:
      do {
        if (active_item > 0)
          --active_item;
        else
          active_item = int(items.size())-1;
      } while (items[active_item]->skippable()
               && (active_item != last_active_item));
      break;

    case MENU_ACTION_DOWN:
      do {
        if(active_item < int(items.size())-1 )
          ++active_item;
        else
          active_item = 0;
      } while (items[active_item]->skippable()
               && (active_item != last_active_item));
      break;

    case MENU_ACTION_BACK:
      if(on_back_action()) {
        MenuManager::instance().pop_menu();
      }
      return;
      break;

    default:
      break;
  }

  if (items[active_item]->no_other_action()) {
    items[active_item]->process_action(menuaction);
    return;
  }

  items[active_item]->process_action(menuaction);
  if(items[active_item]->changes_width()) {
    calculate_width();
  }
  if(menuaction == MENU_ACTION_HIT) {
    menu_action(items[active_item].get());
  }
}

void
Menu::draw_item(DrawingContext& context, int index)
{
  float menu_height = get_height();
  float menu_width_ = get_width();

  MenuItem* pitem = items[index].get();

  float x_pos       = pos.x - menu_width_/2;
  float y_pos       = pos.y + 24*index - menu_height/2 + 12;

  pitem->draw(context, Vector(x_pos, y_pos), menu_width_, active_item == index);

  if(active_item == index)
  {
    float blink = (sinf(real_time * M_PI * 1.0f)/2.0f + 0.5f) * 0.5f + 0.25f;
    context.draw_filled_rect(Rectf(Vector(pos.x - menu_width_/2 + 10 - 2, y_pos - 12 - 2),
                                   Vector(pos.x + menu_width_/2 - 10 + 2, y_pos + 12 + 2)),
                             Color(1.0f, 1.0f, 1.0f, blink),
                             14.0f,
                             LAYER_GUI-10);
    context.draw_filled_rect(Rectf(Vector(pos.x - menu_width_/2 + 10, y_pos - 12),
                                   Vector(pos.x + menu_width_/2 - 10, y_pos + 12)),
                             Color(1.0f, 1.0f, 1.0f, 0.5f),
                             12.0f,
                             LAYER_GUI-10);
  }
}

void
Menu::calculate_width()
{
  /* The width of the menu has to be more than the width of the text
     with the most characters */
  float max_width = 0;
  for(unsigned int i = 0; i < items.size(); ++i)
  {
    float w = items[i]->get_width();
    if(w > max_width)
      max_width = w;
  }
  menu_width = max_width;
}

float
Menu::get_width() const
{
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

  throw std::runtime_error("MenuItem not found: " + std::to_string(id));
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

int Menu::get_active_item_id() const
{
  return items[active_item]->id;
}

void
Menu::event(const SDL_Event& ev)
{
  items[active_item]->event(ev);
  switch(ev.type) {
    case SDL_KEYDOWN:
    case SDL_TEXTINPUT:
      if(((ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_BACKSPACE) ||
         ev.type == SDL_TEXTINPUT) && items[active_item]->changes_width())
      {
        // Changed item value? Let's recalculate width:
        calculate_width();
      }
    break;

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
        if (!items[new_active_item]->skippable())
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

bool
Menu::is_sensitive() const {
  return false;
}

/* EOF */
