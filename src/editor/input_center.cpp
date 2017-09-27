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

#include "editor/input_center.hpp"

#include "badguy/willowisp.hpp"
#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "editor/node_marker.hpp"
#include "editor/object_menu.hpp"
#include "editor/point_marker.hpp"
#include "editor/tile_selection.hpp"
#include "editor/tip.hpp"
#include "editor/tool_icon.hpp"
#include "editor/worldmap_objects.hpp"
#include "math/rectf.hpp"
#include "object/ambient_sound.hpp"
#include "object/camera.hpp"
#include "object/invisible_wall.hpp"
#include "object/path.hpp"
#include "object/platform.hpp"
#include "object/coin.hpp"
#include "object/tilemap.hpp"
#include "object/wind.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/game_object.hpp"
#include "supertux/game_object_ptr.hpp"
#include "supertux/level.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "trigger/climbable.hpp"
#include "trigger/scripttrigger.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "trigger/sequence_trigger.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "math/vector.hpp"

bool EditorInputCenter::render_grid = true;
bool EditorInputCenter::snap_to_grid = false;
int EditorInputCenter::selected_snap_grid_size = 3;

EditorInputCenter::EditorInputCenter() :
  hovered_tile(0, 0),
  sector_pos(0, 0),
  mouse_pos(0, 0),
  dragging(false),
  dragging_right(false),
  drag_start(0, 0),
  dragged_object(NULL),
  hovered_object(NULL),
  marked_object(NULL),
  edited_path(NULL),
  last_node_marker(NULL),
  object_tip(),
  obj_mouse_desync(0, 0)
{
}

EditorInputCenter::~EditorInputCenter()
{
}

void
EditorInputCenter::update(float elapsed_time) {
  if (hovered_object && !hovered_object->is_valid()) {
    hovered_object = NULL;
    object_tip = NULL;
  }

  if (marked_object && !marked_object->is_valid()) {
    delete_markers();
  }

  if (edited_path && !edited_path->is_valid()) {
    delete_markers();
  }
}

void
EditorInputCenter::delete_markers() {
  auto sector = Editor::current()->currentsector;
  for (auto& moving_object : sector->moving_objects) {
    auto marker = dynamic_cast<PointMarker*>(moving_object);
    if (marker) {
      marker->remove_me();
    }
  }
  marked_object = NULL;
  edited_path = NULL;
  last_node_marker = NULL;
}

Rectf
EditorInputCenter::drag_rect() {
  int start_x, start_y, end_x, end_y;

  if (drag_start.x < sector_pos.x) {
    start_x = drag_start.x;
    end_x = sector_pos.x;
  } else {
    start_x = sector_pos.x;
    end_x = drag_start.x;
  }

  if (drag_start.y < sector_pos.y) {
    start_y = drag_start.y;
    end_y = sector_pos.y;
  } else {
    start_y = sector_pos.y;
    end_y = drag_start.y;
  }

  return Rectf( start_x, start_y, end_x, end_y );
}

void
EditorInputCenter::input_tile(const Vector& pos, uint32_t tile) {
  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if ( !tilemap ) {
    return;
  }

  if ( pos.x < 0 || pos.y < 0 ||
       pos.x >= tilemap->get_width() ||
       pos.y >= tilemap->get_height()) {
    return;
  }

  tilemap->change(pos.x, pos.y, tile);
}

void
EditorInputCenter::put_tile() {
  auto tiles = Editor::current()->tileselect.tiles.get();
  Vector add_tile;
  for (add_tile.x = tiles->width-1; add_tile.x >= 0; add_tile.x--) {
    for (add_tile.y = tiles->height-1; add_tile.y >= 0; add_tile.y--) {
      input_tile(hovered_tile + add_tile, tiles->pos(add_tile.x, add_tile.y));
    }
  }
}

