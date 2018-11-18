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

#include "editor/editor.hpp"
#include "editor/node_marker.hpp"
#include "editor/object_menu.hpp"
#include "editor/tile_selection.hpp"
#include "editor/tip.hpp"
#include "editor/util.hpp"
#include "editor/worldmap_objects.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/sector.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

namespace {

  const int snap_grid_sizes[4] = {4, 8, 16, 32};

} // namespace

bool EditorInputCenter::render_background = true;
bool EditorInputCenter::render_grid = true;
bool EditorInputCenter::snap_to_grid = false;
int EditorInputCenter::selected_snap_grid_size = 3;

EditorInputCenter::EditorInputCenter(Editor& editor) :
  m_editor(editor),
  m_hovered_tile(0, 0),
  m_sector_pos(0, 0),
  m_mouse_pos(0, 0),
  m_dragging(false),
  m_dragging_right(false),
  m_drag_start(0, 0),
  m_dragged_object(nullptr),
  m_hovered_object(nullptr),
  m_marked_object(nullptr),
  m_edited_path(nullptr),
  m_last_node_marker(nullptr),
  m_object_tip(),
  m_obj_mouse_desync(0, 0)
{
}

EditorInputCenter::~EditorInputCenter()
{
}

void
EditorInputCenter::update(float dt_sec)
{
  if (m_hovered_object && !m_hovered_object->is_valid()) {
    m_hovered_object = nullptr;
    m_object_tip = nullptr;
  }

  if (m_marked_object && !m_marked_object->is_valid()) {
    delete_markers();
  }

  if (m_edited_path && !m_edited_path->is_valid()) {
    delete_markers();
  }
}

void
EditorInputCenter::delete_markers()
{
  auto* sector = m_editor.get_sector();
  for (auto& moving_object : sector->get_objects_by_type<MovingObject>()) {
    auto marker = dynamic_cast<PointMarker*>(&moving_object);
    if (marker) {
      marker->remove_me();
    }
  }
  m_marked_object = nullptr;
  m_edited_path = nullptr;
  m_last_node_marker = nullptr;
}

Rectf
EditorInputCenter::drag_rect()
{
  int start_x, start_y, end_x, end_y;

  if (m_drag_start.x < m_sector_pos.x) {
    start_x = static_cast<int>(m_drag_start.x);
    end_x = static_cast<int>(m_sector_pos.x);
  } else {
    start_x = static_cast<int>(m_sector_pos.x);
    end_x = static_cast<int>(m_drag_start.x);
  }

  if (m_drag_start.y < m_sector_pos.y) {
    start_y = static_cast<int>(m_drag_start.y);
    end_y = static_cast<int>(m_sector_pos.y);
  } else {
    start_y = static_cast<int>(m_sector_pos.y);
    end_y = static_cast<int>(m_drag_start.y);
  }

  return Rectf( static_cast<float>(start_x),
                static_cast<float>(start_y),
                static_cast<float>(end_x),
                static_cast<float>(end_y) );
}

void
EditorInputCenter::input_tile(const Vector& pos, uint32_t tile)
{
  auto tilemap = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if ( !tilemap ) {
    return;
  }

  if ( pos.x < 0 ||
       pos.y < 0 ||
       pos.x >= static_cast<float>(tilemap->get_width()) ||
       pos.y >= static_cast<float>(tilemap->get_height())) {
    return;
  }

  tilemap->change(static_cast<int>(pos.x), static_cast<int>(pos.y), tile);
}

void
EditorInputCenter::put_tile()
{
  auto tiles = m_editor.get_tiles();
  Vector add_tile;
  for (add_tile.x = static_cast<float>(tiles->m_width) - 1.0f; add_tile.x >= 0.0f; add_tile.x--) {
    for (add_tile.y = static_cast<float>(tiles->m_height) - 1.0f; add_tile.y >= 0; add_tile.y--) {
      input_tile(m_hovered_tile + add_tile, tiles->pos(static_cast<int>(add_tile.x),
                                                     static_cast<int>(add_tile.y)));
    }
  }
}

