//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/tilemap.hpp"

#include <tuple>

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

#include "editor/editor.hpp"
#include "supertux/autotile.hpp"
#include "supertux/debug.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "collision/collision_object.hpp"
#include "collision/collision_movement_manager.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/layer.hpp"
#include "video/surface.hpp"

TileMap::TileMap(const TileSet *new_tileset) :
  PathObject(),
  m_editor_active(true),
  m_tileset(new_tileset),
  m_tiles(),
  m_real_solid(false),
  m_effective_solid(false),
  m_speed_x(1),
  m_speed_y(1),
  m_width(0),
  m_height(0),
  m_z_pos(0),
  m_offset(Vector(0,0)),
  m_movement(0, 0),
  m_objects_hit_bottom(),
  m_ground_movement_manager(nullptr),
  m_flip(NO_FLIP),
  m_alpha(1.0),
  m_current_alpha(1.0),
  m_remaining_fade_time(0),
  m_tint(1, 1, 1),
  m_current_tint(1, 1, 1),
  m_remaining_tint_fade_time(0),
  m_draw_target(DrawingTarget::COLORMAP),
  m_new_size_x(0),
  m_new_size_y(0),
  m_new_offset_x(0),
  m_new_offset_y(0),
  m_add_path(false),
  m_starting_node(0)
{
}

TileMap::TileMap(const TileSet *tileset_, const ReaderMapping& reader) :
  GameObject(reader),
  PathObject(),
  m_editor_active(true),
  m_tileset(tileset_),
  m_tiles(),
  m_real_solid(false),
  m_effective_solid(false),
  m_speed_x(1),
  m_speed_y(1),
  m_width(-1),
  m_height(-1),
  m_z_pos(0),
  m_offset(Vector(0, 0)),
  m_movement(Vector(0, 0)),
  m_objects_hit_bottom(),
  m_ground_movement_manager(nullptr),
  m_flip(NO_FLIP),
  m_alpha(1.0),
  m_current_alpha(1.0),
  m_remaining_fade_time(0),
  m_tint(1, 1, 1),
  m_current_tint(1, 1, 1),
  m_remaining_tint_fade_time(0),
  m_draw_target(DrawingTarget::COLORMAP),
  m_new_size_x(0),
  m_new_size_y(0),
  m_new_offset_x(0),
  m_new_offset_y(0),
  m_add_path(false),
  m_starting_node(0)
{
  assert(m_tileset);

  reader.get("solid",  m_real_solid);

  bool backward_compatibility_fudge = false;

  if (!reader.get("speed-x", m_speed_x)) {
    if (reader.get("speed",  m_speed_x)) {
      backward_compatibility_fudge = true;
    }
  }

  if (!reader.get("speed-y", m_speed_y)) {
    if (backward_compatibility_fudge) {
      m_speed_y = m_speed_x;
    }
  }

  m_z_pos = reader_get_layer(reader, 0);

  if (!Editor::is_active()) {
    if (m_real_solid && ((m_speed_x != 1) || (m_speed_y != 1))) {
      log_warning << "Speed of solid tilemap is not 1. fixing" << std::endl;
      m_speed_x = 1;
      m_speed_y = 1;
    }
  }

  reader.get("starting-node", m_starting_node, 0);

  init_path(reader, false);

  std::string draw_target_s = "normal";
  reader.get("draw-target", draw_target_s);
  if (draw_target_s == "normal") m_draw_target = DrawingTarget::COLORMAP;
  if (draw_target_s == "lightmap") m_draw_target = DrawingTarget::LIGHTMAP;

  if (reader.get("alpha", m_alpha)) {
    m_current_alpha = m_alpha;
  }

  std::vector<float> vColor;
  if (reader.get("tint", vColor)) {
    m_current_tint = Color(vColor);
    m_tint = m_current_tint;
  }

  /* Initialize effective_solid based on real_solid and current_alpha. */
  m_effective_solid = m_real_solid;
  update_effective_solid(false);

  reader.get("width", m_width);
  reader.get("height", m_height);
  if (m_width < 0 || m_height < 0) {
    //throw std::runtime_error("Invalid/No width/height specified in tilemap.");
    m_width = 0;
    m_height = 0;
    m_tiles.clear();
    resize(static_cast<int>(Sector::get().get_width() / 32.0f),
           static_cast<int>(Sector::get().get_height() / 32.0f));
    m_editor_active = false;
  } else {
    if (!reader.get("tiles", m_tiles))
      throw std::runtime_error("No tiles in tilemap.");

    if (int(m_tiles.size()) != m_width * m_height) {
      throw std::runtime_error("wrong number of tiles in tilemap.");
    }
  }

  bool empty = true;

  // make sure all tiles used on the tilemap are loaded and tilemap isn't empty
  for (const auto& tile : m_tiles) {
    if (tile != 0) {
      empty = false;
    }

    m_tileset->get(tile);
  }

  if (empty)
  {
    log_info << "Tilemap '" << get_name() << "', z-pos '" << m_z_pos << "' is empty." << std::endl;
  }
}

