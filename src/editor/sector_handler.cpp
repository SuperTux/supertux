//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "editor/sector_handler.hpp"

#include "editor/editor.hpp"
#include "supertux/game_object_change.hpp"
#include "util/writer.hpp"

EditorSectorHandler::EditorSectorHandler()
{
}

bool
EditorSectorHandler::should_update_object(const GameObject& object)
{
  // Newly placed objects in the editor should be updated to their latest version.
  return Editor::is_active() && !object.get_uid();
}

bool
EditorSectorHandler::before_object_add(GameObject& object)
{
  // Attempt to add the new object to the layers widget.
  Editor::current()->add_layer(object);

  return true;
}

void
EditorSectorHandler::on_object_changes(const GameObjectChanges& changes)
{
  std::ostringstream stream;
  Writer writer(stream);

  changes.save(writer);
  log_warning << stream.str() << std::endl;
}

/* EOF */
