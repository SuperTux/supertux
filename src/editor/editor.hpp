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

#include "editor/input_center.hpp"
#include "editor/input_gui.hpp"
#include "editor/layers_gui.hpp"
#include "editor/scroller.hpp"
#include "supertux/screen.hpp"
#include "util/currenton.hpp"
#include "video/surface_ptr.hpp"

class GameObject;
class Level;
class ObjectGroup;
class Path;
class Savegame;
class Sector;
class TileSet;
class World;

class Editor : public Screen,
               public Currenton<Editor>
{
  public:
    Editor();

    virtual void draw(Compositor&) override;
    virtual void update(float elapsed_time) override;

    virtual void setup() override;
    virtual void leave() override;

    void event(SDL_Event& ev);
    void resize();

  protected:
    std::unique_ptr<Level> level;
    std::unique_ptr<World> world;

    std::string levelfile;
    std::string test_levelfile;
    bool worldmap_mode;

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

    static bool is_active();

    Level* get_level() const {
      return level.get();
    }

    World* get_world() const {
      return world.get();
    }

    void set_world(std::unique_ptr<World> w);

    TileSet* get_tileset() const {
      return tileset;
    }

    TileSelection* get_tiles() const {
      return tileselect.tiles.get();
    }

    const std::string& get_tileselect_object() const {
      return tileselect.object;
    }

    EditorInputGui::InputType get_tileselect_input_type() const {
      return tileselect.input_type;
    }

    int get_tileselect_select_mode() const;

    int get_tileselect_move_mode() const;

    std::string get_levelfile() const {
      return levelfile;
    }

    void set_level(const std::string& levelfile_) {
      Editor::current()->levelfile = levelfile_;
      Editor::current()->reload_request = true;
    }

    void set_worldmap_mode(bool new_mode) {
      worldmap_mode = new_mode;
    }

    bool get_worldmap_mode() const {
      return worldmap_mode;
    }

    bool is_testing_level() const {
      return leveltested;
    }
    
    /**
     * Checks whether the level can be saved and
     * does not contain obvious issues
     * (currently: check if main sector
     *  and a spawn point named "main" is present)
     */
    void check_save_prerequisites(bool& sector_valid, bool& spawnpoint_valid) const;

    void load_sector(const std::string& name);
    void load_sector(size_t id);

    void update_node_iterators();
    void esc_press();
    void delete_markers();
    void sort_layers();

    void select_tilegroup(int id);
    const std::vector<Tilegroup>& get_tilegroups() const;
    void change_tileset();

    void select_objectgroup(int id);
    const std::vector<ObjectGroup>& get_objectgroups() const;

    std::unique_ptr<Savegame> m_savegame;

    Sector* currentsector;

    // speed is in tiles per frame
    void scroll_up(float speed = 1.0f);
    void scroll_down(float speed = 1.0f);
    void scroll_left(float speed = 1.0f);
    void scroll_right(float speed = 1.0f);

    bool is_level_loaded() const { return levelloaded; }

    void edit_path(Path* path, GameObject* new_marked_object) {
      inputcenter.edit_path(path, new_marked_object);
    }

    void add_layer(GameObject* layer) {
      layerselect.add_layer(layer);
    }

    GameObject* get_selected_tilemap() const { return layerselect.selected_tilemap; }

  protected:
    bool levelloaded;
    bool leveltested;

    TileSet* tileset;

    EditorInputCenter inputcenter;
    EditorInputGui tileselect;
    EditorLayersGui layerselect;
    EditorScroller scroller;

  private:
    bool enabled;
    SurfacePtr bgr_surface;

    void reload_level();
    void load_layers();
    void quit_editor();
    void test_level();
    void update_keyboard();

    bool can_scroll_horz() const;
    bool can_scroll_vert() const;

    Editor(const Editor&);
    Editor& operator=(const Editor&);
};

#endif // HEADER_SUPERTUX_EDITOR_EDITOR_HPP

/* EOF */
