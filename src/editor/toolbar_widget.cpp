//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2024 Vankata453
//                2025 Tobias Markus <tobbi.bugs@gmail.com>
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

#include "editor/toolbar_widget.hpp"

#include "editor/button_widget.hpp"
#include "editor/editor.hpp"
#include "editor/tilebox.hpp"
#include "editor/tool_icon.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "gui/menu.hpp"
#include "gui/notification.hpp"
#include "math/util.hpp"
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

using InputType = EditorTilebox::InputType;

EditorToolbarWidget::EditorToolbarWidget(Editor& editor) :
  m_editor(editor),
  m_undo_widget(),
  m_redo_widget(),
  m_widgets(),
  m_widgets_width(0.f),
  m_widgets_width_offset(0.f)
{
    std::array<std::unique_ptr<EditorToolbarButtonWidget>, 8> general_widgets = {
    // Undo button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/undo.png",
        std::bind(&Editor::undo, Editor::current()),
        _("Undo"),
        Sizef(32.f, 32.f)),

    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/redo.png",
        std::bind(&Editor::redo, Editor::current()),
        _("Redo"),
        Sizef(32.f, 32.f)),

    // Grid button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/grid_button.png",
      [this] {
        auto& snap_grid_size = g_config->editor_selected_snap_grid_size;
        if (snap_grid_size == 0)
        {
          if(!g_config->editor_render_grid)
          {
            snap_grid_size = 3;
          }
          g_config->editor_render_grid = !g_config->editor_render_grid;
        }
        else
          snap_grid_size--;
      },
      _("Change / Toggle grid size")),

    // Play button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/play_button.png",
      [this] { Editor::current()->m_test_request = true; },
      _("Test level")),

    // Save button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/save.png",
      [this] {
        Editor::current()->save_level();
      },
      _("Save level")),

    // Mode button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/toggle_tile_object_mode.png",
      std::bind(&EditorToolbarWidget::toggle_tile_object_mode, this),
      _("Toggle between object and tile mode")),

    // Mouse select button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/arrow.png",
      [this]() {
        Editor::current()->get_toolbox_widget()->set_mouse_tool();
      },
      _("Select or move the object under the mouse")),

    // Rubber button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/rubber.png",
      [this]() {
        Editor::current()->get_toolbox_widget()->set_rubber_tool();
      },
      _("Delete the tile or object under the mouse"))
  };

  std::array<std::unique_ptr<EditorToolbarButtonWidget>, 4> tile_mode_widgets = {
    // Select mode mouse
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/select-mode0.png",
    [this] {
      Editor::current()->get_toolbox_widget()->set_tileselect_select_mode(0);
    },
    _("Draw mode")),

    // Select mode area
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/select-mode1.png",
      [this] {
        Editor::current()->get_toolbox_widget()->set_tileselect_select_mode(1);
      },
      _("Box draw mode")),

    // Select mode fill button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/select-mode2.png",
      [this] {
        Editor::current()->get_toolbox_widget()->set_tileselect_select_mode(2);
      },
      _("Fill mode")),

    // Select mode same button
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/select-mode3.png",
      [this] {
        Editor::current()->get_toolbox_widget()->set_tileselect_select_mode(3);
      },
      _("Replace mode")),
  };

  std::array<std::unique_ptr<EditorToolbarButtonWidget>, 3> object_mode_widgets = {
    // Path edit mode
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/path_node.png",
      [this] {
        Editor::current()->get_tilebox().set_object("#node");
      },
      _("Path edit mode (Clicking adds path nodes to the selected object if it supports them)")),

    // Select mode
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/move-mode0.png",
      [this] {
        Editor::current()->get_toolbox_widget()->set_tileselect_move_mode(0);
      },
      _("Select mode (Clicking selects the object under the mouse)")),

    // Duplicate mode
    std::make_unique<EditorToolbarButtonWidget>("images/engine/editor/move-mode1.png",
      [this] {
        Editor::current()->get_toolbox_widget()->set_tileselect_move_mode(1);
      },
      _("Duplicate mode (Clicking duplicates the object under the mouse)")),
  };

  size_t i = 0;
  for (auto &widget : general_widgets)
  {
    Vector pos(32 * i, 0);
    widget->set_position(pos);
    widget->set_flat(true);
    m_widgets.insert(m_widgets.begin() + i, std::move(widget));
    ++i;
  }

  for (auto &widget : tile_mode_widgets)
  {
    Vector pos(32 * i, 0);
    widget->set_position(pos);
    widget->set_flat(true);
    widget->set_visible_in_object_mode(false);
    widget->set_visible(false);
    m_widgets.insert(m_widgets.begin() + i, std::move(widget));
    ++i;
  }

  for (auto &widget : object_mode_widgets)
  {
    Vector pos(32 * (i - tile_mode_widgets.size()), 0);
    widget->set_position(pos);
    widget->set_flat(true);
    widget->set_visible_in_tile_mode(false);
    widget->set_visible(false);
    m_widgets.insert(m_widgets.begin() + i, std::move(widget));
    ++i;
  }
  m_widgets_width = 32.f * (i - std::max(tile_mode_widgets.size(), object_mode_widgets.size()) - 3);

  m_undo_widget = m_widgets[0].get();
  m_redo_widget = m_widgets[1].get();
  m_undo_widget->set_disabled(true);
  m_redo_widget->set_disabled(true);

  // auto code_widget = std::make_unique<EditorToolbarButtonWidget>(
  //   "images/engine/editor/select-mode3.png", Vector(320, 0), [this] {
  //     std::ostringstream level_ostream;
  //     Writer output_writer(level_ostream);
  //     m_level->save(output_writer);
  //     auto level_content = level_ostream.str();
  //     MenuManager::instance().push_menu(std::make_unique<ScriptMenu>(&level_content));
  //     log_warning << level_content << std::endl;
  //   });
  // m_widgets.insert(m_widgets.begin() + 10, std::move(code_widget));
}

