//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2024 Vankata453
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
#include "editor/menu_list_item.hpp"
#include "editor/menu_list_widget.hpp"
#include "editor/node_marker.hpp"
#include "editor/toolbar_widget.hpp"
#include "editor/tilebox.hpp"
#include "editor/object_info.hpp"
#include "editor/tool_icon.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "gui/menu.hpp"
#include "interface/control_enum.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

using InputMode = Editor::InputMode;

EditorToolboxWidget::EditorToolboxWidget(Editor& editor) :
  m_editor(editor),
  m_tilebox(new EditorTilebox(editor, Rectf())),
  m_pos_x(0.f),
  m_hovered_item(HoveredItem::NONE),
  m_hovered_tool(-1),
  m_rubber(new ToolIcon("images/engine/editor/rubber.png")),
  m_select_mode(new ToolIcon("images/engine/editor/select-mode0.png")),
  m_node_marker_mode(new ToolIcon("images/engine/editor/path_node.png")),
  m_move_mode(new ToolIcon("images/engine/editor/move-mode0.png")),
  m_tile_groups_dropdown(std::make_unique<ControlEnum<int>>()),
  m_object_groups_dropdown(std::make_unique<ControlEnum<int>>()),
  m_brushes(),
  m_brushes_list_widget(std::make_unique<MenuListWidget>()),
  m_has_mouse_focus(false)
{
  m_select_mode->push_mode("images/engine/editor/select-mode1.png");
  m_select_mode->push_mode("images/engine/editor/select-mode2.png");
  m_select_mode->push_mode("images/engine/editor/select-mode3.png");
  m_move_mode->push_mode("images/engine/editor/move-mode1.png");
  //settings_mode->push_mode("images/engine/editor/settings-mode1.png");

  auto prepare_dropdowns = [this]()
  {
    prepare_tile_group_dropdown();
    prepare_object_group_dropdown();
    load_brushes();
  };

  auto editor_project = Editor::current()->get_project();
  editor_project->add_sector_set_callback(prepare_dropdowns);
  editor_project->add_tileset_reloaded_callback(prepare_dropdowns);

  set_mouse_tool();
}

void
EditorToolboxWidget::prepare_tile_group_dropdown()
{
  auto editor_project = Editor::current()->get_project();
  m_tile_groups_dropdown->clear_items();

  int i = 0;
  for (const auto &tile_group : editor_project->get_tile_groups())
  {
    m_tile_groups_dropdown->add_option(i, tile_group.name);
    i++;
  }
  m_tile_groups_dropdown->bind_value(&(m_tilebox->m_tile_group_id));
  auto on_tile_group_select = [this]
  {
    select_tile_group(m_tilebox->m_tile_group_id);
  };
  m_tile_groups_dropdown->m_on_change_callbacks.emplace_back(on_tile_group_select);
}

void
EditorToolboxWidget::prepare_object_group_dropdown()
{
  auto editor_project = Editor::current()->get_project();
  auto level = editor_project->get_level();
  bool is_worldmap = level->is_worldmap();

  m_object_groups_dropdown->clear_items();
  
  int i = 0;
  for (const auto &object_group : get_tilebox().get_object_info().m_groups)
  {
    if (object_group.is_worldmap() == is_worldmap)
    {
      m_object_groups_dropdown->add_option(i, object_group.get_name());
      i++;
    }
  }
  m_object_groups_dropdown->bind_value(&(m_tilebox->m_object_group_id));
  auto on_object_group_select = [this]
  {
    select_object_group(m_tilebox->m_object_group_id);
  };
  m_object_groups_dropdown->m_on_change_callbacks.emplace_back(on_object_group_select);
}

void
EditorToolboxWidget::load_brushes()
{
  auto test_selection = std::make_unique<TileSelection>();
  test_selection->m_height = 4;
  test_selection->m_width = 4;
  test_selection->m_tiles = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
  m_brushes["Test Brush"] = std::move(test_selection);

  for(const auto& brush : m_brushes)
  {
    auto on_brush_selected = [&brush, &test_selection, this]
    {
      *(m_editor.get_tile_selection()) = *(brush.second.get());
    };
    auto brush_item = std::make_unique<MenuListItem>(brush.first, on_brush_selected);

    auto is_brush_selected = [&brush, this]
    {
      return *(m_editor.get_tile_selection()) == *(brush.second.get());
    };
    brush_item->set_is_selected_handler(is_brush_selected);

    m_brushes_list_widget->add_item(std::move(brush_item));
  }
}

