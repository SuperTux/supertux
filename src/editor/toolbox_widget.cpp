//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "editor/toolbox_widget.hpp"

#include "editor/editor.hpp"
#include "editor/object_info.hpp"
#include "editor/tile_selection.hpp"
#include "editor/tip.hpp"
#include "editor/tool_icon.hpp"
#include "editor/util.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/console.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

EditorToolboxWidget::EditorToolboxWidget(Editor& editor) :
  m_editor(editor),
  m_tiles(new TileSelection()),
  m_object(),
  m_object_tip(),
  m_input_type(InputType::NONE),
  m_active_tilegroup(),
  m_active_objectgroup(-1),
  m_object_info(new ObjectInfo()),
  m_rubber(new ToolIcon("images/engine/editor/rubber.png")),
  m_select_mode(new ToolIcon("images/engine/editor/select-mode0.png")),
  m_move_mode(new ToolIcon("images/engine/editor/move-mode0.png")),
  m_undo_mode(new ToolIcon("images/engine/editor/arrow.png")),
  m_hovered_item(HoveredItem::NONE),
  m_hovered_tile(-1),
  m_tile_scrolling(TileScrolling::NONE),
  m_using_scroll_wheel(false),
  m_wheel_scroll_amount(0),
  m_starting_tile(0),
  m_dragging(false),
  m_drag_start(0, 0),
  m_Xpos(512),
  m_mouse_pos(0, 0),
  m_has_mouse_focus(false)
{
  m_select_mode->push_mode("images/engine/editor/select-mode1.png");
  m_select_mode->push_mode("images/engine/editor/select-mode2.png");
  m_move_mode->push_mode("images/engine/editor/move-mode1.png");
  m_undo_mode->push_mode("images/engine/editor/redo.png");
  //settings_mode->push_mode("images/engine/editor/settings-mode1.png");
}

void
EditorToolboxWidget::draw(DrawingContext& context)
{
  //SCREEN_WIDTH SCREEN_HEIGHT
  context.color().draw_filled_rect(Rectf(Vector(static_cast<float>(m_Xpos), 0),
                                         Vector(static_cast<float>(context.get_width()),
                                                static_cast<float>(context.get_height()))),
                                     g_config->editorcolor,
                                     0.0f, LAYER_GUI-10);
  if (m_dragging) {
    context.color().draw_filled_rect(selection_draw_rect(), Color(0.2f, 0.4f, 1.0f, 0.6f),
                                       0.0f, LAYER_GUI+1);
  }

  if (m_hovered_item != HoveredItem::NONE)
  {
    if (m_object_tip) {
      m_object_tip->draw(context, Vector(m_mouse_pos.x + 25, m_mouse_pos.y), true);
    }

    context.color().draw_filled_rect(get_item_rect(m_hovered_item),
                                       g_config->editorhovercolor,
                                       0.0f, LAYER_GUI - 5);
  }

  context.color().draw_text(Resources::normal_font, _("Tiles"),
                            Vector(static_cast<float>(context.get_width()), 5),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::default_color);
  context.color().draw_text(Resources::normal_font, _("Objects"),
                            Vector(static_cast<float>(context.get_width()), 37),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::default_color);

  m_rubber->draw(context);
  m_undo_mode->draw(context);
  switch (m_input_type)
  {
    case InputType::TILE:
      m_select_mode->draw(context);
      break;

    case InputType::OBJECT:
      m_move_mode->draw(context);
      break;

    default:
      break;
  }

  draw_tilegroup(context);
  draw_objectgroup(context);
}

