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
#include "editor/network_protocol.hpp"
#include "network/host.hpp"
#include "util/writer.hpp"

EditorSectorHandler::EditorSectorHandler(Editor& editor, Sector& sector) :
  m_editor(editor),
  m_sector(sector)
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
  m_editor.add_layer(object);

  return true;
}

void
EditorSectorHandler::on_object_changes(const GameObjectStates& changes)
{
  broadcast_sector_changes([this, changes](Writer& writer)
    {
      writer.write("sector", m_sector.get_name());
      writer.start_list("object-changes");

      GameObjectStates states = changes;
      for (GameObjectState& state : states.objects)
      {
        switch (state.action)
        {
          case GameObjectState::Action::MODIFY: // Need to send current object data
            state.data = m_sector.get_object_by_uid<GameObject>(state.uid)->save();
            break;

          case GameObjectState::Action::DELETE: // No need to send data, just the action is enough
            state.data.clear();
            break;

          default:
            break;
        }
      }
      states.save(writer);

      writer.end_list("object-changes");
    });
}

void
EditorSectorHandler::on_property_changes(const std::string& original_name)
{
  broadcast_sector_changes([this, original_name](Writer& writer)
    {
      writer.write("sector", original_name);
      m_sector.save_properties(writer);
    });
}


void
EditorSectorHandler::broadcast_sector_changes(const std::function<void(Writer&)>& write_func) const
{
  // This function is needed for remote level editing.
  // Do not proceed if no networking is active.
  if (!m_editor.get_network_host())
    return;

  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-sector-changes");
  write_func(writer);
  writer.end_list("supertux-sector-changes");

  network::StagedPacket packet(EditorNetworkProtocol::OP_SECTOR_CHANGES, stream.str(), 5.f);
  m_editor.get_network_host()->broadcast_packet(packet, true);
}

/* EOF */