void
TileMap::finish_construction()
{
  get_parent()->update_solid(this);

  if (get_path() && get_path()->get_nodes().size() > 0) {
    if (m_starting_node >= static_cast<int>(get_path()->get_nodes().size()))
      m_starting_node = static_cast<int>(get_path()->get_nodes().size()) - 1;

    set_offset(m_path_handle.get_pos(get_size() * 32, get_path()->get_nodes()[m_starting_node].position));
    get_walker()->jump_to_node(m_starting_node);
  }

  m_add_path = get_walker() && get_path() && get_path()->is_valid();
}

TileMap::~TileMap()
{
}

void
TileMap::float_channel(float target, float &current, float remaining_time, float dt_sec)
{
  float amt = (target - current) / (remaining_time / dt_sec);
  if (amt > 0) current = std::min(current + amt, target);
  if (amt < 0) current = std::max(current + amt, target);
}

void
TileMap::apply_offset_x(int fill_id, int xoffset)
{
  if (!xoffset)
    return;
  for (int y = 0; y < m_height; y++) {
    for (int x = 0; x < m_width; x++) {
      int X = (xoffset < 0) ? x : (m_width - x - 1);
      if (X - xoffset < 0 || X - xoffset >= m_width) {
        m_tiles[y * m_width + X] = fill_id;
      } else {
        m_tiles[y * m_width + X] = m_tiles[y * m_width + X - xoffset];
      }
    }
  }
}

void
TileMap::apply_offset_y(int fill_id, int yoffset)
{
  if (!yoffset)
    return;
  for (int y = 0; y < m_height; y++) {
    int Y = (yoffset < 0) ? y : (m_height - y - 1);
    for (int x = 0; x < m_width; x++) {
      if (Y - yoffset < 0 || Y - yoffset >= m_height) {
        m_tiles[Y * m_width + x] = fill_id;
      } else {
        m_tiles[Y * m_width + x] = m_tiles[(Y - yoffset) * m_width + x];
      }
    }
  }
}

ObjectSettings
TileMap::get_settings()
{
  m_new_size_x = m_width;
  m_new_size_y = m_height;
  m_new_offset_x = 0;
  m_new_offset_y = 0;

  ObjectSettings result = GameObject::get_settings();

  result.add_bool(_("Solid"), &m_real_solid, "solid");
  result.add_int(_("Resize offset x"), &m_new_offset_x);
  result.add_int(_("Resize offset y"), &m_new_offset_y);

  result.add_int(_("Width"), &m_new_size_x);
  result.add_int(_("Height"), &m_new_size_y);

  result.add_float(_("Alpha"), &m_alpha, "alpha", 1.0f);
  result.add_float(_("Speed x"), &m_speed_x, "speed-x", 1.0f);
  result.add_float(_("Speed y"), &m_speed_y, "speed-y", 1.0f);
  result.add_color(_("Tint"), &m_tint, "tint", Color::WHITE);
  result.add_int(_("Z-pos"), &m_z_pos, "z-pos");
  result.add_enum(_("Draw target"), reinterpret_cast<int*>(&m_draw_target),
                  {_("Normal"), _("Lightmap")},
                  {"normal", "lightmap"},
                  static_cast<int>(DrawingTarget::COLORMAP),
                  "draw-target");

  result.add_path_ref(_("Path"), *this, get_path_ref(), "path-ref");
  result.add_int(_("Starting Node"), &m_starting_node, "starting-node", 0, 0U);
  m_add_path = get_walker() && get_path() && get_path()->is_valid();
  result.add_bool(_("Following path"), &m_add_path);

  if (get_walker() && get_path() && get_path()->is_valid()) {
    result.add_walk_mode(_("Path Mode"), &get_path()->m_mode, {}, {});
    result.add_bool(_("Adapt Speed"), &get_path()->m_adapt_speed, {}, {});
    result.add_bool(_("Running"), &get_walker()->m_running, "running", false);
    result.add_path_handle(_("Handle"), m_path_handle, "handle");
  }

  result.add_tiles(_("Tiles"), this, "tiles");

  result.reorder({"solid", "running", "speed-x", "speed-y", "tint", "draw-target", "alpha", "z-pos", "name", "path-ref", "width", "height", "tiles"});

  if (!m_editor_active) {
    result.add_remove();
  }

  return result;
}

