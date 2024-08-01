//  SuperTux
//  Copyright (C) 2024 James De Ricco <james.dericco.dev@gmail.com>
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

#include "gui/menu_filesystem_item_processor_music_help.hpp"

#include "fmt/format.h"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

void item_processor_music_help(MenuItem& menu_item, const std::string& file_path, bool in_basedir) {
    std::unique_ptr<SoundFile> sound_file;
    try {
      sound_file = load_sound_file(file_path);
    } catch (...) {
      menu_item.set_help("");
      return;
    }

    const std::vector<std::string>& authors = sound_file->m_authors;
    const std::string& license = sound_file->m_license;
    const std::string& title = sound_file->m_title;

    if (title.empty() && authors.empty() && license.empty()) {
      menu_item.set_help("");
      return;
    }

    const std::string filename = FileSystem::basename(file_path);
    const std::string title_or_filename_line = title.empty() ? filename : "\"" + title + "\""; // assumes path is just a filename

    std::string author_lines = "";

    for (const std::string& author : authors) {
      author_lines.append("\n" + fmt::format(fmt::runtime(_("Author") +": {}"), author));
    }

    const std::string license_line = fmt::format(fmt::runtime(_("License") + ": {}"), license);

    const std::string help_text =
        title_or_filename_line
        + author_lines
        + (license.empty() ? "" : "\n" + license_line);

    menu_item.set_help(help_text);
}

/* EOF */
