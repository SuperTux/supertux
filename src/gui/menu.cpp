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

#include "control/input_manager.hpp"
#include "gui/item_action.hpp"
#include "gui/item_back.hpp"
#include "gui/item_badguy_select.hpp"
#include "gui/item_color.hpp"
#include "gui/item_colorchannel.hpp"
#include "gui/item_colordisplay.hpp"
#include "gui/item_controlfield.hpp"
#include "gui/item_file.hpp"
#include "gui/item_goto.hpp"
#include "gui/item_hl.hpp"
#include "gui/item_inactive.hpp"
#include "gui/item_intfield.hpp"
#include "gui/item_label.hpp"
#include "gui/item_floatfield.hpp"
#include "gui/item_script.hpp"
#include "gui/item_script_line.hpp"
#include "gui/item_stringselect.hpp"
#include "gui/item_textfield.hpp"
#include "gui/item_toggle.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "math/util.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

static const float MENU_REPEAT_INITIAL = 0.4f;
static const float MENU_REPEAT_RATE    = 0.1f;

Menu::Menu() :
  pos(Vector(static_cast<float>(SCREEN_WIDTH) / 2.0f,
             static_cast<float>(SCREEN_HEIGHT) / 2.0f)),
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
MenuItem&
Menu::add_item(std::unique_ptr<MenuItem> new_item)
{
  items.push_back(std::move(new_item));
  MenuItem& item = *items.back();

  /* If a new menu is being built, the active item shouldn't be set to
   * something that isn't selectable. Set the active_item to the first
   * selectable item added.
   */

  if (active_item == -1 && !item.skippable())
  {
    active_item = static_cast<int>(items.size()) - 1;
  }

  calculate_width();

  return item;
}