void
TileMap::after_editor_set()
{
  if ((m_new_size_x != m_width || m_new_size_y != m_height ||
      m_new_offset_x || m_new_offset_y) &&
      m_new_size_x > 0 && m_new_size_y > 0) {
    resize(m_new_size_x, m_new_size_y, 0, m_new_offset_x, m_new_offset_y);
  }

  if (get_walker() && get_path() && get_path()->is_valid()) {
    if (!m_add_path) {
      get_path()->m_nodes.clear();
    }
  } else {
    if (m_add_path) {
      init_path_pos(m_offset);
    }
  }

  m_current_tint = m_tint;
  m_current_alpha = m_alpha;
}

void
TileMap::save_state()
{
  GameObject::save_state();
  PathObject::save_state();
}

void
TileMap::check_state()
{
  GameObject::check_state();
  PathObject::check_state();
}

void
TileMap::update(float dt_sec)
{
  // handle tilemap fading
  if (m_current_alpha != m_alpha) {
    m_remaining_fade_time = std::max(0.0f, m_remaining_fade_time - dt_sec);
    if (m_remaining_fade_time == 0.0f) {
      m_current_alpha = m_alpha;
    } else {
      float_channel(m_alpha, m_current_alpha, m_remaining_fade_time, dt_sec);
    }
    update_effective_solid ();
  }

  // handle tint fading
  if (m_current_tint != m_tint) {

    m_remaining_tint_fade_time = std::max(0.0f, m_remaining_tint_fade_time - dt_sec);
    if (m_remaining_tint_fade_time == 0.0f) {
      m_current_tint = m_tint;
    } else {
      float_channel(m_tint.red  , m_current_tint.red  , m_remaining_tint_fade_time, dt_sec);
      float_channel(m_tint.green, m_current_tint.green, m_remaining_tint_fade_time, dt_sec);
      float_channel(m_tint.blue , m_current_tint.blue , m_remaining_tint_fade_time, dt_sec);
      float_channel(m_tint.alpha, m_current_tint.alpha, m_remaining_tint_fade_time, dt_sec);
    }
  }

  // if we have a path to follow, follow it
  if (get_walker()) {
    m_movement = Vector(0, 0);
    get_walker()->update(dt_sec);
    Vector v = get_walker()->get_pos(get_size() * 32, m_path_handle);
    if (get_path() && get_path()->is_valid()) {
      m_movement = v - get_offset();
      set_offset(v);
      if (m_ground_movement_manager != nullptr) {
        for (CollisionObject* other_object : m_objects_hit_bottom) {
          m_ground_movement_manager->register_movement(*this, *other_object, m_movement);
          other_object->propagate_movement(m_movement);
        }
      }
    } else {
      set_offset(m_path_handle.get_pos(get_size() * 32, Vector(0, 0)));
    }
  }

  m_objects_hit_bottom.clear();
}

