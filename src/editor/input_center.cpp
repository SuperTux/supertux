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
#include "math/rectf.hpp"
#include "object/ambient_sound.hpp"
#include "object/camera.hpp"
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

EditorInputCenter::EditorInputCenter() :
  hovered_tile(0, 0),
  sector_pos(0, 0),
  mouse_pos(0, 0),
  dragging(false),
  dragging_left(false),
  drag_start(0, 0),
  dragged_object(NULL),
  marked_object(NULL),
  edited_path(NULL),
  last_node_marker(NULL),
  object_tip(),
  obj_mouse_desync(0, 0),
  render_grid(true),
  scrolling(Vector())
{
}

EditorInputCenter::~EditorInputCenter()
{
}

void
EditorInputCenter::update(float elapsed_time) {
  update_scroll();

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
  for (auto moving_object : sector->moving_objects) {
    PointMarker* marker = dynamic_cast<PointMarker*>(moving_object);
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

  return Rectf( Vector(start_x, start_y), Vector(end_x, end_y) );
}

void
EditorInputCenter::input_tile(Vector pos, uint32_t tile) {
  TileMap* tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
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

  int x_, y_;
  x_ = sgn_x ? 0 : -dr.get_width();
  for (int x = dr.p1.x; x <= dr.p2.x; x++, x_++) {
    y_ = sgn_y ? 0 : -dr.get_height();
    for (int y = dr.p1.y; y <= dr.p2.y; y++, y_++) {
      input_tile( Vector(x, y), Editor::current()->tileselect.tiles->pos(x_, y_) );
    }
  }
}

void
EditorInputCenter::fill() {

  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if (! tilemap) {
    return;
  }

  // The tile that is going to be replaced:
  Uint32 replace_tile = tilemap->get_tile_id(hovered_tile.x, hovered_tile.y);

  if (Editor::current()->tileselect.tiles->pos(0, 0) == tilemap->get_tile_id(hovered_tile.x, hovered_tile.y)) {
    // Replacing by the same tiles shouldn't do anything.
    return;
  }

  std::vector<Vector> pos_stack;
  pos_stack.clear();
  pos_stack.push_back(hovered_tile);
  auto tiles = Editor::current()->tileselect.tiles.get();

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
  for (auto moving_object : Editor::current()->currentsector->moving_objects) {
    PointMarker* pm = dynamic_cast<PointMarker*>(moving_object);
    if (!moving_object->do_save() && !pm) {
      continue;
    }
    Rectf bbox = moving_object->get_bbox();
    if (sector_pos.x >= bbox.p1.x && sector_pos.y >= bbox.p1.y &&
        sector_pos.x <= bbox.p2.x && sector_pos.y <= bbox.p2.y ) {
      if (moving_object->do_save()) {
        std::unique_ptr<Tip> new_tip(new Tip(moving_object));
        object_tip = move(new_tip);
      } else {
        break;
      }
      return;
    }
  }
  object_tip = NULL;
}

void
EditorInputCenter::edit_path(Path* path, GameObject* new_marked_object) {
  if (!path) return;
  delete_markers();

  if (path->is_valid()) {
    edited_path = path;
    edited_path->edit_path();
    if (new_marked_object) {
      marked_object = new_marked_object;
    }
  } else {
    edited_path = NULL;
  }
}

void
EditorInputCenter::mark_object() {
  delete_markers();

  AmbientSound* dc1 = dynamic_cast<AmbientSound*>(dragged_object);
  Climbable* dc2 = dynamic_cast<Climbable*>(dragged_object);
  ScriptTrigger* dc3 = dynamic_cast<ScriptTrigger*>(dragged_object);
  SecretAreaTrigger* dc4 = dynamic_cast<SecretAreaTrigger*>(dragged_object);
  SequenceTrigger* dc5 = dynamic_cast<SequenceTrigger*>(dragged_object);
  Wind* dc6 = dynamic_cast<Wind*>(dragged_object);

  if (dc1 || dc2 || dc3 || dc4 || dc5 || dc6) {
    marked_object = dragged_object;
    dragged_object->edit_bbox();
    return;
  }

  Coin* coin = dynamic_cast<Coin*>(dragged_object);
  if (coin) {
    if (coin->get_path()) {
      edit_path(coin->get_path(), dragged_object);
    }
    return;
  }

  WillOWisp* willo = dynamic_cast<WillOWisp*>(dragged_object);
  if (willo) {
    if (willo->get_path()) {
      edit_path(willo->get_path(), dragged_object);
    }
    return;
  }

  Platform* platform = dynamic_cast<Platform*>(dragged_object);
  if (platform) {
    edit_path(&platform->get_path(), dragged_object);
    return;
  }
}

void
EditorInputCenter::grab_object() {
  for (auto moving_object : Editor::current()->currentsector->moving_objects) {
    Rectf bbox = moving_object->get_bbox();

    if (sector_pos.x >= bbox.p1.x && sector_pos.y >= bbox.p1.y &&
        sector_pos.x <= bbox.p2.x && sector_pos.y <= bbox.p2.y ) {
      if (!moving_object->is_valid()) {
        continue;
      }

      dragged_object = moving_object;
      PointMarker* pm = dynamic_cast<PointMarker*>(moving_object);
      obj_mouse_desync = sector_pos - bbox.p1;
      // marker testing
      if (!pm) {
        mark_object();
      }
      last_node_marker = dynamic_cast<NodeMarker*>(pm);
      return;
    }
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
EditorInputCenter::set_object() {
  for (auto moving_object : Editor::current()->currentsector->moving_objects) {
    Rectf bbox = moving_object->get_bbox();
    if (sector_pos.x >= bbox.p1.x && sector_pos.y >= bbox.p1.y &&
        sector_pos.x <= bbox.p2.x && sector_pos.y <= bbox.p2.y ) {
      std::unique_ptr<Menu> om(new ObjectMenu(moving_object));
      Editor::current()->deactivate_request = true;
      MenuManager::instance().push_menu(move(om));
      return;
    }
  }
}

void
EditorInputCenter::move_object() {
  if (dragged_object) {
    if (!dragged_object->is_valid()) {
      dragged_object = NULL;
      return;
    }
    dragged_object->move_to(sector_pos - obj_mouse_desync);
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
  for (auto moving_object : Editor::current()->currentsector->moving_objects) {
    Rectf bbox = moving_object->get_bbox();
    if (bbox.p2.x >= dr.p1.x && bbox.p1.x <= dr.p2.x &&
        bbox.p2.y >= dr.p1.y && bbox.p1.y <= dr.p2.y ) {
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
  for (auto moving_object : sector->moving_objects) {
    NodeMarker* marker = dynamic_cast<NodeMarker*>(moving_object);
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
  auto tileselect = &(Editor::current()->tileselect);
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
    game_object = ObjectFactory::instance().create(tileselect->object, sector_pos, LEFT);
  } catch(const std::exception& e) {
    log_warning << "Error creating object " << tileselect->object << ": " << e.what() << std::endl;
    return;
  }
  if (game_object == NULL)
    throw std::runtime_error("Creating " + tileselect->object + " object failed.");

  MovingObject* mo = dynamic_cast<MovingObject*> (game_object.get());
  if (!mo) {
    Editor::current()->layerselect.add_layer(game_object.get());
  }

  try {
    Editor::current()->currentsector->add_object(game_object);
  } catch(const std::exception& e) {
    log_warning << "Error adding object " << tileselect->object << ": " << e.what() << std::endl;
    return;
  }
}

void
EditorInputCenter::process_left_click() {
  dragging = true;
  dragging_left = false;
  drag_start = sector_pos;
  switch (Editor::current()->tileselect.input_type) {
    case EditorInputGui::IP_TILE: {
      switch (Editor::current()->tileselect.select_mode->get_mode()) {
        case 0:
          put_tile();
          break;
        case 2:
          fill();
          break;
        default:
          break;
      }
    } break;
    case EditorInputGui::IP_OBJECT:
      grab_object();
      if (!Editor::current()->tileselect.object.empty()) {
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
  dragging = true;
  dragging_left = true;
  drag_start = sector_pos;
  switch (Editor::current()->tileselect.input_type) {
    case EditorInputGui::IP_TILE: {
      //possible future usage
    } break;
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
  TileMap* tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
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
        //TODO: Make the right clicks working.
        auto controller = InputManager::current()->get_controller();
        if (controller->hold(Controller::ACTION)) {
          controller->set_control(Controller::ACTION, false);
          process_right_click();
        } else {
          process_left_click();
        }
      } break;
      case SDL_BUTTON_RIGHT: {
        process_right_click();
      } break;
    } break;

    case SDL_MOUSEBUTTONUP:
      dragging = false;
      if(ev.button.button == SDL_BUTTON_LEFT) {
        if (tileselect->input_type == EditorInputGui::IP_OBJECT) {
          if (tileselect->select_mode->get_mode() == 1 &&
              tileselect->object.empty() ) {
            rubber_rect();
          }
        }
      }
      break;

    case SDL_MOUSEMOTION:
    {
      mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      actualize_pos();
      actualize_scrolling();
      if (dragging) {
        switch (tileselect->input_type) {
          case EditorInputGui::IP_TILE:
            if (dragging_left) {
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
              rubber_rect();
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
      }
      break;
    default:
      break;
  }
}

void
EditorInputCenter::actualize_pos() {
  sector_pos = mouse_pos + Editor::current()->currentsector->camera->get_translation();
  hovered_tile = sp_to_tp(sector_pos);
  // update tip
  hover_object();
}

// How close to edge before scrolling starts (in tiles)
#define SCROLL_TILE_PROX 3
#define SCROLL_PROX (SCROLL_TILE_PROX * 32.0f)

void
EditorInputCenter::actualize_scrolling() {
  // What we can actually see
  int editor_height = SCREEN_HEIGHT - 32;
  int editor_width = SCREEN_WIDTH - 128;

  // Proximities to boundaries, may be negative
  int top_prox = mouse_pos.y;
  int bottom_prox = editor_height - mouse_pos.y;

  int left_prox = mouse_pos.x;
  int right_prox = editor_width - mouse_pos.x;

  // Get the distance
  if (left_prox > 0 && left_prox < SCROLL_PROX) {
    left_prox = SCROLL_PROX - left_prox;
    scrolling.x = -(left_prox  / SCROLL_PROX);
  } else if (right_prox > 0 && right_prox < SCROLL_PROX) {
    right_prox = SCROLL_PROX - right_prox;
    scrolling.x = right_prox  / SCROLL_PROX;
  }

  if (top_prox > 0 && top_prox < SCROLL_PROX) {
    top_prox = SCROLL_PROX - top_prox;
    scrolling.y = -(top_prox  / SCROLL_PROX);
  } else if (bottom_prox > 0 && bottom_prox < SCROLL_PROX) {
    bottom_prox = SCROLL_PROX - bottom_prox;
    scrolling.y = bottom_prox / SCROLL_PROX;
  }
}

#undef SCROLL_PROX
#undef SCROLL_TILE_PROX

void
EditorInputCenter::update_scroll() {
  float horiz_scroll = scrolling.x;
  float vert_scroll = scrolling.y;

  if (horiz_scroll < 0)
    Editor::current()->scroll_left(-horiz_scroll);
  else if (horiz_scroll > 0)
    Editor::current()->scroll_right(horiz_scroll);
  else {}

  if (vert_scroll < 0)
    Editor::current()->scroll_up(-vert_scroll);
  else if (vert_scroll > 0)
    Editor::current()->scroll_down(vert_scroll);
  else {}
}

void
EditorInputCenter::stop_scrolling() {
  scrolling.x = 0;
  scrolling.y = 0;
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
        if (tile_id) {
          const Tile* tg_tile = editor->tileset->get( tile_id );
          tg_tile->draw(context, tp_to_sp(on_tile) - editor->currentsector->camera->get_translation(),
                        LAYER_GUI-11, Color(1, 1, 1, 0.5));
        }
      }
    }
  }
}

void
EditorInputCenter::draw_tile_grid(DrawingContext& context) {
  auto editor = Editor::current();
  if ( !editor->layerselect.selected_tilemap ) {
    return;
  }

  TileMap* current_tm = dynamic_cast<TileMap*>(editor->layerselect.selected_tilemap);
  int tm_width = current_tm->get_width();
  int tm_height = current_tm->get_height();
  Rectf draw_rect = Rectf(editor->currentsector->camera->get_translation(),
        editor->currentsector->camera->get_translation() + Vector(SCREEN_WIDTH, SCREEN_HEIGHT));
  Vector start = sp_to_tp( Vector(draw_rect.p1.x, draw_rect.p1.y) );
  Vector end = sp_to_tp( Vector(draw_rect.p2.x, draw_rect.p2.y) );
  start.x = std::max(0.0f, start.x);
  start.y = std::max(0.0f, start.y);
  end.x = std::min(float(tm_width-1), end.x);
  end.y = std::min(float(tm_height-1), end.y);

  Vector line_start, line_end;
  for (int i = start.x; i <= end.x; i++) {
    line_start = tile_screen_pos( Vector(i, 0) );
    line_end = tile_screen_pos( Vector(i, tm_height) );
    context.draw_line(line_start, line_end, Color(1, 1, 1, 0.7), current_tm->get_layer());
  }

  for (int i = start.y; i <= end.y; i++) {
    line_start = tile_screen_pos( Vector(0, i) );
    line_end = tile_screen_pos( Vector(tm_width, i) );
    context.draw_line(line_start, line_end, Color(1, 1, 1, 0.7), current_tm->get_layer());
  }

  start = tile_screen_pos( Vector(0, 0) );
  end = tile_screen_pos( Vector(tm_width, tm_height) );
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
    context.draw_line(node1->position - Editor::current()->currentsector->camera->get_translation(),
                      node2->position - Editor::current()->currentsector->camera->get_translation(),
                      Color(1, 0, 0), LAYER_GUI - 21);
  }
}

void
EditorInputCenter::draw(DrawingContext& context) {
  draw_tile_tip(context);
  draw_path(context);

  if (render_grid) {
    draw_tile_grid(context);
  }

  if (object_tip) {
    object_tip->draw(context, mouse_pos);
  }

  if (dragging && Editor::current()->tileselect.select_mode->get_mode() == 1
      && !dragging_left) {
    // Draw selection rectangle...
    Vector p0 = drag_start - Editor::current()->currentsector->camera->get_translation();
    Vector p1 = Vector(drag_start.x, sector_pos.y) - Editor::current()->currentsector->camera->get_translation();
    Vector p2 = Vector(sector_pos.x, drag_start.y) - Editor::current()->currentsector->camera->get_translation();

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

  if (dragging && dragging_left) {
    context.draw_filled_rect(selection_draw_rect(),
                             Color(0.2f, 0.4f, 1.0f, 0.6f), 0.0f, LAYER_GUI-13);
  }
}

Vector
EditorInputCenter::tp_to_sp(Vector tp) {
  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if (tilemap) {
    Vector sp = Vector( tp.x * 32, tp.y * 32 );
    return sp + tilemap->get_offset();
  } else {
    return Vector(0, 0);
  }
}

Vector
EditorInputCenter::sp_to_tp(Vector sp) {
  auto tilemap = dynamic_cast<TileMap*>(Editor::current()->layerselect.selected_tilemap);
  if (tilemap) {
    sp -= tilemap->get_offset();
    int x = sp.x / 32;
    int y = sp.y / 32;
    return Vector( x, y );
  } else {
    return Vector(0, 0);
  }
}

Vector
EditorInputCenter::tile_screen_pos(Vector tp) {
  Vector sp = tp_to_sp(tp);
  return sp - Editor::current()->currentsector->camera->get_translation();
}

/* EOF */