void
EditorToolboxWidget::draw_tilegroup(DrawingContext& context)
{
  if (m_input_type == InputType::TILE) {
    int pos = -1;
    for (auto& tile_ID : m_active_tilegroup->tiles) {
      pos++;
      if (pos < m_starting_tile) {
        continue;
      }
      auto position = get_tile_coords(pos - m_starting_tile);
      draw_tile(context.color(), *m_editor.get_tileset(), tile_ID, position, LAYER_GUI - 9);

      if (g_config->developer_mode && m_active_tilegroup->developers_group)
      {
        // Display tile ID on top of tile:
        context.color().draw_text(Resources::console_font, std::to_string(tile_ID),
                                  position + Vector(16, 16), ALIGN_CENTER, LAYER_GUI - 9, Color::WHITE);
      }
      /*if (tile_ID == 0) {
        continue;
      }
      const Tile* tg_tile = m_editor.get_tileset()->get(tile_ID);
      tg_tile->draw(context.color(), get_tile_coords(pos - starting_tile), LAYER_GUI-9);*/
    }
  }
}

void
EditorToolboxWidget::draw_objectgroup(DrawingContext& context)
{
  if (m_input_type == InputType::OBJECT) {
    int pos = -1;
    for (auto& icon : m_object_info->m_groups[m_active_objectgroup].get_icons()) {
      pos++;
      if (pos < m_starting_tile) {
        continue;
      }
      icon.draw(context, get_tile_coords(pos - m_starting_tile));
    }
  }
}

void
EditorToolboxWidget::update(float dt_sec)
{
  switch (m_tile_scrolling)
  {
    case TileScrolling::UP:
      {
        if (m_starting_tile > 0)
        {
          if (m_using_scroll_wheel)
          {
            m_starting_tile -= 4 * m_wheel_scroll_amount;
            if (m_starting_tile < 0)
            {
              m_starting_tile = 0;
            }
            m_tile_scrolling = TileScrolling::NONE;
          }
          else
          {
            m_starting_tile -= 4;
          }
        }
      }
      break;

    case TileScrolling::DOWN:
      {
        int size;
        if (m_input_type == InputType::OBJECT) {
          size = static_cast<int>(m_object_info->m_groups[m_active_objectgroup].get_icons().size());
        } else {
          if (m_active_tilegroup == nullptr)
          {
            return;
          }
          size = static_cast<int>(m_active_tilegroup->tiles.size());
        }
        if (m_starting_tile < size-5) {
          if (m_using_scroll_wheel)
          {
            m_starting_tile -= 4 * m_wheel_scroll_amount;
            if (m_starting_tile > size - 4)
            {
              m_starting_tile = size - 4;
            }
            m_tile_scrolling = TileScrolling::NONE;
          }
          else
          {
            m_starting_tile += 4;
          }
        }
      }
      break;

    default:
      break;
  }
}

Rectf
EditorToolboxWidget::normalize_selection() const
{
  Vector drag_start_ = m_drag_start;
  Vector drag_end = Vector(static_cast<float>(m_hovered_tile % 4),
                           static_cast<float>(m_hovered_tile / 4)); // NOLINT
  if (drag_start_.x > drag_end.x) {
    std::swap(drag_start_.x, drag_end.x);
  }
  if (drag_start_.y > drag_end.y) {
    std::swap(drag_start_.y, drag_end.y);
  }
  return Rectf(drag_start_, drag_end);
}

Rectf
EditorToolboxWidget::selection_draw_rect() const
{
  Rectf select = normalize_selection();
  select.set_p2(select.p2() + Vector(1, 1));
  select.set_p1((select.p1() * 32.0f) + Vector(static_cast<float>(m_Xpos), static_cast<float>(m_Ypos)));
  select.set_p2((select.p2() * 32.0f) + Vector(static_cast<float>(m_Xpos), static_cast<float>(m_Ypos)));
  return select;
}

void
EditorToolboxWidget::update_selection()
{
  Rectf select = normalize_selection();
  m_tiles->m_tiles.clear();
  m_tiles->m_width = static_cast<int>(select.get_width() + 1);
  m_tiles->m_height = static_cast<int>(select.get_height() + 1);

  int size = static_cast<int>(m_active_tilegroup->tiles.size());
  for (int y = static_cast<int>(select.get_top()); y <= static_cast<int>(select.get_bottom()); y++) {
    for (int x = static_cast<int>(select.get_left()); x <= static_cast<int>(select.get_right()); x++) {
      int tile_pos = y*4 + x + m_starting_tile;
      if (tile_pos < size && tile_pos >= 0) {
        m_tiles->m_tiles.push_back(m_active_tilegroup->tiles[tile_pos]);
      } else {
        m_tiles->m_tiles.push_back(0);
      }
    }
  }
}

