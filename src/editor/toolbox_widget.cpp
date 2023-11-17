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
  m_object_tip(new Tip()),
  m_input_type(InputType::NONE),
  m_active_tilegroup(),
  m_active_objectgroup(-1),
  m_object_info(new ObjectInfo()),
  m_scrollbar(),
  m_scroll_progress(1.f),
  m_rubber(new ToolIcon("images/engine/editor/rubber.png")),
  m_select_mode(new ToolIcon("images/engine/editor/select-mode0.png")),
  m_node_marker_mode(new ToolIcon("images/engine/editor/path_node.png")),
  m_move_mode(new ToolIcon("images/engine/editor/move-mode0.png")),
  m_undo_mode(new ToolIcon("images/engine/editor/arrow.png")),
  m_hovered_item(HoveredItem::NONE),
  m_hovered_tile(-1),
  m_dragging(false),
  m_drag_start(0, 0),
  m_Xpos(512),
  m_mouse_pos(0, 0),
  m_has_mouse_focus(false)
{
  m_scrollbar.reset(new ControlScrollbar(1.f, 1.f, m_scroll_progress, 35.f));

  m_select_mode->push_mode("images/engine/editor/select-mode1.png");
  m_select_mode->push_mode("images/engine/editor/select-mode2.png");
  m_select_mode->push_mode("images/engine/editor/select-mode3.png");
  m_move_mode->push_mode("images/engine/editor/move-mode1.png");
  m_undo_mode->push_mode("images/engine/editor/redo.png");
  //settings_mode->push_mode("images/engine/editor/settings-mode1.png");
}

void
EditorToolboxWidget::draw(DrawingContext& context)
{
  context.color().draw_filled_rect(Rectf(Vector(static_cast<float>(m_Xpos), 0),
                                         Vector(context.get_width(),
                                                context.get_height())),
                                     g_config->editorcolor,
                                     0.0f, LAYER_GUI-10);

  if (m_dragging)
  {
    context.color().draw_filled_rect(selection_draw_rect(), Color(0.2f, 0.4f, 1.0f, 0.6f),
                                       0.0f, LAYER_GUI+1);
  }

  if (m_hovered_item != HoveredItem::NONE && m_hovered_item != HoveredItem::SCROLLBAR)
  {
    m_object_tip->draw(context, Vector(m_mouse_pos.x + 25, m_mouse_pos.y), true);

    context.color().draw_filled_rect(get_item_rect(m_hovered_item),
                                       g_config->editorhovercolor,
                                       0.0f, LAYER_GUI - 5);
  }

  context.color().draw_text(Resources::normal_font, _("Tiles"),
                            Vector(context.get_width(), 5),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::default_color);
  context.color().draw_text(Resources::normal_font, _("Objects"),
                            Vector(context.get_width(), 37),
                            ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::default_color);

  m_rubber->draw(context);
  m_undo_mode->draw(context);
  switch (m_input_type)
  {
    case InputType::TILE:
      m_select_mode->draw(context);
      break;

    case InputType::OBJECT:
      m_node_marker_mode->draw(context);
      m_move_mode->draw(context);
      break;

    default:
      break;
  }

  const Rect original_clip = context.get_viewport();
  context.set_viewport(Rect(m_Xpos, m_Ypos, SCREEN_WIDTH, SCREEN_HEIGHT));
  switch (m_input_type)
  {
    case InputType::TILE:
      draw_tilegroup(context);
      break;

    case InputType::OBJECT:
      draw_objectgroup(context);
      break;

    default:
      break;
  }
  context.set_viewport(original_clip);

  if (m_hovered_item == HoveredItem::TILE || m_hovered_item == HoveredItem::SCROLLBAR)
    m_scrollbar->draw(context);
}

void
EditorToolboxWidget::draw_tilegroup(DrawingContext& context)
{
  int pos = -1;
  for (auto& tile_ID : m_active_tilegroup->tiles)
  {
    pos++;
    if (pos / 4 < static_cast<int>(m_scroll_progress / 32.f))
      continue;

    auto position = get_tile_coords(pos, false);
    draw_tile(context.color(), *m_editor.get_tileset(), tile_ID, position, LAYER_GUI - 9);

    if (g_config->developer_mode && m_active_tilegroup->developers_group)
    {
      // Display tile ID on top of tile:
      context.color().draw_text(Resources::console_font, std::to_string(tile_ID),
                                position + Vector(16, 16), ALIGN_CENTER, LAYER_GUI - 9, Color::WHITE);
    }
  }
}