void
TileMap::editor_update()
{
  if (get_walker()) {
    if (get_path() && get_path()->is_valid()) {
      m_movement = get_walker()->get_pos(get_size() * 32, m_path_handle) - get_offset();
      set_offset(get_walker()->get_pos(get_size() * 32, m_path_handle));

      if (!get_path()) return;
      if (!get_path()->is_valid()) return;

      if (m_starting_node >= static_cast<int>(get_path()->get_nodes().size()))
        m_starting_node = static_cast<int>(get_path()->get_nodes().size()) - 1;

      m_movement += get_path()->get_nodes()[m_starting_node].position - get_offset();
      set_offset(m_path_handle.get_pos(get_size() * 32, get_path()->get_nodes()[m_starting_node].position));
    } else {
      set_offset(m_path_handle.get_pos(get_size() * 32, Vector(0, 0)));
    }
  }
}

void
TileMap::on_flip(float height)
{
  for (int x = 0; x < get_width(); ++x) {
    for (int y = 0; y < get_height()/2; ++y) {
      // swap tiles
      int y2 = get_height() - 1 - y;
      uint32_t t1 = get_tile_id(x, y);
      uint32_t t2 = get_tile_id(x, y2);
      change(x, y, t2);
      change(x, y2, t1);
    }
  }
  FlipLevelTransformer::transform_flip(m_flip);
  Vector offset = get_offset();
  offset.y = height - offset.y - get_bbox().get_height();
  set_offset(offset);
  PathObject::on_flip();
}

void
TileMap::draw(DrawingContext& context)
{
  // skip draw if current opacity is 0.0
  if (m_current_alpha == 0.0f) return;

  context.push_transform();

  if (m_flip != NO_FLIP) context.set_flip(m_flip);

  if (m_editor_active) {
    if (m_current_alpha != 1.0f) {
      context.set_alpha(m_current_alpha);
    }
  } else {
    context.set_alpha(m_current_alpha/2);
  }

  const float trans_x = context.get_translation().x;
  const float trans_y = context.get_translation().y;
  const bool normal_speed = m_editor_active && Editor::is_active();
  context.set_translation(Vector(trans_x * (normal_speed ? 1.0f : m_speed_x),
                                 trans_y * (normal_speed ? 1.0f : m_speed_y)));

  Rectf draw_rect = context.get_cliprect();
  Rect t_draw_rect = get_tiles_overlapping(draw_rect);
  Vector start = get_tile_position(t_draw_rect.left, t_draw_rect.top);

  Vector pos(0.0f, 0.0f);
  int tx, ty;

  std::unordered_map<SurfacePtr,
                     std::tuple<std::vector<Rectf>,
                                std::vector<Rectf>>> batches;

  for (pos.x = start.x, tx = t_draw_rect.left; tx < t_draw_rect.right; pos.x += 32, ++tx) {
    for (pos.y = start.y, ty = t_draw_rect.top; ty < t_draw_rect.bottom; pos.y += 32, ++ty) {
      int index = ty*m_width + tx;
      assert (index >= 0);
      assert (index < (m_width * m_height));

      if (m_tiles[index] == 0) continue;
      const Tile& tile = m_tileset->get(m_tiles[index]);

      if (g_debug.show_collision_rects) {
        tile.draw_debug(context.color(), pos, LAYER_FOREGROUND1);
      }

      // If the tilemap is active in editor and showing deprecated tiles is enabled, draw indication over each deprecated tile
      if (Editor::is_active() && m_editor_active &&
          g_config->editor_show_deprecated_tiles && tile.is_deprecated())
      {
        context.color().draw_text(Resources::normal_font, "!", pos + Vector(16, 8),
                                  ALIGN_CENTER, LAYER_GUI - 10, Color::RED);
      }

      const SurfacePtr& surface = Editor::is_active() ? tile.get_current_editor_surface() : tile.get_current_surface();
      if (surface) {
        std::get<0>(batches[surface]).emplace_back(surface->get_region());
        std::get<1>(batches[surface]).emplace_back(pos,
                                                   Sizef(static_cast<float>(surface->get_width()),
                                                         static_cast<float>(surface->get_height())));
      }
    }
  }

  Canvas& canvas = context.get_canvas(m_draw_target);

  for (auto& it : batches)
  {
    const SurfacePtr& surface = it.first;
    if (surface) {
      canvas.draw_surface_batch(surface,
                                std::move(std::get<0>(it.second)),
                                std::move(std::get<1>(it.second)),
                                m_current_tint, m_z_pos);
    }
  }

  context.pop_transform();
}

