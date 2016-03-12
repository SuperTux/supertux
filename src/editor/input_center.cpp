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

#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "editor/object_menu.hpp"
#include "editor/tool_icon.hpp"
#include "editor/tip.hpp"
#include "math/rectf.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
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
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "math/vector.hpp"

EditorInputCenter::EditorInputCenter() :
  hovered_tile(0, 0),
  sector_pos(0, 0),
  mouse_pos(0, 0),
  dragging(false),
  drag_start(0, 0),
  dragged_object(NULL),
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
EditorInputCenter::input_tile(Vector pos) {
  if ( !Editor::current()->layerselect.selected_tilemap ) {
    return;
  }

  if ( pos.x < 0 || pos.y < 0 ||
       pos.x >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_width() ||
       pos.y >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_height()) {
    return;
  }

  ((TileMap *)Editor::current()->layerselect.selected_tilemap)->change(pos.x, pos.y,
                                                                       Editor::current()->tileselect.tile);
}

void
EditorInputCenter::put_tile() {
  input_tile(hovered_tile);
}

void
EditorInputCenter::draw_rectangle() {

  Rectf dr = drag_rect();
  dr.p1 = sp_to_tp(dr.p1);
  dr.p2 = sp_to_tp(dr.p2);

  for (int x = dr.p1.x; x <= dr.p2.x; x++) {
    for (int y = dr.p1.y; y <= dr.p2.y; y++) {
      input_tile( Vector(x,y) );
    }
  }
}