bool
EditorToolboxWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  m_dragging = false;
  return false;
}

bool
EditorToolboxWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (button.button == SDL_BUTTON_LEFT)
  {
    switch (m_hovered_item)
    {
      case HoveredItem::TILEGROUP:
        if (m_editor.get_tileset()->get_tilegroups().size() > 1)
        {
          m_editor.disable_keyboard();
          MenuManager::instance().push_menu(MenuStorage::EDITOR_TILEGROUP_MENU);
        }
        else
        {
          m_active_tilegroup.reset(new Tilegroup(m_editor.get_tileset()->get_tilegroups()[0]));
          m_input_type = EditorToolboxWidget::InputType::TILE;
          m_starting_tile = 0;
          update_mouse_icon();
        }
        return true;

      case HoveredItem::OBJECTS:
        if ((m_editor.get_level()->is_worldmap() && m_object_info->get_num_worldmap_groups() > 1) ||
            (!m_editor.get_level()->is_worldmap() && m_object_info->get_num_level_groups() > 1))
        {
          m_editor.disable_keyboard();
          MenuManager::instance().push_menu(MenuStorage::EDITOR_OBJECTGROUP_MENU);
        }
        else
        {
          if (m_editor.get_level()->is_worldmap())
          {
            m_active_objectgroup = m_object_info->get_first_worldmap_group_index();
          }
          else
          {
            m_active_objectgroup = 0;
          }
          m_input_type = EditorToolboxWidget::InputType::OBJECT;
          m_starting_tile = 0;
          update_mouse_icon();
        }
        return true;

      case HoveredItem::TILE:
        switch (m_input_type)
        {
          case InputType::TILE:
            {
              m_dragging = true;
              m_drag_start = Vector(static_cast<float>(m_hovered_tile % 4),
                                    static_cast<float>(m_hovered_tile / 4)); // NOLINT
              int size = static_cast<int>(m_active_tilegroup->tiles.size());
              int tile_pos = m_hovered_tile + m_starting_tile;
              if (tile_pos < size && tile_pos >= 0) {
                m_tiles->set_tile(m_active_tilegroup->tiles[tile_pos]);
              } else {
                m_tiles->set_tile(0);
              }
            }
            break;

          case InputType::OBJECT:
            {
              int size = static_cast<int>(m_object_info->m_groups[m_active_objectgroup].get_icons().size());
              if (m_hovered_tile < size && m_hovered_tile >= 0) {
                m_object = m_object_info->m_groups[m_active_objectgroup].get_icons()[m_hovered_tile + m_starting_tile].get_object_class();
              }
              update_mouse_icon();
            }
            break;

          default:
            break;
        }
        return true;

      case HoveredItem::TOOL:
        switch (m_hovered_tile)
        {
          case 0:
            m_tiles->set_tile(0);
            m_object = "";
            update_mouse_icon();
            break;

          case 1:
            if (m_input_type == InputType::TILE) {
              m_select_mode->next_mode();
            }
            update_mouse_icon();
            break;

          case 2:
            if (m_input_type == InputType::OBJECT) {
              m_move_mode->next_mode();
            }
            update_mouse_icon();
            break;
			
	      case 3:
		    m_object = "#move";
			update_mouse_icon();
			break;

          default:
            break;
        }
        return true;

      case HoveredItem::NONE:
        return false;

      default:
        return false;
    }
  }
  else
  {
    return false;
  }
}