void
EditorToolboxWidget::draw_objectgroup(DrawingContext& context)
{
  int pos = -1;
  for (auto& icon : m_object_info->m_groups[m_active_objectgroup].get_icons())
  {
    pos++;
    if (pos / 4 < static_cast<int>(m_scroll_progress / 32.f))
      continue;

    icon.draw(context, get_tile_coords(pos, false));
  }
}

Rectf
EditorToolboxWidget::normalize_selection(bool rounded) const
{
  const float tile_scroll_progress = (rounded ? floorf(m_scroll_progress / 32.f) : m_scroll_progress / 32.f);

  Vector drag_start = m_drag_start - Vector(0.f, tile_scroll_progress);
  Vector drag_end = Vector(static_cast<float>(m_hovered_tile % 4),
                           static_cast<float>(m_hovered_tile / 4) - tile_scroll_progress); // NOLINT
  if (drag_start.x > drag_end.x) {
    std::swap(drag_start.x, drag_end.x);
  }
  if (drag_start.y > drag_end.y) {
    std::swap(drag_start.y, drag_end.y);
  }
  return Rectf(drag_start, drag_end);
}

Rectf
EditorToolboxWidget::selection_draw_rect() const
{
  Rectf select = normalize_selection(false);
  select.set_p2(select.p2() + Vector(1, 1));
  select.set_p1((select.p1() * 32.0f) + Vector(static_cast<float>(m_Xpos), static_cast<float>(m_Ypos)));
  select.set_p2((select.p2() * 32.0f) + Vector(static_cast<float>(m_Xpos), static_cast<float>(m_Ypos)));

  if (select.get_top() < static_cast<float>(m_Ypos)) // Do not go over tool bar
    select.set_top(static_cast<float>(m_Ypos));

  return select;
}

