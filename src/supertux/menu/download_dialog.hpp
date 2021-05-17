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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_DOWNLOAD_DIALOG_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_DOWNLOAD_DIALOG_HPP

#ifndef __EMSCRIPTEN__

#include "gui/dialog.hpp"

class TransferStatus;
using TransferStatusPtr = std::shared_ptr<TransferStatus>;

class DownloadDialog final : public Dialog
{
private:
  TransferStatusPtr m_status;
  std::string m_title;
  bool m_auto_close;

public:
  DownloadDialog(TransferStatusPtr status, bool auto_close = false, bool passive = false);

  void set_title(const std::string& title);
  void update() override;

private:
  void on_abort();
  void on_download_complete();

  void update_text();

private:
  DownloadDialog(const DownloadDialog&) = delete;
  DownloadDialog& operator=(const DownloadDialog&) = delete;
};

#endif

#endif

/* EOF */