void
EditorInputCenter::fill() {

  if (! Editor::current()->layerselect.selected_tilemap) {
    return;
  }

  // The tile that is going to be replaced:
  Uint32 replace_tile =
      ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_tile_id(hovered_tile.x, hovered_tile.y);

  if (Editor::current()->tileselect.tile ==
      ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_tile_id(hovered_tile.x, hovered_tile.y)) {
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

    // Tests for being inside tilemap:
    if ( pos.x < 0 || pos.y < 0 ||
         pos.x >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_width() ||
         pos.y >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_height()) {
      pos_stack.pop_back();
      continue;
    }

    input_tile (pos);
    Vector pos_;

    // Going left...
    pos_ = pos + Vector(-1, 0);
    if (pos_.x >= 0) {
      if (replace_tile == ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_tile_id(pos_.x, pos_.y)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going right...
    pos_ = pos + Vector(1, 0);
    if (pos_.x < ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_width()) {
      if (replace_tile == ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_tile_id(pos_.x, pos_.y)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going up...
    pos_ = pos + Vector(0, -1);
    if (pos_.y >= 0) {
      if (replace_tile == ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_tile_id(pos_.x, pos_.y)) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going down...
    pos_ = pos + Vector(0, 1);
    if (pos_.y < ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_height()) {
      if (replace_tile == ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_tile_id(pos_.x, pos_.y)) {
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
  for (auto i = Editor::current()->currentsector->moving_objects.begin();
       i != Editor::current()->currentsector->moving_objects.end(); ++i) {
    MovingObject* moving_object = *i;
    if (!moving_object->do_save()) {
      continue;
    }
    Rectf bbox = moving_object->get_bbox();
    if (sector_pos.x >= bbox.p1.x && sector_pos.y >= bbox.p1.y &&
        sector_pos.x <= bbox.p2.x && sector_pos.y <= bbox.p2.y ) {
      std::unique_ptr<Tip> new_tip(new Tip(moving_object));
      object_tip = move(new_tip);
      return;
    }
  }
  object_tip = NULL;
}

void
EditorInputCenter::grab_object() {
  for (auto i = Editor::current()->currentsector->moving_objects.begin();
      i != Editor::current()->currentsector->moving_objects.end(); ++i) {
    MovingObject* moving_object = *i;
    Rectf bbox = moving_object->get_bbox();
    if (sector_pos.x >= bbox.p1.x && sector_pos.y >= bbox.p1.y &&
        sector_pos.x <= bbox.p2.x && sector_pos.y <= bbox.p2.y ) {
      dragged_object = moving_object;
      obj_mouse_desync = sector_pos - bbox.p1;
      return;
    }
  }
  dragged_object = NULL;
}

void
EditorInputCenter::set_object() {
  for (auto i = Editor::current()->currentsector->moving_objects.begin();
      i != Editor::current()->currentsector->moving_objects.end(); ++i) {
    MovingObject* moving_object = *i;
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
    dragged_object->set_pos(sector_pos - obj_mouse_desync);
  }
}

void
EditorInputCenter::rubber_object() {
  if (dragged_object) {
    dragged_object->remove_me();
  }
}

void
EditorInputCenter::rubber_rect() {
  Rectf dr = drag_rect();
  for (auto i = Editor::current()->currentsector->moving_objects.begin();
      i != Editor::current()->currentsector->moving_objects.end(); ++i) {
    MovingObject* moving_object = *i;
    Rectf bbox = moving_object->get_bbox();
    if (bbox.p2.x >= dr.p1.x && bbox.p1.x <= dr.p2.x &&
        bbox.p2.y >= dr.p1.y && bbox.p1.y <= dr.p2.y ) {
      moving_object->remove_me();
    }
  }
}

void
EditorInputCenter::put_object() {
  if (Editor::current()->tileselect.object[0] == '#') {
    return;
  }
  GameObjectPtr game_object;
  try {
    game_object = ObjectFactory::instance().create(Editor::current()->tileselect.object, sector_pos, LEFT);
  } catch(const std::exception& e) {
    log_warning << "Error creating object " << Editor::current()->tileselect.object << ": " << e.what() << std::endl;
    return;
  }
  if (game_object == NULL)
    throw std::runtime_error("Creating " + Editor::current()->tileselect.object + " object failed.");

  MovingObject* mo = dynamic_cast<MovingObject*> (game_object.get());
  if (!mo) {
    Editor::current()->layerselect.add_layer(game_object.get());
  }

  try {
    Editor::current()->currentsector->add_object(game_object);
  } catch(const std::exception& e) {
    log_warning << "Error adding object " << Editor::current()->tileselect.object << ": " << e.what() << std::endl;
    return;
  }
}

void
EditorInputCenter::process_left_click() {
  dragging = true;
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
      if (Editor::current()->tileselect.object != "") {
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
      //possible future usage
    } break;
    case EditorInputGui::IP_OBJECT:
      set_object();
      break;
    default:
      break;
  }
}

void
EditorInputCenter::event(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    switch (ev.button.button) {
      case SDL_BUTTON_LEFT: {
        //TODO: Make the right clicks working.
        if (InputManager::current()->get_controller()->hold(Controller::ACTION)) {
          InputManager::current()->get_controller()->set_control(Controller::ACTION,false);
          process_right_click();
        } else {
          process_left_click();
        }
      } break;
      case SDL_BUTTON_RIGHT: {
        process_right_click();
      } break;
    } break;
    case SDL_MOUSEBUTTONUP: if(ev.button.button == SDL_BUTTON_LEFT) {
      dragging = false;
      if (Editor::current()->tileselect.input_type == EditorInputGui::IP_OBJECT) {
        if (Editor::current()->tileselect.select_mode->get_mode() == 1 &&
            Editor::current()->tileselect.object == "" ) {
          rubber_rect();
        }
      }
    } break;

    case SDL_MOUSEMOTION:
    {
      mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      actualize_pos();
      actualize_scrolling();
      if (dragging) {
        switch (Editor::current()->tileselect.input_type) {
          case EditorInputGui::IP_TILE:
            switch (Editor::current()->tileselect.select_mode->get_mode()) {
              case 0:
                put_tile();
                break;
              case 1:
                draw_rectangle();
                break;
              default:
                break;
            }
            break;
          case EditorInputGui::IP_OBJECT:
            if (Editor::current()->tileselect.object == "") {
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
      if (ev.key.keysym.sym == SDLK_F8)
      {
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
  if ( Editor::current()->tileselect.input_type == EditorInputGui::IP_TILE ) {

    if ( !Editor::current()->layerselect.selected_tilemap ) {
      return;
    }

    if ( Editor::current()->tileselect.tile == 0 || hovered_tile.x < 0 || hovered_tile.y < 0 ||
         hovered_tile.x >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_width() ||
         hovered_tile.y >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_height()) {
      return;
    }

    context.push_transform();
    context.set_alpha(0.5);

    const Tile* tg_tile = Editor::current()->tileset->get( Editor::current()->tileselect.tile );
    tg_tile->draw(context, tp_to_sp(hovered_tile) - Editor::current()->currentsector->camera->get_translation(),
                  LAYER_GUI-11);

    context.pop_transform();
  }
}

void
EditorInputCenter::draw_tile_grid(DrawingContext& context) {
  if ( !Editor::current()->layerselect.selected_tilemap ) {
    return;
  }

  TileMap* current_tm = (TileMap*)Editor::current()->layerselect.selected_tilemap;
  int tm_width = current_tm->get_width();
  int tm_height = current_tm->get_height();
  Rectf draw_rect = Rectf(Editor::current()->currentsector->camera->get_translation(),
        Editor::current()->currentsector->camera->get_translation() + Vector(SCREEN_WIDTH, SCREEN_HEIGHT));
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

  start = tile_screen_pos( Vector(0,0) );
  end = tile_screen_pos( Vector(tm_width, tm_height) );
  context.draw_line(start, Vector(start.x, end.y), Color(1, 0, 1), current_tm->get_layer());
  context.draw_line(start, Vector(end.x, start.y), Color(1, 0, 1), current_tm->get_layer());
  context.draw_line(Vector(start.x, end.y), end, Color(1, 0, 1), current_tm->get_layer());
  context.draw_line(Vector(end.x, start.y), end, Color(1, 0, 1), current_tm->get_layer());
}

void
EditorInputCenter::draw(DrawingContext& context) {
  draw_tile_tip(context);

  if (render_grid) {
    draw_tile_grid(context);
  }

  if (object_tip) {
    object_tip->draw(context, mouse_pos);
  }

  if (dragging && Editor::current()->tileselect.select_mode->get_mode() == 1) {
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
}

Vector
EditorInputCenter::tp_to_sp(Vector tp) {
  if (Editor::current()->layerselect.selected_tilemap) {
    Vector sp = Vector( tp.x * 32, tp.y * 32 );
    // This line is cool :DDDDD
    return sp + ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_offset();
  } else {
    return Vector(0, 0);
  }
}

Vector
EditorInputCenter::sp_to_tp(Vector sp) {
  if (Editor::current()->layerselect.selected_tilemap) {
    // This line is cool :DDDDD
    sp -= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_offset();
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