void
TileMap::set(int newwidth, int newheight, const std::vector<unsigned int>&newt,
             int new_z_pos, bool newsolid)
{
  if (int(newt.size()) != newwidth * newheight)
    throw std::runtime_error("Wrong tilecount count.");

  m_width  = newwidth;
  m_height = newheight;

  m_tiles.resize(newt.size());
  m_tiles = newt;

  if (new_z_pos > (LAYER_GUI - 100))
    m_z_pos = LAYER_GUI - 100;
  else
    m_z_pos  = new_z_pos;
  m_real_solid  = newsolid;
  update_effective_solid ();

  // make sure all tiles are loaded
  for (const auto& tile : m_tiles)
    m_tileset->get(tile);
}

void
TileMap::resize(int new_width, int new_height, int fill_id,
                int xoffset, int yoffset)
{
  bool offset_finished_x = false;
  bool offset_finished_y = false;
  if (xoffset < 0 && new_width - m_width < 0)
  {
    apply_offset_x(fill_id, xoffset);
    offset_finished_x = true;
  }
  if (yoffset < 0 && new_height - m_height < 0)
  {
    apply_offset_y(fill_id, yoffset);
    offset_finished_y = true;
  }
  if (new_width < m_width) {
    // remap tiles for new width
    for (int y = 0; y < m_height && y < new_height; ++y) {
      for (int x = 0; x < new_width; ++x) {
        m_tiles[y * new_width + x] = m_tiles[y * m_width + x];
      }
    }
  }

  m_tiles.resize(new_width * new_height, fill_id);

  if (new_width > m_width) {
    // remap tiles
    for (int y = std::min(m_height, new_height)-1; y >= 0; --y) {
      for (int x = new_width-1; x >= 0; --x) {
        if (x >= m_width) {
          m_tiles[y * new_width + x] = fill_id;
          continue;
        }

        m_tiles[y * new_width + x] = m_tiles[y * m_width + x];
      }
    }
  }
  m_height = new_height;
  m_width = new_width;
  if (!offset_finished_x)
    apply_offset_x(fill_id, xoffset);
  if (!offset_finished_y)
    apply_offset_y(fill_id, yoffset);
}

void TileMap::resize(const Size& newsize, const Size& resize_offset) {
  resize(newsize.width, newsize.height, 0, resize_offset.width, resize_offset.height);
}

Rect
TileMap::get_tiles_overlapping(const Rectf &rect) const
{
  Rectf rect2 = rect;
  rect2.move(-m_offset);

  int t_left   = std::max(0     , int(floorf(rect2.get_left  () / 32)));
  int t_right  = std::min(m_width , int(ceilf (rect2.get_right () / 32)));
  int t_top    = std::max(0     , int(floorf(rect2.get_top   () / 32)));
  int t_bottom = std::min(m_height, int(ceilf (rect2.get_bottom() / 32)));
  return Rect(t_left, t_top, t_right, t_bottom);
}

void
TileMap::hits_object_bottom(CollisionObject& object)
{
  m_objects_hit_bottom.insert(&object);
}

void
TileMap::notify_object_removal(CollisionObject* other)
{
  m_objects_hit_bottom.erase(other);
}

void
TileMap::set_solid(bool solid)
{
  m_real_solid = solid;
  update_effective_solid ();
}

uint32_t
TileMap::get_tile_id(int x, int y) const
{
  if (x < 0) x = 0;
  if (x >= m_width) x = m_width - 1;
  if (y < 0) y = 0;
  if (y >= m_height) y = m_height - 1;

  if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
    //log_warning << "tile outside tilemap requested" << std::endl;
    return 0;
  }

  return m_tiles[y*m_width + x];
}

