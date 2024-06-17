//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_PORT_EMSCRIPTEN_HPP
#define HEADER_SUPERTUX_PORT_EMSCRIPTEN_HPP

// Export functions for emscripten
// If you add functions here, make sure to make CMakeLists.txt export them!
#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>

#include "addon/addon_manager.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/video_system.hpp"

extern "C" {

void set_resolution(int w, int h);
void save_config();
void init_emscripten();
void onDownloadProgress(int id, int loaded, int total);
void onDownloadFinished(int id);
void onDownloadError(int id);
void onDownloadAborted(int id);
const char* getExceptionMessage(intptr_t address);

} // extern "C"

#endif

#endif
