//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/download_dialog.hpp"

#include <sstream>

#include "gui/menu_manager.hpp"
#include "util/gettext.hpp"

DownloadDialog::DownloadDialog(TransferStatusPtr status) :
  m_status(status),
  m_title()
{
  add_default_button(_("Abort Download"), [this]{
      on_abort();
    });

  update_text();

  status->then(
    [this](bool success)
    {
      if (success)
      {
        on_download_complete();
      }
      else
      {
        std::unique_ptr<Dialog> dialog(new Dialog);
        dialog->set_text(_("Error:\n") + m_status->error_msg);
        dialog->add_button(_("Ok"));
        MenuManager::instance().set_dialog(std::move(dialog));
      }
    });
}

void
DownloadDialog::update()
{
  m_status->update();

  update_text();
}

void
DownloadDialog::set_title(const std::string& title)
{
  m_title = title;
}

void
DownloadDialog::update_text()
{
  std::ostringstream out;
  out << m_title << "\n";

  if (m_status->dltotal == 0)
  {
    out << "---\n---";
  }
  else
  {
    int percent = 100 * m_status->dlnow / m_status->dltotal;
    out << m_status->dlnow/1000 << "/" << m_status->dltotal/1000 << " kB\n" << percent << "%";
  }

  set_text(out.str());
}

void
DownloadDialog::on_abort()
{
  m_status->abort();
}

void
DownloadDialog::on_download_complete()
{
  clear_buttons();
  add_button(_("Close"), [this]{
      MenuManager::instance().set_dialog({});
    });
}

/* EOF */