bool
TileMap::is_outside_bounds(const Vector& pos) const
{
  auto pos_ = (pos - m_offset) / 32.0f;
  float width = static_cast<float>(m_width);
  float height = static_cast<float>(m_height);
  return pos_.x < 0 || pos_.x >= width || pos_.y < 0 || pos_.y >= height;
}

const Tile&
TileMap::get_tile(int x, int y) const
{
  uint32_t id = get_tile_id(x, y);
  return m_tileset->get(id);
}

uint32_t
TileMap::get_tile_id_at(const Vector& pos) const
{
  Vector xy = (pos - m_offset) / 32.0f;
  return get_tile_id(static_cast<int>(xy.x), static_cast<int>(xy.y));
}

uint32_t
TileMap::get_tile_id_at(float x, float y) const
{
  return get_tile_id_at(Vector(x, y));
}

const Tile&
TileMap::get_tile_at(const Vector& pos) const
{
  uint32_t id = get_tile_id_at(pos);
  return m_tileset->get(id);
}

void
TileMap::change(int x, int y, uint32_t newtile)
{
  if(x < 0 || x >= m_width || y < 0 || y >= m_height)
    return;

  m_tiles[y*m_width + x] = newtile;
}

void
TileMap::change_at(const Vector& pos, uint32_t newtile)
{
  Vector xy = (pos - m_offset) / 32.0f;
  change(int(xy.x), int(xy.y), newtile);
}

void
TileMap::change_at(float x, float y, uint32_t newtile)
{
  change_at(Vector(x, y), newtile);
}

void
TileMap::change_all(uint32_t oldtile, uint32_t newtile)
{
  for (int x = 0; x < get_width(); x++) {
    for (int y = 0; y < get_height(); y++) {
      if (get_tile_id(x,y) != oldtile)
        continue;

      change(x,y,newtile);
    }
  }
}

void
TileMap::autotile(int x, int y, uint32_t tile)
{
  if (x < 0 || x >= m_width || y < 0 || y >= m_height)
    return;

  uint32_t current_tile = m_tiles[y*m_width + x];
  AutotileSet* curr_set;
  if (current_tile == 0)
  {
    // Special case 1 : If the tile is empty, check if we can use a non-solid
    // tile from the currently selected tile's autotile set (if any).
    curr_set = m_tileset->get_autotileset_from_tile(tile);
  }
  else if (m_tileset->get_autotileset_from_tile(tile) != nullptr &&
      m_tileset->get_autotileset_from_tile(tile)->is_member(current_tile))
  {
    // Special case 2 : If the tile is in multiple autotilesets, check if it
    // is in the same tileset as the selected tile. (Example : tile 47)
    curr_set = m_tileset->get_autotileset_from_tile(tile);
  }
  else
  {
    curr_set = m_tileset->get_autotileset_from_tile(current_tile);
  }

  // If tile is not autotileable, abort
  // If tile is from a corner autotileset, abort as well
  if (curr_set == nullptr)
  {
    return;
  }

  uint32_t realtile = curr_set->get_autotile(current_tile,
    curr_set->is_solid(get_tile_id(x-1, y-1)),
    curr_set->is_solid(get_tile_id(x  , y-1)),
    curr_set->is_solid(get_tile_id(x+1, y-1)),
    curr_set->is_solid(get_tile_id(x-1, y  )),
    curr_set->is_solid(get_tile_id(x  , y  )),
    curr_set->is_solid(get_tile_id(x+1, y  )),
    curr_set->is_solid(get_tile_id(x-1, y+1)),
    curr_set->is_solid(get_tile_id(x  , y+1)),
    curr_set->is_solid(get_tile_id(x+1, y+1)),
    x, y);

  m_tiles[y*m_width + x] = realtile;
}