MenuItem&
Menu::add_item(std::unique_ptr<MenuItem> new_item, int pos_)
{
  items.insert(items.begin()+pos_,std::move(new_item));
  MenuItem& item = *items[pos_];

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

ItemHorizontalLine&
Menu::add_hl()
{
  auto item = std::make_unique<ItemHorizontalLine>();
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemLabel&
Menu::add_label(const std::string& text)
{
  auto item = std::make_unique<ItemLabel>(text);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemControlField&
Menu::add_controlfield(int id, const std::string& text,
                       const std::string& mapping)
{
  auto item = std::make_unique<ItemControlField>(text, mapping, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemTextField&
Menu::add_textfield(const std::string& text, std::string* input, int id)
{
  auto item = std::make_unique<ItemTextField>(text, input, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemScript&
Menu::add_script(const std::string& text, std::string* script, int id)
{
  auto item = std::make_unique<ItemScript>(text, script, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemScriptLine&
Menu::add_script_line(std::string* input, int id)
{
  auto item = std::make_unique<ItemScriptLine>(input, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemIntField&
Menu::add_intfield(const std::string& text, int* input, int id)
{
  auto item = std::make_unique<ItemIntField>(text, input, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemFloatField&
Menu::add_floatfield(const std::string& text, float* input, int id)
{
  auto item = std::make_unique<ItemFloatField>(text, input, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemAction&
Menu::add_entry(int id, const std::string& text)
{
  auto item = std::make_unique<ItemAction>(text, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemAction&
Menu::add_entry(const std::string& text, std::function<void()> callback)
{
  auto item = std::make_unique<ItemAction>(text, -1, callback);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemInactive&
Menu::add_inactive(const std::string& text)
{
  auto item = std::make_unique<ItemInactive>(text);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemToggle&
Menu::add_toggle(int id, const std::string& text, bool* toggled)
{
  auto item = std::make_unique<ItemToggle>(text, toggled, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemToggle&
Menu::add_toggle(int id, const std::string& text,
                 std::function<bool()> get_func,
                 std::function<void(bool)> set_func)
{
  auto item = std::make_unique<ItemToggle>(text, get_func, set_func, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemStringSelect&
Menu::add_string_select(int id, const std::string& text, int* selected, const std::vector<std::string>& strings)
{
  auto item = std::make_unique<ItemStringSelect>(text, strings, selected, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemFile&
Menu::add_file(const std::string& text, std::string* input, const std::vector<std::string>& extensions,
               const std::string& basedir, int id)
{
  auto item = std::make_unique<ItemFile>(text, input, extensions, basedir, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemBack&
Menu::add_back(const std::string& text, int id)
{
  auto item = std::make_unique<ItemBack>(text, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemGoTo&
Menu::add_submenu(const std::string& text, int submenu, int id)
{
  auto item = std::make_unique<ItemGoTo>(text, submenu, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemColorChannel&
Menu::add_color_channel(float* input, Color channel, int id) {
  auto item = std::make_unique<ItemColorChannel>(input, channel, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemColorDisplay&
Menu::add_color_display(Color* color, int id) {
  auto item = std::make_unique<ItemColorDisplay>(color, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemColor&
Menu::add_color(const std::string& text, Color* color, int id) {
  auto item = std::make_unique<ItemColor>(text, color, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

ItemBadguySelect&
Menu::add_badguy_select(const std::string& text, std::vector<std::string>* badguys, int id) {
  auto item = std::make_unique<ItemBadguySelect>(text, badguys, id);
  auto item_ptr = item.get();
  add_item(std::move(item));
  return *item_ptr;
}

void
Menu::clear()
{
  items.clear();
  active_item = -1;
}

void
Menu::process_input(const Controller& controller)
{
  int menu_height = static_cast<int>(get_height());
  if (menu_height > SCREEN_HEIGHT)
  { // Scrolling
    int scroll_offset = (menu_height - SCREEN_HEIGHT) / 2 + 32;
    pos.y = static_cast<float>(SCREEN_HEIGHT) / 2.0f - static_cast<float>(scroll_offset) * ((static_cast<float>(active_item) / static_cast<float>(items.size() - 1)) - 0.5f) * 2.0f;
  }

  MenuAction menuaction = MenuAction::NONE;

  /** check main input controller... */
  if (controller.pressed(Controller::UP)) {
    menuaction = MenuAction::UP;
    menu_repeat_time = g_real_time + MENU_REPEAT_INITIAL;
  }
  if (controller.hold(Controller::UP) &&
     menu_repeat_time != 0 && g_real_time > menu_repeat_time) {
    menuaction = MenuAction::UP;
    menu_repeat_time = g_real_time + MENU_REPEAT_RATE;
  }

  if (controller.pressed(Controller::DOWN)) {
    menuaction = MenuAction::DOWN;
    menu_repeat_time = g_real_time + MENU_REPEAT_INITIAL;
  }
  if (controller.hold(Controller::DOWN) &&
     menu_repeat_time != 0 && g_real_time > menu_repeat_time) {
    menuaction = MenuAction::DOWN;
    menu_repeat_time = g_real_time + MENU_REPEAT_RATE;
  }

  if (controller.pressed(Controller::LEFT)) {
    menuaction = MenuAction::LEFT;
    menu_repeat_time = g_real_time + MENU_REPEAT_INITIAL;
  }
  if (controller.hold(Controller::LEFT) &&
     menu_repeat_time != 0 && g_real_time > menu_repeat_time) {
    menuaction = MenuAction::LEFT;
    menu_repeat_time = g_real_time + MENU_REPEAT_RATE;
  }

  if (controller.pressed(Controller::RIGHT)) {
    menuaction = MenuAction::RIGHT;
    menu_repeat_time = g_real_time + MENU_REPEAT_INITIAL;
  }
  if (controller.hold(Controller::RIGHT) &&
     menu_repeat_time != 0 && g_real_time > menu_repeat_time) {
    menuaction = MenuAction::RIGHT;
    menu_repeat_time = g_real_time + MENU_REPEAT_RATE;
  }

  if (controller.pressed(Controller::ACTION) ||
     controller.pressed(Controller::MENU_SELECT) ||
     (!is_sensitive() && controller.pressed(Controller::MENU_SELECT_SPACE))) {
    menuaction = MenuAction::HIT;
  }

  if (controller.pressed(Controller::ESCAPE) ||
     controller.pressed(Controller::CHEAT_MENU) ||
     controller.pressed(Controller::DEBUG_MENU) ||
     controller.pressed(Controller::MENU_BACK)) {
    menuaction = MenuAction::BACK;
  }

  if (controller.pressed(Controller::REMOVE)) {
    menuaction = MenuAction::REMOVE;
    menu_repeat_time = g_real_time + MENU_REPEAT_INITIAL;
  }
  if (controller.hold(Controller::REMOVE) &&
     menu_repeat_time != 0 && g_real_time > menu_repeat_time) {
    menuaction = MenuAction::REMOVE;
    menu_repeat_time = g_real_time + MENU_REPEAT_RATE;
  }

  if (items.size() == 0)
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

  switch (menuaction) {
    case MenuAction::UP:
      do {
        if (active_item > 0)
          --active_item;
        else
          active_item = int(items.size())-1;
      } while (items[active_item]->skippable()
               && (active_item != last_active_item));
      break;

    case MenuAction::DOWN:
      do {
        if (active_item < int(items.size())-1 )
          ++active_item;
        else
          active_item = 0;
      } while (items[active_item]->skippable()
               && (active_item != last_active_item));
      break;

    case MenuAction::BACK:
      if (on_back_action()) {
        MenuManager::instance().pop_menu();
      }
      return;

    default:
      break;
  }

  if (items[active_item]->no_other_action()) {
    items[active_item]->process_action(menuaction);
    return;
  }

  items[active_item]->process_action(menuaction);
  if (items[active_item]->changes_width()) {
    calculate_width();
  }
  if (menuaction == MenuAction::HIT) {
    menu_action(*items[active_item]);
  }
}

void
Menu::draw_item(DrawingContext& context, int index)
{
  float menu_height = get_height();
  float menu_width_ = get_width();

  MenuItem* pitem = items[index].get();

  float x_pos       = pos.x - menu_width_/2;
  float y_pos       = pos.y + 24.0f * static_cast<float>(index) - menu_height / 2.0f + 12.0f;

  pitem->draw(context, Vector(x_pos, y_pos), static_cast<int>(menu_width_), active_item == index);

  if (active_item == index)
  {
    float blink = (sinf(g_real_time * math::PI * 1.0f)/2.0f + 0.5f) * 0.5f + 0.25f;
    context.color().draw_filled_rect(Rectf(Vector(pos.x - menu_width_/2 + 10 - 2, y_pos - 12 - 2),
                                   Vector(pos.x + menu_width_/2 - 10 + 2, y_pos + 12 + 2)),
                             Color(1.0f, 1.0f, 1.0f, blink),
                             14.0f,
                             LAYER_GUI-10);
    context.color().draw_filled_rect(Rectf(Vector(pos.x - menu_width_/2 + 10, y_pos - 12),
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
  for (unsigned int i = 0; i < items.size(); ++i)
  {
    float w = static_cast<float>(items[i]->get_width());
    if (w > max_width)
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
  return static_cast<float>(items.size() * 24);
}

void
Menu::on_window_resize()
{
  pos.x = static_cast<float>(SCREEN_WIDTH) / 2.0f;
  pos.y = static_cast<float>(SCREEN_HEIGHT) / 2.0f;
}

void
Menu::draw(DrawingContext& context)
{
  if (!items[active_item]->get_help().empty())
  {
    int text_width  = static_cast<int>(Resources::normal_font->get_text_width(items[active_item]->get_help()));
    int text_height = static_cast<int>(Resources::normal_font->get_text_height(items[active_item]->get_help()));

    Rectf text_rect(pos.x - static_cast<float>(text_width) / 2.0f - 8.0f,
                    static_cast<float>(SCREEN_HEIGHT) - 48.0f - static_cast<float>(text_height) / 2.0f - 4.0f,
                    pos.x + static_cast<float>(text_width) / 2.0f + 8.0f,
                    static_cast<float>(SCREEN_HEIGHT) - 48.0f + static_cast<float>(text_height) / 2.0f + 4.0f);

    context.color().draw_filled_rect(Rectf(text_rect.p1() - Vector(4,4),
                                             text_rect.p2() + Vector(4,4)),
                                       Color(0.2f, 0.3f, 0.4f, 0.8f),
                                       16.0f,
                                       LAYER_GUI-10);

    context.color().draw_filled_rect(text_rect,
                                       Color(0.6f, 0.7f, 0.8f, 0.5f),
                                       16.0f,
                                       LAYER_GUI-10);

    context.color().draw_text(Resources::normal_font, items[active_item]->get_help(),
                              Vector(pos.x, static_cast<float>(SCREEN_HEIGHT) - 48.0f - static_cast<float>(text_height) / 2.0f),
                              ALIGN_CENTER, LAYER_GUI);
  }

  for (unsigned int i = 0; i < items.size(); ++i)
  {
    draw_item(context, i);
  }
}

MenuItem&
Menu::get_item_by_id(int id)
{
  for (const auto& item : items)
  {
    if (item->get_id() == id)
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
    if (item->get_id() == id)
    {
      return *item;
    }
  }

  throw std::runtime_error("MenuItem not found");
}

int Menu::get_active_item_id() const
{
  return items[active_item]->get_id();
}

void
Menu::event(const SDL_Event& ev)
{
  items[active_item]->event(ev);
  switch (ev.type) {
    case SDL_KEYDOWN:
    case SDL_TEXTINPUT:
      if (((ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_BACKSPACE) ||
         ev.type == SDL_TEXTINPUT) && items[active_item]->changes_width())
      {
        // Changed item value? Let's recalculate width:
        calculate_width();
      }
    break;

    case SDL_MOUSEBUTTONDOWN:
    if (ev.button.button == SDL_BUTTON_LEFT)
    {
      Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);

      if (mouse_pos.x > pos.x - get_width() / 2.0f &&
          mouse_pos.x < pos.x + get_width() / 2.0f &&
          mouse_pos.y > pos.y - get_height() / 2.0f &&
          mouse_pos.y < pos.y + get_height() / 2.0f)
      {
        process_action(MenuAction::HIT);
      }
    }
    break;

    case SDL_MOUSEMOTION:
    {
      Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);
      float x = mouse_pos.x;
      float y = mouse_pos.y;

      if (x > pos.x - get_width()/2 &&
         x < pos.x + get_width()/2 &&
         y > pos.y - get_height()/2 &&
         y < pos.y + get_height()/2)
      {
        int new_active_item
          = static_cast<int> ((y - (pos.y - get_height()/2)) / 24);

        /* only change the mouse focus to a selectable item */
        if (!items[new_active_item]->skippable())
          active_item = new_active_item;

        if (MouseCursor::current())
          MouseCursor::current()->set_state(MouseCursorState::LINK);
      }
      else
      {
        if (MouseCursor::current())
          MouseCursor::current()->set_state(MouseCursorState::NORMAL);
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
  for (size_t i = 0; i < items.size(); ++i) {
    if (items[i]->get_id() == id) {
      active_item = static_cast<int>(i);
      break;
    }
  }
}

bool
Menu::is_sensitive() const {
  return false;
}

/* EOF */