void
EditorToolboxWidget::update_selection()
{
  Rectf select = normalize_selection(true);
  m_tiles->m_tiles.clear();
  m_tiles->m_width = static_cast<int>(select.get_width() + 1);
  m_tiles->m_height = static_cast<int>(select.get_height() + 1);

  int size = static_cast<int>(m_active_tilegroup->tiles.size());
  for (int y = static_cast<int>(select.get_top()); y <= static_cast<int>(select.get_bottom()); y++) {
    for (int x = static_cast<int>(select.get_left()); x <= static_cast<int>(select.get_right()); x++) {
      int tile_pos = (y + static_cast<int>(m_scroll_progress / 32.f)) * 4 + x;
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
  m_scrollbar->on_mouse_button_up(button);

  m_dragging = false;
  return false;
}

bool
EditorToolboxWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  if (m_scrollbar->on_mouse_button_down(button))
    return true;

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
          select_tilegroup(0);
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
              if (m_hovered_tile < size && m_hovered_tile >= 0) {
                m_tiles->set_tile(m_active_tilegroup->tiles[m_hovered_tile]);
              } else {
                m_tiles->set_tile(0);
              }
            }
            break;

          case InputType::OBJECT:
            {
              int size = static_cast<int>(m_object_info->m_groups[m_active_objectgroup].get_icons().size());
              if (m_hovered_tile < size && m_hovered_tile >= 0) {
                m_object = m_object_info->m_groups[m_active_objectgroup].get_icons()[m_hovered_tile].get_object_class();
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
            switch (m_input_type)
            {
              case InputType::TILE:
                m_select_mode->next_mode();
                break;
              case InputType::OBJECT:
                m_object = "#node";
                break;
              default:
                break;
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
  if (m_scrollbar->on_mouse_motion(motion))
  {
    m_hovered_item = HoveredItem::SCROLLBAR;
    m_object_tip->set_visible(false);
    return true;
  }

  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  float x = m_mouse_pos.x - static_cast<float>(m_Xpos);
  float y = m_mouse_pos.y - static_cast<float>(m_Ypos);

  if (x < 0)
  {
    m_hovered_item = HoveredItem::NONE;
    m_has_mouse_focus = false;
    m_object_tip->set_visible(false);
    return false;
  }

  // mouse is currently over the toolbox
  m_has_mouse_focus = true;

  if (y < 0) /** Tool bar */
  {
    if (y < -64) {
      m_hovered_item = HoveredItem::TILEGROUP;
    } else if (y < -32) {
      m_hovered_item = HoveredItem::OBJECTS;
    } else {
      m_hovered_item = HoveredItem::TOOL;
      m_hovered_tile = get_tool_pos(m_mouse_pos);
    }
    m_object_tip->set_visible(false);
    return false;
  }
  else /** Tile/object bar */
  {
    update_hovered_tile();
  }

  return false;
}

bool
EditorToolboxWidget::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (m_hovered_item == HoveredItem::TILE)
  {
    m_scrollbar->on_mouse_wheel(wheel);
    update_hovered_tile();
  }

  return false;
}

void
EditorToolboxWidget::update_hovered_tile()
{
  const int prev_hovered_tile = std::move(m_hovered_tile);
  m_hovered_item = HoveredItem::TILE;
  m_hovered_tile = get_tile_pos(m_mouse_pos);
  if (m_dragging && m_input_type == InputType::TILE) {
    update_selection();
  }
  else if (m_input_type == InputType::OBJECT && m_hovered_tile != prev_hovered_tile) {
    const auto& icons = m_object_info->m_groups[m_active_objectgroup].get_icons();
    if (m_hovered_tile < static_cast<int>(icons.size())) {
      const std::string obj_class = icons[m_hovered_tile].get_object_class();
      std::string obj_name = obj_class;
      try {
        obj_name = GameObjectFactory::instance().get_display_name(obj_class);
      }
      catch (const std::exception& err) {
        log_warning << "Unable to get display name of object '" << obj_class << "': " << err.what() << std::endl;
      }
      m_object_tip->set_info(obj_name);
    }
    else {
      m_object_tip->set_visible(false);
    }
  }
}

void
EditorToolboxWidget::resize()
{
  m_Xpos = SCREEN_WIDTH - 128;
  m_rubber->m_pos           = Vector(static_cast<float>(m_Xpos)        , 64.0f);
  m_select_mode->m_pos      = Vector(static_cast<float>(m_Xpos) + 32.0f, 64.0f);
  m_node_marker_mode->m_pos = Vector(static_cast<float>(m_Xpos) + 32.0f, 64.0f);
  m_move_mode->m_pos        = Vector(static_cast<float>(m_Xpos) + 64.0f, 64.0f);
  m_undo_mode->m_pos        = Vector(static_cast<float>(m_Xpos) + 96.0f, 64.0f);

  const Vector screen_size(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));
  m_scrollbar->set_covered_region(screen_size.y - static_cast<float>(m_Ypos));
  m_scrollbar->set_total_region(get_total_scrollbar_region());
  m_scrollbar->set_rect(Rectf(Vector(screen_size.x - 5.f, m_Ypos), screen_size));
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
        if (m_object == "#node")
          MouseCursor::current()->set_icon(m_node_marker_mode->get_current_surface());
        else
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
EditorToolboxWidget::get_tile_coords(const int pos, bool relative) const
{
  return Vector(static_cast<float>((pos % 4) * 32 + (relative ? m_Xpos : 0)),
                static_cast<float>((pos / 4) * 32 + (relative ? m_Ypos : 0)) - m_scroll_progress);
}

int
EditorToolboxWidget::get_tile_pos(const Vector& coords) const
{
  int x = static_cast<int>((coords.x - static_cast<float>(m_Xpos)) / 32.0f);
  int y = static_cast<int>((coords.y - static_cast<float>(m_Ypos) + m_scroll_progress) / 32.0f);
  return y * 4 + x;
}

Vector
EditorToolboxWidget::get_tool_coords(const int pos) const
{
  return Vector(static_cast<float>((pos % 4) * 32 + m_Xpos),
                static_cast<float>((pos / 4) * 32 + 64));
}

int
EditorToolboxWidget::get_tool_pos(const Vector& coords) const
{
  int x = static_cast<int>((coords.x - static_cast<float>(m_Xpos)) / 32.0f);
  int y = static_cast<int>((coords.y - 64.0f) / 32.0f);
  return y * 4 + x;
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
      Rectf rect(coords, coords + Vector(32, 32));

      if (rect.get_top() < static_cast<float>(m_Ypos)) // Do not go over tool bar
        rect.set_top(static_cast<float>(m_Ypos));

      return rect;
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
  m_input_type = InputType::TILE;
  reset_scrollbar();
  update_mouse_icon();
}

void
EditorToolboxWidget::select_objectgroup(int id)
{
  m_active_objectgroup = id;
  m_input_type = InputType::OBJECT;
  reset_scrollbar();
  update_mouse_icon();
}

float
EditorToolboxWidget::get_total_scrollbar_region() const
{
  switch (m_input_type)
  {
    case InputType::TILE:
      return ceilf(static_cast<float>(m_active_tilegroup->tiles.size()) / 4.f) * 32.f;

    case InputType::OBJECT:
      return ceilf(static_cast<float>(m_object_info->m_groups[m_active_objectgroup].get_icons().size()) / 4.f) * 32.f;

    default:
      return 1.f;
  }
}

void
EditorToolboxWidget::reset_scrollbar()
{
  m_scroll_progress = 0.f;
  m_scrollbar->set_total_region(get_total_scrollbar_region());
}

bool
EditorToolboxWidget::has_mouse_focus() const
{
  return m_has_mouse_focus;
}

/* EOF */