void
TileMap::autotile_corner(int x, int y, uint32_t tile, AutotileCornerOperation op)
{
  if (x < 0 || x >= m_width || y < 0 || y >= m_height)
    return;

  if (!m_tileset->get_autotileset_from_tile(tile)->is_corner())
    return;

  AutotileSet* curr_set = m_tileset->get_autotileset_from_tile(tile);

  // If tile is not autotileable, abort
  if (curr_set == nullptr)
  {
    return;
  }

  // If tile is not empty or already of the appropriate tileset, abort
  uint32_t current_tile = m_tiles[y*m_width + x];
  if (current_tile != 0 && (m_tileset->get_autotileset_from_tile(tile) != nullptr
      && !m_tileset->get_autotileset_from_tile(tile)->is_member(current_tile)))
  {
    return;
  }

  // If the current tile is 0, it will automatically return 0
  uint8_t mask = curr_set->get_mask_from_tile(current_tile);
  if (op == AutotileCornerOperation::REMOVE_TOP_LEFT) mask = static_cast<uint8_t>(mask & 0x07);
  if (op == AutotileCornerOperation::REMOVE_TOP_RIGHT) mask = static_cast<uint8_t>(mask & 0x0B);
  if (op == AutotileCornerOperation::REMOVE_BOTTOM_LEFT) mask = static_cast<uint8_t>(mask & 0x0D);
  if (op == AutotileCornerOperation::REMOVE_BOTTOM_RIGHT) mask = static_cast<uint8_t>(mask & 0x0E);
  if (op == AutotileCornerOperation::ADD_TOP_LEFT) mask = static_cast<uint8_t>(mask | 0x08);
  if (op == AutotileCornerOperation::ADD_TOP_RIGHT) mask = static_cast<uint8_t>(mask | 0x04);
  if (op == AutotileCornerOperation::ADD_BOTTOM_LEFT) mask = static_cast<uint8_t>(mask | 0x02);
  if (op == AutotileCornerOperation::ADD_BOTTOM_RIGHT) mask = static_cast<uint8_t>(mask | 0x01);

  uint32_t realtile = (!mask) ? 0 : curr_set->get_autotile(current_tile,
    (mask & 0x08) != 0,
    false,
    (mask & 0x04) != 0,
    false,
    false,
    false,
    (mask & 0x02) != 0,
    false,
    (mask & 0x01) != 0,
    x, y);

  m_tiles[y*m_width + x] = realtile;
}

bool
TileMap::is_corner(uint32_t tile) const
{
  auto* ats = m_tileset->get_autotileset_from_tile(tile);
  return ats && ats->is_corner();
}

void
TileMap::autotile_erase(const Vector& pos, const Vector& corner_pos)
{
  if (pos.x < 0.f || pos.x >= static_cast<float>(m_width) ||
      pos.y < 0.f || pos.y >= static_cast<float>(m_height))
    return;

  if (corner_pos.x < 0.f || corner_pos.x >= static_cast<float>(m_width) ||
      corner_pos.y < 0.f || corner_pos.y >= static_cast<float>(m_height))
    return;

  uint32_t current_tile = m_tiles[static_cast<int>(pos.y)*m_width
                                  + static_cast<int>(pos.x)];

  AutotileSet* curr_set = m_tileset->get_autotileset_from_tile(current_tile);

  if (curr_set && curr_set->is_corner()) {
    int x = static_cast<int>(corner_pos.x), y = static_cast<int>(corner_pos.y);
    autotile_corner(x, y, current_tile, AutotileCornerOperation::REMOVE_TOP_LEFT);
    autotile_corner(x-1, y, current_tile, AutotileCornerOperation::REMOVE_TOP_RIGHT);
    autotile_corner(x, y-1, current_tile, AutotileCornerOperation::REMOVE_BOTTOM_LEFT);
    autotile_corner(x-1, y-1, current_tile, AutotileCornerOperation::REMOVE_BOTTOM_RIGHT);
  }
  else
  {
    int x = static_cast<int>(pos.x), y = static_cast<int>(pos.y);
    m_tiles[y*m_width + x] = 0;

    if (x - 1 >= 0 && y - 1 >= 0 && !is_corner(m_tiles[(y-1)*m_width + x-1])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[(y-1)*m_width + x-1]);
      autotile(x-1, y-1, m_tiles[(y-1)*m_width + x-1]);
    }

    if (y - 1 >= 0 && !is_corner(m_tiles[(y-1)*m_width + x])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[(y-1)*m_width + x]);
      autotile(x, y-1, m_tiles[(y-1)*m_width + x]);
    }

    if (y - 1 >= 0 && x + 1 < m_width && !is_corner(m_tiles[(y-1)*m_width + x+1])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[(y-1)*m_width + x+1]);
      autotile(x+1, y-1, m_tiles[(y-1)*m_width + x+1]);
    }

    if (x - 1 >= 0 && !is_corner(m_tiles[y*m_width + x-1])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[y*m_width + x-1]);
      autotile(x-1, y, m_tiles[y*m_width + x-1]);
    }

    if (x + 1 < m_width && !is_corner(m_tiles[y*m_width + x+1])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[y*m_width + x+1]);
      autotile(x+1, y, m_tiles[y*m_width + x+1]);
    }

    if (x - 1 >= 0 && y + 1 < m_height && !is_corner(m_tiles[(y+1)*m_width + x-1])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[(y+1)*m_width + x-1]);
      autotile(x-1, y+1, m_tiles[(y+1)*m_width + x-1]);
    }

    if (y + 1 < m_height && !is_corner(m_tiles[(y+1)*m_width + x])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[(y+1)*m_width + x]);
      autotile(x, y+1, m_tiles[(y+1)*m_width + x]);
    }

    if (y + 1 < m_height && x + 1 < m_width && !is_corner(m_tiles[(y+1)*m_width + x+1])) {
      if (m_tiles[y*m_width + x] == 0)
        autotile(x, y, m_tiles[(y+1)*m_width + x+1]);
      autotile(x+1, y+1, m_tiles[(y+1)*m_width + x+1]);
    }
  }
}