bool
EditorToolboxWidget::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  float x = m_mouse_pos.x - static_cast<float>(m_Xpos);
  float y = m_mouse_pos.y - static_cast<float>(m_Ypos);

  if (x < 0) {
    m_hovered_item = HoveredItem::NONE;
    m_tile_scrolling = TileScrolling::NONE;
    m_has_mouse_focus = false;
    m_object_tip = nullptr;
    return false;
  }

  // mouse is currently over the toolbox
  m_has_mouse_focus = true;

  if (y < 0) {
    if (y < -64) {
      m_hovered_item = HoveredItem::TILEGROUP;
    } else if (y < -32) {
      m_hovered_item = HoveredItem::OBJECTS;
    } else {
      m_hovered_item = HoveredItem::TOOL;
      m_hovered_tile = get_tool_pos(m_mouse_pos);
    }
    m_tile_scrolling = TileScrolling::NONE;
    m_object_tip = nullptr;
    return false;
  } else {
    const int prev_hovered_tile = std::move(m_hovered_tile);
    m_hovered_item = HoveredItem::TILE;
    m_hovered_tile = get_tile_pos(m_mouse_pos);
    if (m_dragging && m_input_type == InputType::TILE) {
      update_selection();
    }
    else if (m_input_type == InputType::OBJECT && m_hovered_tile != prev_hovered_tile) {
      const auto& icons = m_object_info->m_groups[m_active_objectgroup].get_icons();
      if (m_hovered_tile + m_starting_tile < static_cast<int>(icons.size())) {
        const std::string obj_class = icons[m_hovered_tile + m_starting_tile].get_object_class();
        std::string obj_name;
        try {
          obj_name = GameObjectFactory::instance().get_display_name(obj_class);
        }
        catch (std::exception& err) {
          log_warning << "Unable to find name for object with class \"" << obj_class << "\": " << err.what() << std::endl;
          obj_name = obj_class;
        }
        m_object_tip = std::make_unique<Tip>(obj_name);
      }
      else {
        m_object_tip = nullptr;
      }
    }
  }

  if (y < 16) {
    m_tile_scrolling = TileScrolling::UP;
    m_using_scroll_wheel = false;
  } else if (y > static_cast<float>(SCREEN_HEIGHT - 16 - m_Ypos)) {
    m_tile_scrolling = TileScrolling::DOWN;
    m_using_scroll_wheel = false;
  } else {
    m_tile_scrolling = TileScrolling::NONE;
  }

  return false;
}

bool
EditorToolboxWidget::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (m_hovered_item != HoveredItem::NONE)
  {
    if (wheel.y > 0) {
      m_tile_scrolling = TileScrolling::UP;
    }
    else {
      m_tile_scrolling = TileScrolling::DOWN;
    }
    m_using_scroll_wheel = true;
    m_wheel_scroll_amount = wheel.y;
  }
  return false;
}

void
EditorToolboxWidget::resize()
{
  m_Xpos = SCREEN_WIDTH - 128;
  m_rubber->m_pos        = Vector(static_cast<float>(m_Xpos)        , 64.0f);
  m_select_mode->m_pos   = Vector(static_cast<float>(m_Xpos) + 32.0f, 64.0f);
  m_move_mode->m_pos     = Vector(static_cast<float>(m_Xpos) + 64.0f, 64.0f);
  m_undo_mode->m_pos     = Vector(static_cast<float>(m_Xpos) + 96.0f, 64.0f);
}

void
EditorToolboxWidget::setup()
{
  resize();
  m_tiles->set_tile(0);
}

void
EditorToolboxWidget::update_mouse_icon()
{
  switch (m_input_type) {
    case InputType::NONE:
      MouseCursor::current()->set_icon(nullptr);
      break;
    case InputType::OBJECT:
      if (m_object.empty()) {
        MouseCursor::current()->set_icon(m_rubber->get_current_surface());
      } else {
        MouseCursor::current()->set_icon(m_move_mode->get_current_surface());
      }
      break;
    case InputType::TILE:
      MouseCursor::current()->set_icon(m_select_mode->get_current_surface());
      break;
    default:
      break;
  }
}

Vector
EditorToolboxWidget::get_tile_coords(const int pos) const
{
  int x = pos%4;
  int y = pos/4;
  return Vector(static_cast<float>(x * 32 + m_Xpos),
                static_cast<float>(y * 32 + m_Ypos));
}