void
EditorInputCenter::draw_rectangle()
{
  Rectf dr = drag_rect();
  dr.p1 = sp_to_tp(dr.p1);
  dr.p2 = sp_to_tp(dr.p2);
  bool sgn_x = m_drag_start.x < m_sector_pos.x;
  bool sgn_y = m_drag_start.y < m_sector_pos.y;

  int x_ = sgn_x ? 0 : static_cast<int>(-dr.get_width());
  for (int x = static_cast<int>(dr.p1.x); x <= static_cast<int>(dr.p2.x); x++, x_++) {
    int y_ = sgn_y ? 0 : static_cast<int>(-dr.get_height());
    for (int y = static_cast<int>(dr.p1.y); y <= static_cast<int>(dr.p2.y); y++, y_++) {
      input_tile( Vector(static_cast<float>(x), static_cast<float>(y)), m_editor.get_tiles()->pos(x_, y_) );
    }
  }
}

void
EditorInputCenter::fill()
{
  auto tiles = m_editor.get_tiles();
  auto tilemap = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if (! tilemap) {
    return;
  }

  // The tile that is going to be replaced:
  Uint32 replace_tile = tilemap->get_tile_id(static_cast<int>(m_hovered_tile.x), static_cast<int>(m_hovered_tile.y));

  if (replace_tile == tiles->pos(0, 0)) {
    // Replacing by the same tiles shouldn't do anything.
    return;
  }

  std::vector<Vector> pos_stack;
  pos_stack.clear();
  pos_stack.push_back(m_hovered_tile);

  // Passing recursively trough all tiles to be replaced...
  while (pos_stack.size()) {

    if (pos_stack.size() > 1000000) {
      log_warning << "More than 1'000'000 tiles in stack to fill, STOP" << std::endl;
      return;
    }

    Vector pos = pos_stack[pos_stack.size() - 1];
    Vector tpos = pos - m_hovered_tile;

    // Tests for being inside tilemap:
    if ( pos.x < 0 ||
         pos.y < 0 ||
         pos.x >= static_cast<float>(tilemap->get_width()) ||
         pos.y >= static_cast<float>(tilemap->get_height()))
    {
      pos_stack.pop_back();
      continue;
    }

    input_tile(pos, tiles->pos(static_cast<int>(tpos.x), static_cast<int>(tpos.y)));
    Vector pos_;

    // Going left...
    pos_ = pos + Vector(-1, 0);
    if (pos_.x >= 0) {
      if (replace_tile == tilemap->get_tile_id(static_cast<int>(pos_.x), static_cast<int>(pos_.y)) &&
          replace_tile != tiles->pos(static_cast<int>(tpos.x - 1), static_cast<int>(tpos.y))) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going right...
    pos_ = pos + Vector(1, 0);
    if (pos_.x < static_cast<float>(tilemap->get_width())) {
      if (replace_tile == tilemap->get_tile_id(static_cast<int>(pos_.x), static_cast<int>(pos_.y)) &&
          replace_tile != tiles->pos(static_cast<int>(tpos.x + 1), static_cast<int>(tpos.y))) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going up...
    pos_ = pos + Vector(0, -1);
    if (pos_.y >= 0) {
      if (replace_tile == tilemap->get_tile_id(static_cast<int>(pos_.x), static_cast<int>(pos_.y))&&
          replace_tile != tiles->pos(static_cast<int>(tpos.x), static_cast<int>(tpos.y - 1))) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // Going down...
    pos_ = pos + Vector(0, 1);
    if (pos_.y < static_cast<float>(tilemap->get_height())) {
      if (replace_tile == tilemap->get_tile_id(static_cast<int>(pos_.x), static_cast<int>(pos_.y)) &&
          replace_tile != tiles->pos(static_cast<int>(tpos.x), static_cast<int>(tpos.y + 1))) {
        pos_stack.push_back( pos_ );
        continue;
      }
    }

    // When tiles on each side are already filled or occupied by another tiles, it ends.
    pos_stack.pop_back();
  }
}

void
EditorInputCenter::hover_object()
{
  for (auto& moving_object : m_editor.get_sector()->get_objects_by_type<MovingObject>()) {
    auto pm = dynamic_cast<PointMarker*>(&moving_object);
    if (!moving_object.is_saveable() && !pm) {
      continue;
    }
    Rectf bbox = moving_object.get_bbox();
    if (bbox.contains(m_sector_pos)) {
      if (&moving_object != m_hovered_object) {
        if (moving_object.is_saveable()) {
          auto new_tip = std::make_unique<Tip>(&moving_object);
          m_object_tip = std::move(new_tip);
        }
        m_hovered_object = &moving_object;
      }
      return;
    }
  }
  m_object_tip = nullptr;
  m_hovered_object = nullptr;
}

void
EditorInputCenter::edit_path(Path* path, GameObject* new_marked_object) {
  if (!path) return;
  delete_markers();

  if (!path->is_valid()) {
    m_edited_path = nullptr;
    return;
  }
  m_edited_path = path;
  m_edited_path->edit_path();
  if (new_marked_object) {
    m_marked_object = new_marked_object;
  }
}

void
EditorInputCenter::mark_object()
{
  delete_markers();
  if (!m_dragged_object || !m_dragged_object->is_valid()) return;

  if (m_dragged_object->has_variable_size()) {
    m_marked_object = m_dragged_object;
    m_dragged_object->edit_bbox();
    return;
  }

  auto path_obj = dynamic_cast<PathObject*>(m_dragged_object);
  if (path_obj && path_obj->get_path())
  {
    edit_path(path_obj->get_path(), m_dragged_object);
  }
}

void
EditorInputCenter::grab_object()
{
  if (m_hovered_object) {
    if (!m_hovered_object->is_valid()) {
      m_hovered_object = nullptr;
      return;
    }

    m_dragged_object = m_hovered_object;
    auto pm = dynamic_cast<PointMarker*>(m_hovered_object);
    m_obj_mouse_desync = m_sector_pos - m_hovered_object->get_pos();
    // marker testing
    if (!pm) {
      mark_object();
    }
    m_last_node_marker = dynamic_cast<NodeMarker*>(pm);
    return;
  }
  m_dragged_object = nullptr;
  if (m_edited_path && m_editor.get_tileselect_object() == "#node") {
    if (m_edited_path->is_valid()) {
      return;
    }
  }
  delete_markers();
}

void
EditorInputCenter::clone_object()
{
  if (m_hovered_object && m_hovered_object->is_saveable()) {
    if (!m_hovered_object->is_valid()) {
      m_hovered_object = nullptr;
      return;
    }

    auto pm = dynamic_cast<PointMarker*>(m_hovered_object);
    if (pm) {
      return; //Do not clone markers
    }
    m_obj_mouse_desync = m_sector_pos - m_hovered_object->get_pos();

    std::unique_ptr<GameObject> game_object_uptr;
    try {
      game_object_uptr = GameObjectFactory::instance().create(m_hovered_object->get_class(), m_hovered_object->get_pos());
    } catch(const std::exception& e) {
      log_warning << "Error creating object " << m_hovered_object->get_class() << ": " << e.what() << std::endl;
      return;
    }

    GameObject& game_object = m_editor.get_sector()->add_object(std::move(game_object_uptr));

    m_dragged_object = dynamic_cast<MovingObject*>(&game_object);
    ObjectSettings settings = m_hovered_object->get_settings();
    m_dragged_object->get_settings().copy_from(settings);
    m_dragged_object->after_editor_set();
    return;
  }
  m_dragged_object = nullptr;
  delete_markers();
}

void
EditorInputCenter::set_object()
{
  if (m_hovered_object &&
      m_hovered_object->is_valid() &&
      m_hovered_object->is_saveable())
  {
    auto om = std::make_unique<ObjectMenu>(m_editor, m_hovered_object);
    m_editor.deactivate_request = true;
    MenuManager::instance().push_menu(std::move(om));
    return;
  }
}

void
EditorInputCenter::move_object()
{
  if (m_dragged_object) {
    if (!m_dragged_object->is_valid()) {
      m_dragged_object = nullptr;
      return;
    }
    Vector new_pos = m_sector_pos - m_obj_mouse_desync;
    if (snap_to_grid) {
      auto& snap_grid_size = snap_grid_sizes[selected_snap_grid_size];
      new_pos = (new_pos / static_cast<float>(snap_grid_size)).to_int_vec() * static_cast<float>(snap_grid_size);

      auto pm = dynamic_cast<PointMarker*>(m_dragged_object);
      if (pm) {
        new_pos -= pm->get_offset();
      }
    }
    m_dragged_object->move_to(new_pos);
  }
}

void
EditorInputCenter::rubber_object()
{
  if (!m_edited_path) {
    delete_markers();
  }
  if (m_dragged_object) {
    m_dragged_object->editor_delete();
  }
  m_last_node_marker = nullptr;
}

void
EditorInputCenter::rubber_rect()
{
  delete_markers();
  Rectf dr = drag_rect();
  for (auto& moving_object : m_editor.get_sector()->get_objects_by_type<MovingObject>()) {
    Rectf bbox = moving_object.get_bbox();
    if (dr.contains(bbox)) {
      moving_object.editor_delete();
    }
  }
  m_last_node_marker = nullptr;
}

void
EditorInputCenter::update_node_iterators()
{
  if (!m_edited_path) return;
  if (!m_edited_path->is_valid()) return;

  auto* sector = m_editor.get_sector();
  for (auto& moving_object : sector->get_objects_by_type<MovingObject>()) {
    auto marker = dynamic_cast<NodeMarker*>(&moving_object);
    if (marker) {
      marker->update_iterator();
    }
  }
}

void
EditorInputCenter::add_path_node()
{
  Path::Node new_node;
  new_node.position = m_sector_pos;
  new_node.time = 1;
  m_edited_path->m_nodes.insert(m_last_node_marker->m_node + 1, new_node);
  Sector::get().add<NodeMarker>(m_edited_path, m_edited_path->m_nodes.end() - 1, m_edited_path->m_nodes.size() - 1);
  //last_node_marker = dynamic_cast<NodeMarker*>(marker.get());
  update_node_iterators();
  m_editor.get_sector()->update(0);
  grab_object();
}

void
EditorInputCenter::put_object()
{
  const std::string& obj = m_editor.get_tileselect_object();
  if (obj[0] == '#') {
    if (m_edited_path && obj == "#node") {
      if (m_edited_path->is_valid() && m_last_node_marker) {
        add_path_node();
      }
    }
    return;
  }
  std::unique_ptr<GameObject> game_object;
  try {
    auto target_pos = m_sector_pos;
    if (snap_to_grid)
    {
      auto& snap_grid_size = snap_grid_sizes[selected_snap_grid_size];
      target_pos = (m_sector_pos / static_cast<float>(snap_grid_size)).to_int_vec() * static_cast<float>(snap_grid_size);
    }
    game_object = GameObjectFactory::instance().create(obj, target_pos, LEFT);
  } catch(const std::exception& e) {
    log_warning << "Error creating object " << obj << ": " << e.what() << std::endl;
    return;
  }
  if (game_object == nullptr)
    throw std::runtime_error("Creating " + obj + " object failed.");

  auto mo = dynamic_cast<MovingObject*> (game_object.get());
  if (!mo) {
    m_editor.add_layer(game_object.get());
  }
  else if (!snap_to_grid) {
    auto bbox = mo->get_bbox();
    mo->move_to(mo->get_pos() - Vector(bbox.get_width() / 2, bbox.get_height() / 2));
  }

  auto wo = dynamic_cast<worldmap_editor::WorldmapObject*> (game_object.get());
  if (wo) {
    wo->move_to(wo->get_pos() / 32);
  }

  try {
    m_editor.get_sector()->add_object(std::move(game_object));
  } catch(const std::exception& e) {
    log_warning << "Error adding object " << obj << ": " << e.what() << std::endl;
    return;
  }
}

void
EditorInputCenter::process_left_click()
{
  m_dragging = true;
  m_dragging_right = false;
  m_drag_start = m_sector_pos;

  switch (m_editor.get_tileselect_input_type())
  {
    case EditorInputGui::IP_TILE:
      switch (m_editor.get_tileselect_select_mode())
      {
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
      break;

    case EditorInputGui::IP_OBJECT:
      switch (m_editor.get_tileselect_move_mode())
      {
        case 0:
          grab_object();
          break;

        case 1:
          clone_object();
          break;

        default:
          break;
      }

      if (!m_editor.get_tileselect_object().empty()) {
        if (!m_dragged_object) {
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
EditorInputCenter::process_right_click()
{
  switch (m_editor.get_tileselect_input_type())
  {
    case EditorInputGui::IP_TILE:
      m_dragging = true;
      m_dragging_right = true;
      m_drag_start = m_sector_pos;
      update_tile_selection();
      break;

    case EditorInputGui::IP_NONE:
    case EditorInputGui::IP_OBJECT:
      set_object();
      break;

    default:
      break;
  }
}

Rectf
EditorInputCenter::tile_drag_rect()
{
  Rectf result = drag_rect();

  // Increase drag rectangle size to the
  // nearest tile border respectively.
  result = Rectf(floorf(result.p1.x / 32) * 32,
                 floorf(result.p1.y / 32) * 32,
                 ceilf(result.p2.x / 32) * 32,
                 ceilf(result.p2.y / 32) * 32);
  result.p1 = sp_to_tp(result.p1);
  result.p2 = sp_to_tp(result.p2);
  return result;
}

Rectf
EditorInputCenter::selection_draw_rect()
{
  Rectf select = tile_drag_rect();
  select.p1 = tile_screen_pos(select.p1);
  select.p2 = tile_screen_pos(select.p2);
  return select;
}

void
EditorInputCenter::update_tile_selection()
{
  Rectf select = tile_drag_rect();
  auto tiles = m_editor.get_tiles();
  auto tilemap = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if ( !tilemap ) {
    return;
  }

  tiles->m_tiles.clear();
  tiles->m_width = static_cast<int>(select.get_width());
  tiles->m_height = static_cast<int>(select.get_height());

  int w = static_cast<int>(tilemap->get_width());
  int h = static_cast<int>(tilemap->get_height());
  for (int y = static_cast<int>(select.p1.y); y < static_cast<int>(select.p2.y); y++) {
    for (int x = static_cast<int>(select.p1.x); x < static_cast<int>(select.p2.x); x++) {
      if ( x < 0 || y < 0 || x >= w || y >= h) {
        tiles->m_tiles.push_back(0);
      } else {
        tiles->m_tiles.push_back(tilemap->get_tile_id(x, y));
      }
    }
  }
}

bool
EditorInputCenter::on_mouse_button_up(const SDL_MouseButtonEvent& button)
{
  m_dragging = false;
  return true;
}

bool
EditorInputCenter::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  switch (button.button)
  {
    case SDL_BUTTON_LEFT:
      process_left_click();
      return true;

    case SDL_BUTTON_RIGHT:
      process_right_click();
      return true;

    default:
      return false;
  }
}

bool
EditorInputCenter::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(motion.x, motion.y);
  update_pos();

  if (m_dragging)
  {
    switch (m_editor.get_tileselect_input_type())
    {
      case EditorInputGui::IP_TILE:
        if (m_dragging_right) {
          update_tile_selection();
        } else {
          switch (m_editor.get_tileselect_select_mode()) {
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
        if (m_editor.get_tileselect_object().empty()) {
          if (m_editor.get_tileselect_select_mode() == 1) {
            rubber_rect();
          }
        } else {
          move_object();
        }
        break;

      default:
        break;
    }
    return true;
  }
  else
  {
    return false;
  }
}

bool
EditorInputCenter::on_key_up(const SDL_KeyboardEvent& key)
{
  auto sym = key.keysym.sym;
  if (sym == SDLK_LSHIFT || sym == SDLK_RSHIFT)
  {
    snap_to_grid = !snap_to_grid;
  }
  return true;
}

bool
EditorInputCenter::on_key_down(const SDL_KeyboardEvent& key)
{
  auto sym = key.keysym.sym;
  if (sym == SDLK_F8) {
    render_grid = !render_grid;
  }
  if (sym == SDLK_F7 || sym == SDLK_LSHIFT || sym == SDLK_RSHIFT) {
    snap_to_grid = !snap_to_grid;
  }
  return true;
}

void
EditorInputCenter::update_pos()
{
  m_sector_pos = m_mouse_pos + m_editor.get_sector()->get_camera().get_translation();
  m_hovered_tile = sp_to_tp(m_sector_pos);
  // update tip
  hover_object();
}

void
EditorInputCenter::draw_tile_tip(DrawingContext& context)
{
  if ( m_editor.get_tileselect_input_type() == EditorInputGui::IP_TILE ) {

    auto tilemap = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
    if (!tilemap) {
      return;
    }

    Vector drawn_tile = m_hovered_tile;
    auto tiles = m_editor.get_tiles();

    for (drawn_tile.x = static_cast<float>(tiles->m_width) - 1.0f; drawn_tile.x >= 0.0f; drawn_tile.x--) {
      for (drawn_tile.y = static_cast<float>(tiles->m_height) - 1.0f; drawn_tile.y >= 0.0f; drawn_tile.y--) {
        Vector on_tile = m_hovered_tile + drawn_tile;

        if (m_editor.get_tiles()->empty() ||
            on_tile.x < 0 ||
            on_tile.y < 0 ||
            on_tile.x >= static_cast<float>(tilemap->get_width()) ||
            on_tile.y >= static_cast<float>(tilemap->get_height())) {
          continue;
        }
        uint32_t tile_id = tiles->pos(static_cast<int>(drawn_tile.x), static_cast<int>(drawn_tile.y));
        draw_tile(context.color(), *m_editor.get_tileset(), tile_id,
                  tp_to_sp(on_tile) - m_editor.get_sector()->get_camera().get_translation(),
                  LAYER_GUI-11, Color(1, 1, 1, 0.5));
        /*if (tile_id) {
          const Tile* tg_tile = m_editor.get_tileset()->get( tile_id );
          tg_tile->draw(context.color(), tp_to_sp(on_tile) - m_editor.get_sector()->camera->get_translation(),
                        LAYER_GUI-11, Color(1, 1, 1, 0.5));
        }*/
      }
    }
  }
}

void
EditorInputCenter::draw_tile_grid(DrawingContext& context, const Color& line_color, int tile_size)
{
  if ( !m_editor.get_selected_tilemap() ) {
    return;
  }

  auto current_tm = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if ( current_tm == nullptr )
    return;
  int tm_width = current_tm->get_width() * (32 / tile_size);
  int tm_height = current_tm->get_height() * (32 / tile_size);
  auto cam_translation = m_editor.get_sector()->get_camera().get_translation();
  Rectf draw_rect = Rectf(cam_translation, cam_translation +
                          Vector(static_cast<float>(context.get_width()),
                                 static_cast<float>(context.get_height())));
  Vector start = sp_to_tp( Vector(draw_rect.p1.x, draw_rect.p1.y), tile_size );
  Vector end = sp_to_tp( Vector(draw_rect.p2.x, draw_rect.p2.y), tile_size );
  start.x = std::max(0.0f, start.x);
  start.y = std::max(0.0f, start.y);
  end.x = std::min(float(tm_width-1), end.x);
  end.y = std::min(float(tm_height-1), end.y);

  Vector line_start, line_end;
  for (int i = static_cast<int>(start.x); i <= static_cast<int>(end.x); i++) {
    line_start = tile_screen_pos( Vector(static_cast<float>(i), 0.0f), tile_size );
    line_end = tile_screen_pos( Vector(static_cast<float>(i), static_cast<float>(tm_height)), tile_size );
    context.color().draw_line(line_start, line_end, line_color, current_tm->get_layer());
  }

  for (int i = static_cast<int>(start.y); i <= static_cast<int>(end.y); i++) {
    line_start = tile_screen_pos( Vector(0.0f, static_cast<float>(i)), tile_size );
    line_end = tile_screen_pos( Vector(static_cast<float>(tm_width), static_cast<float>(i)), tile_size );
    context.color().draw_line(line_start, line_end, line_color, current_tm->get_layer());
  }
}

void
EditorInputCenter::draw_tilemap_border(DrawingContext& context)
{
  if ( !m_editor.get_selected_tilemap() ) return;

  auto current_tm = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if ( !current_tm ) return;

  Vector start = tile_screen_pos( Vector(0, 0) );
  Vector end = tile_screen_pos( Vector(static_cast<float>(current_tm->get_width()),
                                       static_cast<float>(current_tm->get_height())) );
  context.color().draw_line(start, Vector(start.x, end.y), Color(1, 0, 1), current_tm->get_layer());
  context.color().draw_line(start, Vector(end.x, start.y), Color(1, 0, 1), current_tm->get_layer());
  context.color().draw_line(Vector(start.x, end.y), end, Color(1, 0, 1), current_tm->get_layer());
  context.color().draw_line(Vector(end.x, start.y), end, Color(1, 0, 1), current_tm->get_layer());
}

void
EditorInputCenter::draw_path(DrawingContext& context)
{
  if (!m_edited_path) return;
  if (!m_marked_object) return;
  if (!m_marked_object->is_valid()) return;
  if (!m_edited_path->is_valid()) return;

  for (auto i = m_edited_path->m_nodes.begin(); i != m_edited_path->m_nodes.end(); ++i) {
    auto j = i+1;
    Path::Node* node1 = &(*i);
    Path::Node* node2;
    if (j == m_edited_path->m_nodes.end()) {
      if (m_edited_path->m_mode == WalkMode::CIRCULAR || m_edited_path->m_mode == WalkMode::UNORDERED) {
        //loop to the first node
        node2 = &(*m_edited_path->m_nodes.begin());
      } else {
        continue;
      }
    } else {
      node2 = &(*j);
    }
    auto cam_translation = m_editor.get_sector()->get_camera().get_translation();
    context.color().draw_line(node1->position - cam_translation,
                              node2->position - cam_translation,
                              Color(1, 0, 0), LAYER_GUI - 21);
  }
}

void
EditorInputCenter::draw(DrawingContext& context)
{
  draw_tile_tip(context);
  draw_path(context);

  if (render_grid) {
    draw_tile_grid(context, Color(1.f, 1.f, 1.f, 0.7f));
    draw_tilemap_border(context);
    auto snap_grid_size = snap_grid_sizes[selected_snap_grid_size];
    if (snap_grid_size != 32) {
      draw_tile_grid(context, Color(1.f, 1.f, 1.f, 0.4f), snap_grid_size);
    }
  }

  if (m_object_tip) {
    m_object_tip->draw(context, m_mouse_pos);
  }

  if (m_dragging && m_editor.get_tileselect_select_mode() == 1
      && !m_dragging_right) {
    // Draw selection rectangle...
    auto cam_translation = m_editor.get_sector()->get_camera().get_translation();
    Vector p0 = m_drag_start - cam_translation;
    Vector p1 = Vector(m_drag_start.x, m_sector_pos.y) - cam_translation;
    Vector p2 = Vector(m_sector_pos.x, m_drag_start.y) - cam_translation;

    context.color().draw_filled_rect(Rectf(p0, p1 + Vector(2, 2)),
                                       Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);
    context.color().draw_filled_rect(Rectf(p2, m_mouse_pos + Vector(2, 2)),
                                       Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);
    context.color().draw_filled_rect(Rectf(p0, p2 + Vector(2, 2)),
                                       Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);
    context.color().draw_filled_rect(Rectf(p1, m_mouse_pos + Vector(2, 2)),
                                       Color(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, LAYER_GUI-5);

    context.color().draw_filled_rect(Rectf(p0, m_mouse_pos),
                                       Color(0.0f, 1.0f, 0.0f, 0.2f), 0.0f, LAYER_GUI-5);
  }

  if (m_dragging && m_dragging_right) {
    context.color().draw_filled_rect(selection_draw_rect(),
                                       Color(0.2f, 0.4f, 1.0f, 0.6f), 0.0f, LAYER_GUI-13);
  }
}

Vector
EditorInputCenter::tp_to_sp(const Vector& tp, int tile_size)
{
  auto tilemap = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if (!tilemap)
  {
    return Vector(0, 0);
  }

  Vector sp = tp * static_cast<float>(tile_size);
  return sp + tilemap->get_offset();
}

Vector
EditorInputCenter::sp_to_tp(const Vector& sp, int tile_size)
{
  auto tilemap = dynamic_cast<TileMap*>(m_editor.get_selected_tilemap());
  if (!tilemap)
  {
    return Vector(0, 0);
  }

  Vector sp_ = sp - tilemap->get_offset();
  return sp_ / static_cast<float>(tile_size);
}

Vector
EditorInputCenter::tile_screen_pos(const Vector& tp, int tile_size)
{
  Vector sp = tp_to_sp(tp, tile_size);
  return sp - m_editor.get_sector()->get_camera().get_translation();
}

/* EOF */
