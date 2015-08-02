//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_EDITOR_HPP
#define HEADER_SUPERTUX_EDITOR_EDITOR_HPP

#include <string>

#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
//#include "supertux/game_session.hpp"
#include "supertux/screen.hpp"
#include "util/currenton.hpp"

class Level;
class Sector;

class Editor : public Screen,
               public Currenton<Editor>
{
  public:
    Editor();
    ~Editor();

    virtual void draw(DrawingContext&) override;
    virtual void update(float elapsed_time) override;

    virtual void setup() override;
    virtual void leave() override;

    std::unique_ptr<Level> level;

    std::string levelset;
    std::string levelfile;

    bool quit_request;
    bool newlevel_request;
    bool reload_request;

    Sector* currentsector;

  private:
    bool levelloaded;

    void reload_level();
    void quit_editor();
};

#endif // HEADER_SUPERTUX_EDITOR_EDITOR_HPP
