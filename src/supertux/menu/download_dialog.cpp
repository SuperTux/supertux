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

#ifndef __EMSCRIPTEN__

#include "supertux/menu/download_dialog.hpp"

#include "addon/addon_manager.hpp"

DownloadDialog::DownloadDialog(TransferStatusPtr status, bool auto_close, bool passive) :
  Dialog(passive),
  m_status(std::move(status)),
  m_title(),
  m_auto_close(auto_close)
{
  add_default_button(_("Abort Download"), [this]{
      on_abort();
    });

  update_text();

  m_status->then(
    [this](bool success)
    {
      if (success)
      {
        on_download_complete();
      }
      else
      {
        Dialog::show_message(_("Error:\n") + m_status->error_msg);
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
  if (m_auto_close)
  {
    MenuManager::instance().set_dialog({});
    return;
  }

  clear_buttons();
  add_button(_("Close"), [] {
      MenuManager::instance().set_dialog({});
    });
}

#endif

/* EOF */
