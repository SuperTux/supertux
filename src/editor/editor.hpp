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
#include "editor/input_center.hpp"
#include "editor/input_gui.hpp"
#include "editor/layers_gui.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/screen.hpp"
#include "util/currenton.hpp"

class Level;
class Savegame;
class Sector;
class TileSet;
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
    void resize();

  protected:
    friend class EditorInputCenter;
    friend class EditorInputGui;
    friend class EditorLayersGui;
    friend class EditorLevelsetSelectMenu;
    friend class EditorObjectgroupMenu;
    friend class EditorTilegroupMenu;

    std::unique_ptr<Level> level;
    std::unique_ptr<World> world;

    std::string levelfile;

  public:
    bool quit_request;
    bool newlevel_request;
    bool reload_request;
    bool reactivate_request;
    bool deactivate_request;
    bool save_request;
    bool test_request;

    void disable_keyboard() {
      enabled = false;
    }

    bool is_active();

    Level* get_level() const {
      return level.get();
    }

    World* get_world() const {
      return world.get();
    }

    std::string get_levelfile() const {
      return levelfile;
    }

    void set_level(std::string levelfile_) {
      Editor::current()->levelfile = levelfile_;
      Editor::current()->reload_request = true;
    }

    void load_sector(std::string name);
    void load_sector(int id);

    std::unique_ptr<Savegame> m_savegame;

    Sector* currentsector;

  protected:
    bool levelloaded;
    bool leveltested;

    TileSet* tileset;

    EditorInputCenter inputcenter;
    EditorInputGui tileselect;
    EditorLayersGui layerselect;

    // speed is in tiles per frame
    void scroll_up(float speed = 1.0f);
    void scroll_down(float speed = 1.0f);
    void scroll_left(float speed = 1.0f);
    void scroll_right(float speed = 1.0f);

  private:
    bool enabled;

    void reload_level();
    void load_layers();
    void quit_editor();
    void test_level();
    void update_keyboard();

    bool can_scroll_horz();
    bool can_scroll_vert();

    Editor(const Editor&);
    Editor& operator=(const Editor&);
};

inline bool EditorActive() {
  if (Editor::current()) {
    if (Editor::current()->is_active()) {
      return true;
    }
  }
  return false;
}

#endif // HEADER_SUPERTUX_EDITOR_EDITOR_HPP

/* EOF */