void
EditorToolboxWidget::draw(DrawingContext& context)
{
  draw_separator(context);

  if (m_editor.get_input_mode() == InputMode::TILE)
  {
    m_tile_groups_dropdown->draw(context);
    m_brushes_list_widget->draw(context);
  }
  else
  {
    m_object_groups_dropdown->draw(context);
    context.color().draw_filled_rect(m_brushes_list_widget->get_area(), g_config->editorcolor, LAYER_GUI);
  }

  m_tilebox->draw(context);

  context.color().set_blur(g_config->editor_blur);
  context.color().draw_filled_rect(Rectf(Vector(m_pos_x, 0.f),
                                         Vector(context.get_width(), 96.f)),
                                   g_config->editorcolor,
                                   0.0f, LAYER_GUI-10);
  context.color().set_blur(0);
}

void
EditorToolboxWidget::draw_separator(DrawingContext& context)
{
  constexpr float LINE_THICKNESS = 1.f;
  Rectf border_rect = Rectf{SCREEN_WIDTH - 128.f - LINE_THICKNESS, 0,
                            SCREEN_WIDTH - 128.f, static_cast<float>(SCREEN_HEIGHT - 32.f)};
  Color line_color = (g_config->editorcolor - Color(0.2, 0.2, 0.2, 0.2)).validate();
  context.color().draw_filled_rect(border_rect, line_color, LAYER_GUI + 1);

  Rectf shadow_rect = border_rect;
  shadow_rect.set_left(border_rect.get_left() - 16 + LINE_THICKNESS);
  shadow_rect.set_right(border_rect.get_right() - LINE_THICKNESS);

  auto start_color = Color(0.0f, 0.0f, 0.0f, 0.0f);
  auto end_color = Color(0.0f, 0.0f, 0.0f, 0.2f);
  context.color().draw_gradient(start_color, end_color, LAYER_GUI + 1,
                                GradientDirection::HORIZONTAL, shadow_rect);
}

bool
EditorToolboxWidget::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  if (m_editor.get_input_mode() == InputMode::TILE &&
      m_tile_groups_dropdown->on_mouse_button_up(button))
  {
    return true;
  }
  
  if (m_editor.get_input_mode() == InputMode::OBJECT &&
      m_object_groups_dropdown->on_mouse_button_up(button))
  {
    return true;
  }

  if (m_editor.get_input_mode() == InputMode::TILE &&
      m_brushes_list_widget->on_mouse_button_up(button))
  {
    return true;
  }

  return m_tilebox->on_mouse_button_up(button);
}

bool
EditorToolboxWidget::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  auto input_mode = m_editor.get_input_mode();
  if (input_mode == InputMode::TILE &&
      m_tile_groups_dropdown->on_mouse_button_down(button))
  {
    return true;
  }

  if (input_mode == InputMode::OBJECT &&
      m_object_groups_dropdown->on_mouse_button_down(button))
  {
    return true;
  }

  if (m_tilebox->on_mouse_button_down(button) ||
      (input_mode == InputMode::TILE && m_brushes_list_widget->on_mouse_button_down(button)))
  {
    if (input_mode == InputMode::TILE)
    {
      m_editor.get_overlay_widget()->update_autotileset();
    }
    update_mouse_icon();
    return true;
  }

  return false;
}

void
EditorToolboxWidget::set_rubber_tool()
{
  m_editor.set_selected_object_class_name("");
  m_tilebox->get_tiles()->set_tile(0);
  m_editor.get_overlay_widget()->update_autotileset();
  update_mouse_icon();
}

void
EditorToolboxWidget::set_mouse_tool()
{
  m_editor.set_selected_object_class_name("#move");
  update_mouse_icon();
}

bool
EditorToolboxWidget::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  if (m_editor.get_input_mode() == InputMode::TILE &&
      m_tile_groups_dropdown->on_mouse_motion(motion))
  {
    return true;
  }

  if (m_editor.get_input_mode() == InputMode::OBJECT &&
      m_object_groups_dropdown->on_mouse_motion(motion))
  {
    return true;
  }

  if (m_editor.get_input_mode() == InputMode::TILE &&
      m_brushes_list_widget->on_mouse_motion(motion))
  {
    return true;
  }

  if (m_tilebox->on_mouse_motion(motion))
  {
    m_has_mouse_focus = true;
    return false; // The layers widget wouldn't be able to scroll otherwise.
  }

  const Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  const float relative_x = mouse_pos.x - m_pos_x;
  m_has_mouse_focus = relative_x > 0;

  return false;
}

void
EditorToolboxWidget::update_last_active_group()
{
  switch (m_hovered_item)
  {
    case HoveredItem::TILEGROUP:
      m_last_active_group = HoveredItem::TILEGROUP;
      return;
    case HoveredItem::OBJECTS:
      m_last_active_group = HoveredItem::OBJECTS;
      return;
    default:
      break;
  }

  if (m_last_active_group != HoveredItem::TILEGROUP &&
      m_last_active_group != HoveredItem::OBJECTS)
  {
    m_last_active_group = HoveredItem::TILEGROUP;
  }
}