void
EditorToolbarWidget::toggle_tile_object_mode()
{
  int i = 0;
  auto& tilebox = Editor::current()->get_toolbox_widget()->get_tilebox();
  const auto& input_type = tilebox.get_input_type();

  if (input_type == InputType::OBJECT) // Object mode -> Tile mode
  {
    Editor::current()->select_last_tilegroup();
    for(const auto& toolbar_button : m_widgets)
    {
      toolbar_button->set_visible(toolbar_button->get_visible_in_tile_mode());
    }
    Editor::current()->get_toolbox_widget()->set_tileselect_select_mode(0);
  }
  else // Tile mode -> Object mode
  {
    Editor::current()->select_last_objectgroup();
    for(const auto& toolbar_button : m_widgets)
    {
      toolbar_button->set_visible(toolbar_button->get_visible_in_object_mode());
  	}
    Editor::current()->get_toolbox_widget()->set_tileselect_move_mode(0);
  }

  for (const auto& toolbar_button : m_widgets)
  {
    if (toolbar_button->get_visible())
      ++i;
	}

  m_widgets_width = i * 32.f;
}

bool
EditorToolbarWidget::event(const SDL_Event& ev)
{
  for(const auto& widget : m_widgets)
  {
    if (widget->event(ev))
      return true;
  }
  return false;
}

void
EditorToolbarWidget::update(float dt_sec)
{
  for(const auto& widget : m_widgets)
  {
    widget->update(dt_sec);
  }
}

void
EditorToolbarWidget::draw(DrawingContext& context)
{
  if (!g_config->editor_show_toolbar_widgets)
    return;

  context.color().set_blur(g_config->editor_blur);
  context.color().draw_filled_rect(
    {-g_config->menuroundness, -g_config->menuroundness, m_widgets_width + m_widgets_width_offset, 32},
    Color(0.2f, 0.2f, 0.2f, 0.5f),
    math::clamp(g_config->menuroundness, 0.f, 16.f),
    LAYER_GUI - 5);
  context.color().set_blur(0);

  for(const auto& widget : m_widgets)
  {
    widget->draw(context);
  }
}

void
EditorToolbarWidget::set_undo_disabled(bool state)
{
  if (m_undo_widget)
    m_undo_widget->set_disabled(state);
}

void
EditorToolbarWidget::set_redo_disabled(bool state)
{
  if (m_redo_widget)
    m_redo_widget->set_disabled(state);
}
