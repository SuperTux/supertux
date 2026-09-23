//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "editor/editor_history_manager.hpp"

#include "editor/editor.hpp"
#include "supertux/gameconfig.hpp"

void
EditorHistoryManager::retoggle_undo_tracking()
{
  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto level = editor_project->get_level();
  auto toolbar_widget = editor->get_toolbar_widget();

  toolbar_widget->set_undo_disabled(true);
  toolbar_widget->set_redo_disabled(true);

  // Toggle undo tracking for all sectors.
  for (const auto& sector : level->get_sectors())
    sector->toggle_undo_tracking(g_config->editor_undo_tracking);
}

void
EditorHistoryManager::undo_stack_cleanup()
{
  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto level = editor_project->get_level();

  // Set the undo stack size and perform undo stack cleanup on all sectors.
  for (const auto& sector : level->get_sectors())
  {
    sector->set_undo_stack_size(g_config->editor_undo_stack_size);
    sector->undo_stack_cleanup();
  }
}

void
EditorHistoryManager::undo()
{
  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto sector = editor_project->get_sector();
  auto layers_widget = editor->get_layers_widget();

  BIND_SECTOR(*sector);
  sector->undo();
  layers_widget->update_current_tip();
}

void
EditorHistoryManager::redo()
{
  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto sector = editor_project->get_sector();
  auto layers_widget = editor->get_layers_widget();

  BIND_SECTOR(*sector);
  sector->redo();
  layers_widget->update_current_tip();
}