void
EditorToolboxWidget::switch_current_group(int dir)
{
  update_last_active_group();
  auto editor_project = m_editor.get_project();
  auto level = editor_project->get_level();

  switch (m_last_active_group)
  {
    case HoveredItem::TILEGROUP:
      if (editor_project->get_tileset()->get_tilegroups().size() > 1)
      {
        m_tilebox->change_tile_group(dir);
      }
      else
      {
        select_tile_group(0);
      }
      break;

    case HoveredItem::OBJECTS:
      if ((level->is_worldmap() && m_tilebox->get_object_info().get_num_worldmap_groups() > 1) ||
          (!level->is_worldmap() && m_tilebox->get_object_info().get_num_level_groups() > 1))
      {
        m_tilebox->change_object_group(dir);
      }
      else
      {
        if (level->is_worldmap())
          select_object_group(m_tilebox->get_object_info().get_first_worldmap_group_index());
        else
          select_object_group(0);
      }
      break;

    default:
      break;
  }
}

bool
EditorToolboxWidget::on_mouse_wheel(const SDL_MouseWheelEvent& wheel)
{
  if (m_hovered_item == HoveredItem::TILEGROUP ||
      m_hovered_item == HoveredItem::OBJECTS)
    switch_current_group(wheel.y > 0 ? -1 : 1);

  return m_tilebox->on_mouse_wheel(wheel);
}

void
EditorToolboxWidget::on_window_resize()
{
  m_pos_x = static_cast<float>(SCREEN_WIDTH - 128);
  m_brushes_list_widget->set_area(Rectf(Vector(m_pos_x, SCREEN_HEIGHT * .6f),
                                        Vector(SCREEN_WIDTH * 1.f, SCREEN_HEIGHT)));
  m_brushes_list_widget->recalculate_item_position_and_size();
  m_brushes_list_widget->set_area(Rectf(Vector(m_pos_x, SCREEN_HEIGHT * .6f),
                                        Vector(SCREEN_WIDTH * 1.f, SCREEN_HEIGHT)));

  m_tilebox->set_rect(Rectf(Vector(m_pos_x, 32.f),
                            Vector(SCREEN_WIDTH * 1.f, SCREEN_HEIGHT * .6f)));

  m_tilebox->on_window_resize();

  auto dropdown_selector_rect = Rectf(m_pos_x, 6.f, SCREEN_WIDTH, 26.f);
  m_tile_groups_dropdown->set_rect(dropdown_selector_rect);
  m_object_groups_dropdown->set_rect(dropdown_selector_rect);
}

void
EditorToolboxWidget::setup()
{
  m_tilebox->setup();

  on_window_resize();
  m_tilebox->get_tiles()->set_tile(0);
}

void
EditorToolboxWidget::select_tile_group(int id)
{
  m_editor.set_input_mode(InputMode::TILE);
  m_tilebox->select_tile_group(id);
  update_mouse_icon();
}

void
EditorToolboxWidget::select_object_group(int id)
{
  m_editor.set_input_mode(InputMode::OBJECT);
  m_tilebox->select_object_group(id);
  update_mouse_icon();
}

void
EditorToolboxWidget::select_last_tile_group()
{
  m_tilebox->select_last_tile_group();
}

void
EditorToolboxWidget::select_last_object_group()
{
  m_tilebox->select_last_object_group();
}

int
EditorToolboxWidget::get_tileselect_select_mode() const
{
  return m_select_mode->get_mode();
}

void
EditorToolboxWidget::set_tileselect_select_mode(int mode)
{
  m_select_mode->set_mode(mode);
  update_mouse_icon();
}

int
EditorToolboxWidget::get_object_select_mode() const
{
  return m_move_mode->get_mode();
}

void
EditorToolboxWidget::set_tileselect_move_mode(int mode)
{
  m_move_mode->set_mode(mode);
  update_mouse_icon();
}

void
EditorToolboxWidget::update_mouse_icon()
{
  MouseCursor::current()->set_icon(get_mouse_icon()->get_current_surface());
}

ToolIcon*
EditorToolboxWidget::get_mouse_icon() const
{
  switch (m_editor.get_input_mode())
  {
    case InputMode::NONE:
    case InputMode::OBJECT:
    {
      const std::string& object_class_name = m_editor.get_selected_object_class_name();
      const auto &node_marker_class_name = NodeMarker::class_name();

      if (object_class_name.empty())
        return m_rubber.get();

      if (object_class_name == node_marker_class_name)
        return m_node_marker_mode.get();

      return m_move_mode.get();
    }

    case InputMode::TILE:
    {
      if (m_editor.get_tile_selection()->erases_tiles())
        return m_rubber.get();
      
      return m_select_mode.get();
    }

    default:
      return nullptr;
  }
}
