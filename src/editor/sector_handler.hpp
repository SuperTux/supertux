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

#ifndef HEADER_SUPERTUX_EDITOR_SECTOR_HANDLER_HPP
#define HEADER_SUPERTUX_EDITOR_SECTOR_HANDLER_HPP

#include "supertux/game_object_manager.hpp"

class Sector;

/** Handles various events on level sectors in editor. */
class EditorSectorHandler final : public GameObjectManager::EventHandler
{
public:
  EditorSectorHandler();

  virtual bool should_update_object(const GameObject& object) override;

  virtual bool before_object_add(GameObject& object) override;

  virtual void on_object_changes(const GameObjectChanges& changes) override;

private:
  EditorSectorHandler(const EditorSectorHandler&) = delete;
  EditorSectorHandler& operator=(const EditorSectorHandler&) = delete;
};

#endif

/* EOF */