int
EditorToolboxWidget::get_tile_pos(const Vector& coords) const
{
  int x = static_cast<int>((coords.x - static_cast<float>(m_Xpos)) / 32.0f);
  int y = static_cast<int>((coords.y - static_cast<float>(m_Ypos)) / 32.0f);
  return y*4 + x;
}

Vector
EditorToolboxWidget::get_tool_coords(const int pos) const
{
  int x = pos%4;
  int y = pos/4;
  return Vector(static_cast<float>(x * 32 + m_Xpos),
                static_cast<float>(y * 32 + 64));
}

int
EditorToolboxWidget::get_tool_pos(const Vector& coords) const
{
  int x = static_cast<int>((coords.x - static_cast<float>(m_Xpos)) / 32.0f);
  int y = static_cast<int>((coords.y - 64.0f) / 32.0f);
  return y*4 + x;
}

Rectf
EditorToolboxWidget::get_item_rect(const HoveredItem& item) const
{
  switch (item)
  {
    case HoveredItem::TILEGROUP: return Rectf(Vector(static_cast<float>(m_Xpos), 0.0f), Vector(static_cast<float>(SCREEN_WIDTH), 32.0f));
    case HoveredItem::OBJECTS:   return Rectf(Vector(static_cast<float>(m_Xpos), 32.0f), Vector(static_cast<float>(SCREEN_WIDTH), 64.0f));
    case HoveredItem::TILE:
    {
      auto coords = get_tile_coords(m_hovered_tile);
      return Rectf(coords, coords + Vector(32, 32));
    }
    case HoveredItem::TOOL:
    {
      auto coords = get_tool_coords(m_hovered_tile);
      return Rectf(coords, coords + Vector(32, 32));
    }
    case HoveredItem::NONE:
    default:
      return Rectf();
  }
}

int
EditorToolboxWidget::get_tileselect_select_mode() const
{
  return m_select_mode->get_mode();
}

int
EditorToolboxWidget::get_tileselect_move_mode() const
{
  return m_move_mode->get_mode();
}

void
EditorToolboxWidget::select_tilegroup(int id)
{
  m_active_tilegroup.reset(new Tilegroup(m_editor.get_tileset()->get_tilegroups()[id]));
  m_input_type = EditorToolboxWidget::InputType::TILE;
  m_starting_tile = 0;
  update_mouse_icon();
}

void
EditorToolboxWidget::select_objectgroup(int id)
{
  m_active_objectgroup = id;
  m_input_type = EditorToolboxWidget::InputType::OBJECT;
  m_starting_tile = 0;
  update_mouse_icon();
}

bool
EditorToolboxWidget::has_mouse_focus() const
{
  return m_has_mouse_focus;
}

void
EditorToolboxWidget::show_tile_in_toolbox(uint32_t tile)
{
  for (const auto& tilegroup : m_editor.get_tileset()->get_tilegroups())
  {
    const auto& tiles = tilegroup.tiles;

    for (int id = 0; id < tiles.size(); id++)
    {
      if (tiles[id] == tile)
      {
        m_active_tilegroup.reset(new Tilegroup(tilegroup));
        m_input_type = EditorToolboxWidget::InputType::TILE;
        m_starting_tile = (id / 4) * 4;
        return;
      }
    }
  }
}

void
EditorToolboxWidget::show_object_in_toolbox(const std::string& classname)
{
  m_active_objectgroup = 0;
  m_input_type = EditorToolboxWidget::InputType::OBJECT;

  for (int i = 0; i < m_object_info->m_groups.size(); i++)
  {
    const auto& objs = m_object_info->m_groups.at(i).get_icons();

    for (int id = 0; id < objs.size(); id++)
    {
      const auto& obj = objs[id];

      if (obj.get_object_class() == classname)
      {
        m_active_objectgroup = i;
        m_input_type = EditorToolboxWidget::InputType::OBJECT;
        m_starting_tile = (id / 4) * 4;
        return;
      }
    }
  }
}

/* EOF */
