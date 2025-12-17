//  SuperTux
//  Copyright (C) 2025 Hyland B. <me@ow.swag.toys>
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

#include "supertux/menu/editor_temp_save_as.hpp"

#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/notification.hpp"
#include "supertux/level.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "video/compositor.hpp"

EditorTempSaveAs::EditorTempSaveAs(std::unique_ptr<World> world) :
  m_world(std::move(world)),
  m_file_name()
{
  Level* level = Editor::current()->get_level();
  add_label(_("Save Level as"));

  add_hl();

  add_textfield(_("Level name"), &(level->m_name));
  add_textfield(_("Author"), &(level->m_author));
  add_textfield(_("License"), &(level->m_license));

  add_hl();
  add_entry(MNID_SAVE, _("Save"));
  add_back(_("Cancel"));

  std::string dir;
  int num = 0;
  do {
    num++;
    m_file_name = "level" + std::to_string(num) + ".stl";
    dir = m_world->get_basedir() + "/" + m_file_name;
  } while ( PHYSFS_exists(dir.c_str()) );
}

EditorTempSaveAs::~EditorTempSaveAs()
{
  auto editor = Editor::current();
  if (editor == nullptr) {
    return;
  }
  editor->m_reactivate_request = true;
}

void
EditorTempSaveAs::menu_action(MenuItem& item)
{
  auto editor = Editor::current();

  switch (item.get_id())
  {
    case MNID_SAVE:
    {
      Level* level = editor->get_level();

      if (level->m_name.empty())
      {
        Dialog::show_message(_("Please enter a name for this level."));
        return;
      }

      // post_save will get implicitly called here
      editor->m_save_request = true;
      editor->m_save_request_filename = m_file_name;
      editor->m_save_temp_level = true;

      editor->set_world(std::move(std::unique_ptr<World>(m_world.release())));

      auto notif = std::make_unique<Notification>("create_level_notif", 5.f);
      notif->set_text(_("Level created!"));
      MenuManager::instance().set_notification(std::move(notif));

      MenuManager::instance().clear_menu_stack();
    }
      break;

    default:
      break;
  }
}