void
EditorInputCenter::draw_rectangle() {

  Rectf dr = drag_rect();
  dr.p1 = sp_to_tp(dr.p1);
  dr.p2 = sp_to_tp(dr.p2);
  bool sgn_x = drag_start.x < sector_pos.x;
  bool sgn_y = drag_start.y < sector_pos.y;

  int x_ = sgn_x ? 0 : -dr.get_width();
  for (int x = dr.p1.x; x <= dr.p2.x; x++, x_++) {
    int y_ = sgn_y ? 0 : -dr.get_height();
    for (int y = dr.p1.y; y <= dr.p2.y; y++, y_++) {
      input_tile( Vector(x, y), Editor::current()->tileselect.tiles->pos(x_, y_) );
    }
  }
}

void
EditorInputCenter::fill() {

  auto editor = Editor::current();
  auto tiles = editor->tileselect.tiles.get();
  auto tilemap = dynamic_cast<TileMap*>(editor->layerselect.selected_tilemap);
  if (! tilemap) {
    return;
  }

  // The tile that is going to be replaced:
  Uint32 replace_tile = tilemap->get_tile_id(hovered_tile.x, hovered_tile.y);

  if (replace_tile == tiles->pos(0, 0)) {
    // Replacing by the same tiles shouldn't do anything.
    return;
  }

  std::vector<Vector> pos_stack;
  pos_stack.clear();
  pos_stack.push_back(hovered_tile);

  // Passing recursively trough all tiles to be replaced...
  while (pos_stack.size()) {

    if (pos_stack.size() > 1000000) {
      log_warning << "More than 1'000'000 tiles in stack to fill, STOP" << std::endl;
      return;
    }

    Vector pos = pos_stack[pos_stack.size() - 1];
    Vector tpos = pos - hovered_tile;

    // Tests for being inside tilemap:
    if ( pos.x < 0 || pos.y < 0 ||
         pos.x >= tilemap->get_width() || pos.y >= tilemap->get_height()) {
      pos_stack.pop_back();
      continue;
    }

    input_tile(pos, tiles->pos(tpos.x, tpos.y));
    Vector pos_;

    // Going left...
    pos_ = pos + Vector(-1, 0);
    if (pos_.x >= 0) {
      if (replace_tile == tilemap->get_tile_id(pos_.x, pos_.y) &&
          replace_tile != tiles->pos(tpos.x - 1, tpos.y)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going right...
    pos_ = pos + Vector(1, 0);
    if (pos_.x < tilemap->get_width()) {
      if (replace_tile == tilemap->get_tile_id(pos_.x, pos_.y) &&
          replace_tile != tiles->pos(tpos.x + 1, tpos.y)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going up...
    pos_ = pos + Vector(0, -1);
    if (pos_.y >= 0) {
      if (replace_tile == tilemap->get_tile_id(pos_.x, pos_.y) &&
          replace_tile != tiles->pos(tpos.x, tpos.y - 1)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going down...
    pos_ = pos + Vector(0, 1);
    if (pos_.y < tilemap->get_height()) {
      if (replace_tile == tilemap->get_tile_id(pos_.x, pos_.y) &&
          replace_tile != tiles->pos(tpos.x, tpos.y + 1)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // When tiles on each side are already filled or occupied by another tiles, it ends.
    pos_stack.pop_back();
  }
}

void
EditorInputCenter::hover_object() {
  for (auto& moving_object : Editor::current()->currentsector->moving_objects) {
    auto pm = dynamic_cast<PointMarker*>(moving_object);
    if (!moving_object->do_save() && !pm) {
      continue;
    }
    Rectf bbox = moving_object->get_bbox();
    if (bbox.contains(sector_pos)) {
      if (moving_object != hovered_object) {
        if (moving_object->do_save()) {
          std::unique_ptr<Tip> new_tip(new Tip(moving_object));
          object_tip = move(new_tip);
        }
        hovered_object = moving_object;
      }
      return;
    }
  }
  object_tip = NULL;
  hovered_object = NULL;
}

void
EditorInputCenter::edit_path(Path* path, GameObject* new_marked_object) {
  if (!path) return;
  delete_markers();

  if (!path->is_valid()) {
    edited_path = NULL;
    return;
  }
  edited_path = path;
  edited_path->edit_path();
  if (new_marked_object) {
    marked_object = new_marked_object;
  }
}

void
EditorInputCenter::mark_object() {
  delete_markers();
  if (!dragged_object || !dragged_object->is_valid()) return;

  if (dragged_object->has_variable_size()) {
    marked_object = dragged_object;
    dragged_object->edit_bbox();
    return;
  }

  auto coin = dynamic_cast<Coin*>(dragged_object);
  if (coin) {
    if (coin->get_path()) {
      edit_path(coin->get_path(), dragged_object);
    }
    return;
  }

  auto willo = dynamic_cast<WillOWisp*>(dragged_object);
  if (willo) {
    if (willo->get_path()) {
      edit_path(willo->get_path(), dragged_object);
    }
    return;
  }

  auto platform = dynamic_cast<Platform*>(dragged_object);
  if (platform) {
    edit_path(&platform->get_path(), dragged_object);
    return;
  }
}

void
EditorInputCenter::grab_object() {
  if (hovered_object) {
    if (!hovered_object->is_valid()) {
      hovered_object = NULL;
      return;
    }

    dragged_object = hovered_object;
    auto pm = dynamic_cast<PointMarker*>(hovered_object);
    obj_mouse_desync = sector_pos - hovered_object->get_pos();
    // marker testing
    if (!pm) {
      mark_object();
    }
    last_node_marker = dynamic_cast<NodeMarker*>(pm);
    return;
  }
  dragged_object = NULL;
  if (edited_path && Editor::current()->tileselect.object == "#node") {
    if (edited_path->is_valid()) {
      return;
    }
  }
  delete_markers();
}

void
EditorInputCenter::clone_object() {
  auto editor = Editor::current();
  if (hovered_object && hovered_object->do_save()) {
    if (!hovered_object->is_valid()) {
      hovered_object = NULL;
      return;
    }

    auto pm = dynamic_cast<PointMarker*>(hovered_object);
    if (pm) {
      return; //Do not clone markers
    }
    obj_mouse_desync = sector_pos - hovered_object->get_pos();

    auto tileselect = &(editor->tileselect);
    GameObjectPtr game_object;
    try {
      game_object = ObjectFactory::instance().create(hovered_object->get_class(), hovered_object->get_pos());
    } catch(const std::exception& e) {
      log_warning << "Error creating object " << hovered_object->get_class() << ": " << e.what() << std::endl;
      return;
    }
    if (!game_object)
      throw std::runtime_error("Cloning object failed.");

    try {
      editor->currentsector->add_object(game_object);
    } catch(const std::exception& e) {
      log_warning << "Error adding object " << tileselect->object << ": " << e.what() << std::endl;
      return;
    }

    dragged_object = dynamic_cast<MovingObject*>(game_object.get());
    ObjectSettings settings = hovered_object->get_settings();
    dragged_object->get_settings().copy_from(&settings);
    dragged_object->after_editor_set();
    return;
  }
  dragged_object = NULL;
  delete_markers();
}

void
EditorInputCenter::set_object() {
  if (hovered_object && hovered_object->is_valid() && hovered_object->do_save()) {
    std::unique_ptr<Menu> om(new ObjectMenu(hovered_object));
    Editor::current()->deactivate_request = true;
    MenuManager::instance().push_menu(move(om));
    return;
  }
}

void
EditorInputCenter::move_object() {
  if (dragged_object) {
    if (!dragged_object->is_valid()) {
      dragged_object = NULL;
      return;
    }
    Vector new_pos = sector_pos - obj_mouse_desync;
    if (snap_to_grid) {
      auto& snap_grid_size = snap_grid_sizes[selected_snap_grid_size];
      new_pos = (new_pos / snap_grid_size).to_int_vec() * snap_grid_size;

      auto pm = dynamic_cast<PointMarker*>(dragged_object);
      if (pm) {
        new_pos -= pm->get_offset();
      }
    }
    dragged_object->move_to(new_pos);
  }
}

void
EditorInputCenter::rubber_object() {
  if (!edited_path) {
    delete_markers();
  }
  if (dragged_object) {
    dragged_object->editor_delete();
  }
  last_node_marker = NULL;
}

void
EditorInputCenter::rubber_rect() {
  delete_markers();
  Rectf dr = drag_rect();
  for (auto& moving_object : Editor::current()->currentsector->moving_objects) {
    Rectf bbox = moving_object->get_bbox();
    if (dr.contains(bbox)) {
      moving_object->editor_delete();
    }
  }
  last_node_marker = NULL;
}

void
EditorInputCenter::update_node_iterators() {
  if (!edited_path) return;
  if (!edited_path->is_valid()) return;

  auto sector = Editor::current()->currentsector;
  for (auto& moving_object : sector->moving_objects) {
    auto marker = dynamic_cast<NodeMarker*>(moving_object);
    if (marker) {
      marker->update_iterator();
    }
  }
}

void
EditorInputCenter::add_path_node() {
  Path::Node new_node;
  new_node.position = sector_pos;
  new_node.time = 1;
  edited_path->nodes.insert(last_node_marker->node + 1, new_node);
  GameObjectPtr marker;
  marker = std::make_shared<NodeMarker>(edited_path, edited_path->nodes.end() - 1, edited_path->nodes.size() - 1);
  Sector::current()->add_object(marker);
  //last_node_marker = dynamic_cast<NodeMarker*>(marker.get());
  update_node_iterators();
  Editor::current()->currentsector->update(0);
  grab_object();
}

void
EditorInputCenter::put_object() {
  auto editor = Editor::current();
  auto tileselect = &(editor->tileselect);
  if (tileselect->object[0] == '#') {
    if (edited_path && tileselect->object == "#node") {
      if (edited_path->is_valid() && last_node_marker) {
        add_path_node();
      }
    }
    return;
  }
  GameObjectPtr game_object;
  try {
    auto target_pos = sector_pos;
    if(snap_to_grid)
    {
      auto& snap_grid_size = snap_grid_sizes[selected_snap_grid_size];
      target_pos = (sector_pos / snap_grid_size).to_int_vec() * snap_grid_size;
    }
    game_object = ObjectFactory::instance().create(tileselect->object, target_pos, LEFT);
  } catch(const std::exception& e) {
    log_warning << "Error creating object " << tileselect->object << ": " << e.what() << std::endl;
    return;
  }
  if (game_object == NULL)
    throw std::runtime_error("Creating " + tileselect->object + " object failed.");

  auto mo = dynamic_cast<MovingObject*> (game_object.get());
  if (!mo) {
    editor->layerselect.add_layer(game_object.get());
  }
  else if(!snap_to_grid) {
    auto bbox = mo->get_bbox();
    mo->move_to(mo->get_pos() - Vector(bbox.get_width() / 2, bbox.get_height() / 2));
  }

  auto wo = dynamic_cast<worldmap_editor::WorldmapObject*> (game_object.get());
  if (wo) {
    wo->move_to(wo->get_pos() / 32);
  }

  try {
    editor->currentsector->add_object(game_object);
  } catch(const std::exception& e) {
    log_warning << "Error adding object " << tileselect->object << ": " << e.what() << std::endl;
    return;
  }
}

void
EditorInputCenter::process_left_click() {
  auto tileselect = &(Editor::current()->tileselect);
  dragging = true;
  dragging_right = false;
  drag_start = sector_pos;
  switch (tileselect->input_type) {
    case EditorInputGui::IP_TILE: {
      switch (tileselect->select_mode->get_mode()) {
        case 0:
          put_tile();
          break;
        case 1:
          draw_rectangle();
          break;
        case 2:
          fill();
          break;
        default:
          break;
      }
    } break;
    case EditorInputGui::IP_OBJECT:
      switch (tileselect->move_mode->get_mode()) {
        case 0:
          grab_object();
          break;
        case 1:
          clone_object();
          break;
        default:
          break;
      }
      if (!tileselect->object.empty()) {
        if (!dragged_object) {
          put_object();
        }
      } else {
        rubber_object();
      }
    break;
  default:
    break;
  }
}

void
EditorInputCenter::process_right_click() {
  switch (Editor::current()->tileselect.input_type) {
    case EditorInputGui::IP_TILE: {
      dragging = true;
      dragging_right = true;
      drag_start = sector_pos;
      update_tile_selection();
    } break;
    case EditorInputGui::IP_NONE:
    case EditorInputGui::IP_OBJECT:
      set_object();
      break;
    default:
      break;
  }
}

Rectf
EditorInputCenter::tile_drag_rect() {
  Rectf result = drag_rect();
  result.p1 = sp_to_tp(result.p1);
  result.p2 = sp_to_tp(result.p2);
  return result;
}

Rectf
EditorInputCenter::selection_draw_rect() {
  Rectf select = tile_drag_rect();
  select.p2 += Vector(1, 1);
  select.p1 = tile_screen_pos(select.p1);
  select.p2 = tile_screen_pos(select.p2);
  return select;
}

void
EditorInputCenter::update_tile_selection() {
  Rectf select = tile_drag_rect();
  auto tiles = Editor::current()->tileselect.tiles.get();
  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if ( !tilemap ) {
    return;
  }

  tiles->tiles.clear();
  tiles->width = select.get_width() + 1;
  tiles->height = select.get_height() + 1;

  int w = tilemap->get_width();
  int h = tilemap->get_height();
  for (int y = select.p1.y; y <= select.p2.y; y++) {
    for (int x = select.p1.x; x <= select.p2.x; x++) {
      if ( x < 0 || y < 0 || x >= w || y >= h) {
        tiles->tiles.push_back(0);
      } else {
        tiles->tiles.push_back(tilemap->get_tile_id(x, y));
      }
    }
  }
}

void
EditorInputCenter::event(SDL_Event& ev) {
  auto tileselect = &(Editor::current()->tileselect);
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    switch (ev.button.button) {
      case SDL_BUTTON_LEFT: {
        process_left_click();
      } break;
      case SDL_BUTTON_RIGHT: {
        process_right_click();
      } break;
    } break;

    case SDL_MOUSEBUTTONUP:
      dragging = false;
      break;

    case SDL_MOUSEMOTION:
    {
      mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      update_pos();
      if (dragging) {
        switch (tileselect->input_type) {
          case EditorInputGui::IP_TILE:
            if (dragging_right) {
              update_tile_selection();
            } else {
              switch (tileselect->select_mode->get_mode()) {
                case 0:
                  put_tile();
                  break;
                case 1:
                  draw_rectangle();
                  break;
                default:
                  break;
              }
            }
            break;
          case EditorInputGui::IP_OBJECT:
            if (tileselect->object.empty()) {
              if (tileselect->select_mode->get_mode() == 1) {
                rubber_rect();
              }
            } else {
              move_object();
            }
            break;
          default:
            break;
        }
      }
    } break;
    case SDL_KEYDOWN:
      if (ev.key.keysym.sym == SDLK_F8) {
        render_grid = !render_grid;
      } else if (ev.key.keysym.sym == SDLK_F7) {
        snap_to_grid = !snap_to_grid;
      }
      break;
    default:
      break;
  }
}

void
EditorInputCenter::update_pos() {
  sector_pos = mouse_pos + Editor::current()->currentsector->camera->get_translation();
  hovered_tile = sp_to_tp(sector_pos);
  // update tip
  hover_object();
}

void
EditorInputCenter::draw_tile_tip(DrawingContext& context) {
  auto editor = Editor::current();
  if ( editor->tileselect.input_type == EditorInputGui::IP_TILE ) {

    auto tilemap = dynamic_cast<TileMap*>(editor->layerselect.selected_tilemap);
    if (!tilemap) {
      return;
    }

    Vector drawn_tile = hovered_tile;
    auto tiles = editor->tileselect.tiles.get();

    for (drawn_tile.x = tiles->width-1; drawn_tile.x >= 0; drawn_tile.x--) {
      for (drawn_tile.y = tiles->height-1; drawn_tile.y >= 0; drawn_tile.y--) {
        Vector on_tile = hovered_tile + drawn_tile;

        if ( editor->tileselect.tiles->empty() || on_tile.x < 0 || on_tile.y < 0 ||
             on_tile.x >= tilemap->get_width() || on_tile.y >= tilemap->get_height()) {
          continue;
        }
        uint32_t tile_id = tiles->pos(drawn_tile.x, drawn_tile.y);
        editor->tileset->draw_tile(context, tile_id, tp_to_sp(on_tile) - editor->currentsector->camera->get_translation(),
                                   LAYER_GUI-11, Color(1, 1, 1, 0.5));
        /*if (tile_id) {
          const Tile* tg_tile = editor->tileset->get( tile_id );
          tg_tile->draw(context, tp_to_sp(on_tile) - editor->currentsector->camera->get_translation(),
                        LAYER_GUI-11, Color(1, 1, 1, 0.5));
        }*/
      }
    }
  }
}

void
EditorInputCenter::draw_tile_grid(DrawingContext& context, const Color& line_color, int tile_size) {
  auto editor = Editor::current();
  if ( !editor->layerselect.selected_tilemap ) {
    return;
  }

  auto current_tm = dynamic_cast<TileMap*>(editor->layerselect.selected_tilemap);
  if ( current_tm == NULL )
    return;
  int tm_width = current_tm->get_width() * (32 / tile_size);
  int tm_height = current_tm->get_height() * (32 / tile_size);
  auto cam_translation = editor->currentsector->camera->get_translation();
  Rectf draw_rect = Rectf(cam_translation, cam_translation +
                          Vector(SCREEN_WIDTH, SCREEN_HEIGHT));
  Vector start = sp_to_tp( Vector(draw_rect.p1.x, draw_rect.p1.y), tile_size );
  Vector end = sp_to_tp( Vector(draw_rect.p2.x, draw_rect.p2.y), tile_size );
  start.x = std::max(0.0f, start.x);
  start.y = std::max(0.0f, start.y);
  end.x = std::min(float(tm_width-1), end.x);
  end.y = std::min(float(tm_height-1), end.y);

  Vector line_start, line_end;
  for (int i = start.x; i <= end.x; i++) {
    line_start = tile_screen_pos( Vector(i, 0), tile_size );
    line_end = tile_screen_pos( Vector(i, tm_height), tile_size );
    context.draw_line(line_start, line_end, line_color, current_tm->get_layer());
  }

  for (int i = start.y; i <= end.y; i++) {
    line_start = tile_screen_pos( Vector(0, i), tile_size );
    line_end = tile_screen_pos( Vector(tm_width, i), tile_size );
    context.draw_line(line_start, line_end, line_color, current_tm->get_layer());
  }
}

void
EditorInputCenter::draw_tilemap_border(DrawingContext& context) {
  auto editor = Editor::current();
  if ( !editor->layerselect.selected_tilemap ) return;

  auto current_tm = dynamic_cast<TileMap*>(editor->layerselect.selected_tilemap);
  if ( !current_tm ) return;

  Vector start = tile_screen_pos( Vector(0, 0) );
  Vector end = tile_screen_pos( Vector(current_tm->get_width(), current_tm->get_height()) );
  context.draw_line(start, Vector(start.x, end.y), Color(1, 0, 1), current_tm->get_layer());
  context.draw_line(start, Vector(end.x, start.y), Color(1, 0, 1), current_tm->get_layer());
  context.draw_line(Vector(start.x, end.y), end, Color(1, 0, 1), current_tm->get_layer());
  context.draw_line(Vector(end.x, start.y), end, Color(1, 0, 1), current_tm->get_layer());
}

void
EditorInputCenter::draw_path(DrawingContext& context) {
  if (!edited_path) return;
  if (!marked_object) return;
  if (!marked_object->is_valid()) return;
  if (!edited_path->is_valid()) return;

  for(auto i = edited_path->nodes.begin(); i != edited_path->nodes.end(); ++i) {
    auto j = i+1;
    Path::Node* node1 = &(*i);
    Path::Node* node2;
    if (j == edited_path->nodes.end()) {
      if (edited_path->mode == Path::CIRCULAR || edited_path->mode == Path::UNORDERED) {
        //loop to the first node
        node2 = &(*edited_path->nodes.begin());
      } else {
        continue;
      }
    } else {
      node2 = &(*j);
    }
    auto cam_translation = Editor::current()->currentsector->camera->get_translation();
    context.draw_line(node1->position - cam_translation,
                      node2->position - cam_translation,
                      Color(1, 0, 0), LAYER_GUI - 21);
  }
}

void
EditorInputCenter::draw(DrawingContext& context) {
  auto editor = Editor::current();
  draw_tile_tip(context);
  draw_path(context);

  if (render_grid) {
    draw_tile_grid(context, Color(1, 1, 1, 0.7));
    draw_tilemap_border(context);
    auto snap_grid_size = snap_grid_sizes[selected_snap_grid_size];
    if (snap_grid_size != 32) {
      draw_tile_grid(context, Color(1, 1, 1, 0.4), snap_grid_size);
    }
  }

  if (object_tip) {
    object_tip->draw(context, mouse_pos);
  }

  if (dragging && editor->tileselect.select_mode->get_mode() == 1
      && !dragging_right) {
    // Draw selection rectangle...
    auto cam_translation = editor->currentsector->camera->get_translation();
    Vector p0 = drag_start - cam_translation;
    Vector p1 = Vector(drag_start.x, sector_pos.y) - cam_translation;
    Vector p2 = Vector(sector_pos.x, drag_start.y) - cam_translation;

    context.draw_filled_rect(Rectf(p0, p1 + Vector(2, 2)),
                             Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);
    context.draw_filled_rect(Rectf(p2, mouse_pos + Vector(2, 2)),
                             Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);
    context.draw_filled_rect(Rectf(p0, p2 + Vector(2, 2)),
                             Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);
    context.draw_filled_rect(Rectf(p1, mouse_pos + Vector(2, 2)),
                             Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);

    context.draw_filled_rect(Rectf(p0, mouse_pos),
                             Color(0.0f, 1.0f, 0.0f, 0.2f), 0.0f, LAYER_GUI-5);
  }

  if (dragging && dragging_right) {
    context.draw_filled_rect(selection_draw_rect(),
                             Color(0.2f, 0.4f, 1.0f, 0.6f), 0.0f, LAYER_GUI-13);
  }
}

Vector
EditorInputCenter::tp_to_sp(const Vector& tp, int tile_size) {
  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if(!tilemap)
  {
    return Vector(0, 0);
  }

  Vector sp = tp * tile_size;
  return sp + tilemap->get_offset();
}

Vector
EditorInputCenter::sp_to_tp(const Vector& sp, int tile_size) {
  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if(!tilemap)
  {
    return Vector(0, 0);
  }

  Vector sp_ = sp - tilemap->get_offset();
  return sp_ / tile_size;
}

Vector
EditorInputCenter::tile_screen_pos(const Vector& tp, int tile_size) {
  Vector sp = tp_to_sp(tp, tile_size);
  return sp - Editor::current()->currentsector->camera->get_translation();
}

/* EOF */