AutotileSet*
TileMap::get_autotileset(uint32_t tile) const
{
  return m_tileset->get_autotileset_from_tile(tile);
}

void
TileMap::fade(float alpha_, float time)
{
  m_alpha = alpha_;
  m_remaining_fade_time = time;
}

void
TileMap::tint_fade(const Color& new_tint, float time)
{
  m_tint = new_tint;
  m_remaining_tint_fade_time = time;
}

void
TileMap::tint_fade(float time, float red, float green, float blue, float alpha)
{
  tint_fade(Color(red, green, blue, alpha), time);
}

void
TileMap::set_alpha(float alpha)
{
  m_alpha = alpha;
  m_current_alpha = m_alpha;
  m_remaining_fade_time = 0;
  update_effective_solid ();
}

float
TileMap::get_alpha() const
{
  return m_current_alpha;
}

void
TileMap::move_by(const Vector& shift)
{
  if (!get_path()) {
    init_path_pos(m_offset);
    m_add_path = true;
  }
  get_path()->move_by(shift);
  m_offset += shift;
}

void
TileMap::update_effective_solid(bool update_manager)
{
  if (!m_real_solid)
    m_effective_solid = false;
  else if (m_effective_solid && (m_current_alpha < 0.25f))
    m_effective_solid = false;
  else if (!m_effective_solid && (m_current_alpha >= 0.75f))
    m_effective_solid = true;

  if (update_manager)
    get_parent()->update_solid(this);
}

void
TileMap::set_tileset(const TileSet* new_tileset)
{
  m_tileset = new_tileset;
}


void
TileMap::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addAbstractClass<TileMap>("TileMap", vm.findClass("GameObject"));

  PathObject::register_members(cls);

  cls.addFunc("get_tile_id", &TileMap::get_tile_id);
  cls.addFunc<uint32_t, TileMap, float, float>("get_tile_id_at", &TileMap::get_tile_id_at);
  cls.addFunc("change", &TileMap::change);
  cls.addFunc<void, TileMap, float, float, uint32_t>("change_at", &TileMap::change_at);
  cls.addFunc("change_all", &TileMap::change_all);
  cls.addFunc("fade", &TileMap::fade);
  cls.addFunc<void, TileMap, float, float, float, float, float>("tint_fade", &TileMap::tint_fade);
  cls.addFunc("set_alpha", &TileMap::set_alpha);
  cls.addFunc("get_alpha", &TileMap::get_alpha);
  cls.addFunc("set_solid", &TileMap::set_solid);
}

/* EOF */
