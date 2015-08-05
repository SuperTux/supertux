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
#include <stdexcept>

#include "control/input_manager.hpp"
#include "editor/input_gui.hpp"
#include "editor/layers_gui.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
//#include "supertux/game_session.hpp"
#include "supertux/screen.hpp"
#include "util/currenton.hpp"

class Level;
class Sector;
class World;

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

    void event(SDL_Event& ev);

    std::unique_ptr<Level> level;
    std::unique_ptr<World> world;

    std::string levelfile;

    bool quit_request;
    bool newlevel_request;
    bool reload_request;
    bool reactivate_request;
    bool save_request;

    void disable_keyboard() {
      enabled = false;
    }

    Sector* currentsector;

    bool levelloaded;

    EditorInputGui tileselect;
    EditorLayersGui layerselect;

    void load_sector(std::string name);
    void load_sector(int id);

  private:
    bool enabled;

    void reload_level();
    void load_layers();
    void quit_editor();
    void update_keyboard();

    Editor(const Editor&);
    Editor& operator=(const Editor&);
};

#endif // HEADER_SUPERTUX_EDITOR_EDITOR_HPP
