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
#include <cmath>

#include "editor/editor.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_set.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

TileMap::TileMap(const TileSet *new_tileset) :
  ExposedObject<TileMap, scripting::TileMap>(this),
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
  m_movement(0,0),
  m_flip(NO_FLIP),
  m_alpha(1.0),
  m_current_alpha(1.0),
  m_remaining_fade_time(0),
  m_tint(1, 1, 1),
  m_current_tint(1, 1, 1),
  m_remaining_tint_fade_time(0),
  m_running(false),
  m_draw_target(DrawingTarget::COLORMAP),
  m_new_size_x(0),
  m_new_size_y(0),
  m_new_offset_x(0),
  m_new_offset_y(0),
  m_add_path(false)
{
}

TileMap::TileMap(const TileSet *tileset_, const ReaderMapping& reader) :
  GameObject(reader),
  ExposedObject<TileMap, scripting::TileMap>(this),
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
  m_offset(Vector(0,0)),
  m_movement(Vector(0,0)),
  m_flip(NO_FLIP),
  m_alpha(1.0),
  m_current_alpha(1.0),
  m_remaining_fade_time(0),
  m_tint(1, 1, 1),
  m_current_tint(1, 1, 1),
  m_remaining_tint_fade_time(0),
  m_running(false),
  m_draw_target(DrawingTarget::COLORMAP),
  m_new_size_x(0),
  m_new_size_y(0),
  m_new_offset_x(0),
  m_new_offset_y(0),
  m_add_path(false)
{
  assert(m_tileset);

  reader.get("solid",  m_real_solid);
  reader.get("speed",  m_speed_x);
  reader.get("speed-y", m_speed_y, m_speed_x);

  m_z_pos = reader_get_layer (reader, /* default = */ 0);

  if (m_real_solid && ((m_speed_x != 1) || (m_speed_y != 1))) {
    log_warning << "Speed of solid tilemap is not 1. fixing" << std::endl;
    m_speed_x = 1;
    m_speed_y = 1;
  }

  boost::optional<ReaderMapping> path_mapping;
  if (reader.get("path", path_mapping)) {
    reader.get("running", m_running, false);
    path.reset(new Path());
    path->read(*path_mapping);
    walker.reset(new PathWalker(path.get(), m_running));
    Vector v = path->get_base();
    set_offset(v);
  }

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
  update_effective_solid ();

  reader.get("width", m_width);
  reader.get("height", m_height);
  if(m_width < 0 || m_height < 0) {
    //throw std::runtime_error("Invalid/No width/height specified in tilemap.");
    m_width = 0;
    m_height = 0;
    m_tiles.clear();
    resize(static_cast<int>(Sector::current()->get_width() / 32.0f),
           static_cast<int>(Sector::current()->get_height() / 32.0f));
    m_editor_active = false;
  } else {
    if(!reader.get("tiles", m_tiles))
      throw std::runtime_error("No tiles in tilemap.");

    if(int(m_tiles.size()) != m_width * m_height) {
      throw std::runtime_error("wrong number of tiles in tilemap.");
    }
  }

  bool empty = true;

  // make sure all tiles used on the tilemap are loaded and tilemap isn't empty
  for(const auto& tile : m_tiles) {
    if(tile != 0) {
      empty = false;
    }

    m_tileset->get(tile);
  }

  if(empty)
  {
    log_info << "Tilemap '" << m_name << "', z-pos '" << m_z_pos << "' is empty." << std::endl;
  }
}

TileMap::~TileMap()
{
}

void TileMap::float_channel(float target, float &current, float remaining_time, float elapsed_time)
{
  float amt = (target - current) / (remaining_time / elapsed_time);
  if (amt > 0) current = std::min(current + amt, target);
  if (amt < 0) current = std::max(current + amt, target);
}

void
TileMap::save(Writer& writer) {
  GameObject::save(writer);
  if (m_draw_target == DrawingTarget::LIGHTMAP) {
    writer.write("draw-target", "lightmap", false);
  } else {
    writer.write("draw-target", "normal", false);
  }
  writer.write("width", m_width);
  writer.write("height", m_height);
  writer.write("speed", m_speed_x);
  if(m_speed_y != m_speed_x) {
    writer.write("speed-y", m_speed_y);
  }
  writer.write("solid", m_real_solid);
  writer.write("z-pos", m_z_pos);
  if(m_alpha != 1) {
    writer.write("alpha", m_alpha);
  }
  writer.write("tint", m_tint.toVector());
  if(path) {
    path->save(writer);
  }
  writer.write("tiles", m_tiles);
}

ObjectSettings
TileMap::get_settings() {
  m_new_size_x = m_width;
  m_new_size_y = m_height;
  m_new_offset_x = 0;
  m_new_offset_y = 0;
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_TOGGLE, _("solid"), &m_real_solid));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("resize offset x"), &m_new_offset_x));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("resize offset y"), &m_new_offset_y));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("width"), &m_new_size_x));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("height"), &m_new_size_y));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("alpha"), &m_alpha));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed x"), &m_speed_x));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed y"), &m_speed_y));
  result.options.push_back( ObjectOption(MN_COLOR, _("tint"), &m_tint));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &m_z_pos));

  ObjectOption draw_target_option(MN_STRINGSELECT, _("Draw target"), &m_draw_target);
  draw_target_option.select.push_back(_("normal"));
  draw_target_option.select.push_back(_("lightmap"));
  result.options.push_back(draw_target_option);

  m_add_path = walker.get() && path->is_valid();
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Following path"), &m_add_path));

  if (walker.get() && path->is_valid()) {
    result.options.push_back( Path::get_mode_option(&path->mode) );
    result.options.push_back(ObjectOption(MN_TOGGLE, _("Running"), &m_running, "running"));
  }

  if (!m_editor_active) {
    result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
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

  if (walker.get() && path->is_valid()) {
    if (!m_add_path) {
      path->nodes.clear();
    }
  } else {
    if (m_add_path) {
      path.reset(new Path(m_offset));
      walker.reset(new PathWalker(path.get(), m_running));
    }
  }
}

void
TileMap::update(float elapsed_time)
{
  // handle tilemap fading
  if (m_current_alpha != m_alpha) {
    m_remaining_fade_time = std::max(0.0f, m_remaining_fade_time - elapsed_time);
    if (m_remaining_fade_time == 0.0f) {
      m_current_alpha = m_alpha;
    } else {
      float_channel(m_alpha, m_current_alpha, m_remaining_fade_time, elapsed_time);
    }
    update_effective_solid ();
  }

  // handle tint fading
  if (m_current_tint.red != m_tint.red || m_current_tint.green != m_tint.green ||
      m_current_tint.blue != m_tint.blue || m_current_tint.alpha != m_tint.alpha) {

    m_remaining_tint_fade_time = std::max(0.0f, m_remaining_tint_fade_time - elapsed_time);
    if (m_remaining_tint_fade_time == 0.0f) {
      m_current_tint = m_tint;
    } else {
      float_channel(m_tint.red  , m_current_tint.red  , m_remaining_tint_fade_time, elapsed_time);
      float_channel(m_tint.green, m_current_tint.green, m_remaining_tint_fade_time, elapsed_time);
      float_channel(m_tint.blue , m_current_tint.blue , m_remaining_tint_fade_time, elapsed_time);
      float_channel(m_tint.alpha, m_current_tint.alpha, m_remaining_tint_fade_time, elapsed_time);
    }
  }

  m_movement = Vector(0,0);
  // if we have a path to follow, follow it
  if (walker.get()) {
    Vector v = walker->advance(elapsed_time);
    if (path->is_valid()) {
      m_movement = v - get_offset();
      set_offset(v);
    } else {
      set_offset(Vector(0, 0));
    }
  }
}

void
TileMap::draw(DrawingContext& context)
{
  // skip draw if current opacity is 0.0
  if (m_current_alpha == 0.0) return;

  context.push_transform();

  if (m_flip != NO_FLIP) context.set_flip(m_flip);

  if (m_editor_active) {
    if(m_current_alpha != 1.0) {
      context.set_alpha(m_current_alpha);
    }
  } else {
    context.set_alpha(m_current_alpha/2);
  }

  /* Force the translation to be an integer so that the tiles appear sharper.
   * For consistency (i.e., to avoid 1-pixel gaps), this needs to be done even
   * for solid tilemaps that are guaranteed to have speed 1.
   * FIXME Force integer translation for all graphics, not just tilemaps. */
  float trans_x = roundf(context.get_translation().x);
  float trans_y = roundf(context.get_translation().y);
  bool normal_speed = m_editor_active && Editor::is_active();
  context.set_translation(Vector(std::truncf(trans_x * (normal_speed ? 1.0f : m_speed_x)),
                                 std::truncf(trans_y * (normal_speed ? 1.0f : m_speed_y))));

  Rectf draw_rect = context.get_cliprect();
  Rect t_draw_rect = get_tiles_overlapping(draw_rect);
  Vector start = get_tile_position(t_draw_rect.left, t_draw_rect.top);

  Vector pos;
  int tx, ty;

  std::unordered_map<SurfacePtr,
                     std::tuple<std::vector<Rectf>,
                                std::vector<Rectf>>> batches;

  for(pos.x = start.x, tx = t_draw_rect.left; tx < t_draw_rect.right; pos.x += 32, ++tx) {
    for(pos.y = start.y, ty = t_draw_rect.top; ty < t_draw_rect.bottom; pos.y += 32, ++ty) {
      int index = ty*m_width + tx;
      assert (index >= 0);
      assert (index < (m_width * m_height));

      if (m_tiles[index] == 0) continue;
      const Tile& tile = m_tileset->get(m_tiles[index]);

      const SurfacePtr& surface = tile.get_current_surface();

      if (surface)
      {
        std::get<0>(batches[surface]).push_back(Rectf(surface->get_region())),
        std::get<1>(batches[surface]).push_back(Rectf(pos,
                                                      Sizef(static_cast<float>(surface->get_width()),
                                                            static_cast<float>(surface->get_height()))));
      }
    }
  }

  Canvas& canvas = context.get_canvas(m_draw_target);

  for(const auto& it : batches)
  {
    const SurfacePtr& surface = it.first;
    if (!surface) continue;

    const std::vector<Rectf>& srcrects = std::get<0>(it.second);
    const std::vector<Rectf>& dstrects = std::get<1>(it.second);

    canvas.draw_surface_batch(surface, srcrects, dstrects, m_current_tint, m_z_pos);
  }

  context.pop_transform();
}

void
TileMap::goto_node(int node_no)
{
  if (!walker.get()) return;
  walker->goto_node(node_no);
}

void
TileMap::start_moving()
{
  if (!walker.get()) return;
  walker->start_moving();
}

void
TileMap::stop_moving()
{
  if (!walker.get()) return;
  walker->stop_moving();
}

void
TileMap::set(int newwidth, int newheight, const std::vector<unsigned int>&newt,
             int new_z_pos, bool newsolid)
{
  if(int(newt.size()) != newwidth * newheight)
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
  for(const auto& tile : m_tiles)
    m_tileset->get(tile);
}

void
TileMap::resize(int new_width, int new_height, int fill_id,
                int xoffset, int yoffset)
{
  if(new_width < m_width) {
    // remap tiles for new width
    for(int y = 0; y < m_height && y < new_height; ++y) {
      for(int x = 0; x < new_width; ++x) {
        m_tiles[y * new_width + x] = m_tiles[y * m_width + x];
      }
    }
  }

  m_tiles.resize(new_width * new_height, fill_id);

  if(new_width > m_width) {
    // remap tiles
    for(int y = std::min(m_height, new_height)-1; y >= 0; --y) {
      for(int x = new_width-1; x >= 0; --x) {
        if(x >= m_width) {
          m_tiles[y * new_width + x] = fill_id;
          continue;
        }

        m_tiles[y * new_width + x] = m_tiles[y * m_width + x];
      }
    }
  }

  m_height = new_height;
  m_width = new_width;

  //Apply offset
  if (xoffset || yoffset) {
    int X, Y;
    for(int y = 0; y < m_height; y++) {
      Y = (yoffset < 0) ? y : (m_height - y - 1);
      for(int x = 0; x < m_width; x++) {
        X = (xoffset < 0) ? x : (m_width - x - 1);
        if (Y - yoffset < 0 || Y - yoffset >= m_height ||
            X - xoffset < 0 || X - xoffset >= m_width) {
          m_tiles[Y * new_width + X] = fill_id;
        } else {
          m_tiles[Y * new_width + X] = m_tiles[(Y - yoffset) * m_width + X - xoffset];
        }
      }
    }
  }
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
TileMap::set_solid(bool solid)
{
  m_real_solid = solid;
  update_effective_solid ();
}

uint32_t
TileMap::get_tile_id(int x, int y) const
{
  if(x < 0 || x >= m_width || y < 0 || y >= m_height) {
    //log_warning << "tile outside tilemap requested" << std::endl;
    return 0;
  }

  return m_tiles[y*m_width + x];
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
  Vector xy = (pos - m_offset) / 32;
  return get_tile_id(int(xy.x), int(xy.y));
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
  assert(x >= 0 && x < m_width && y >= 0 && y < m_height);
  m_tiles[y*m_width + x] = newtile;
}

void
TileMap::change_at(const Vector& pos, uint32_t newtile)
{
  Vector xy = (pos - m_offset) / 32;
  change(int(xy.x), int(xy.y), newtile);
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
TileMap::fade(float alpha_, float seconds)
{
  m_alpha = alpha_;
  m_remaining_fade_time = seconds;
}

void
TileMap::tint_fade(const Color& new_tint, float seconds)
{
  m_tint = new_tint;
  m_remaining_tint_fade_time = seconds;
}

void
TileMap::set_alpha(float alpha_)
{
  m_alpha = alpha_;
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
  if (!path) {
    path.reset(new Path(m_offset));
    walker.reset(new PathWalker(path.get()));
    m_add_path = true;
  }
  path->move_by(shift);
  m_offset += shift;
}

/*
 * Private methods
 */
void
TileMap::update_effective_solid()
{
  if (!m_real_solid)
    m_effective_solid = false;
  else if (m_effective_solid && (m_current_alpha < 0.25f))
    m_effective_solid = false;
  else if (!m_effective_solid && (m_current_alpha >= 0.75f))
    m_effective_solid = true;
}

void
TileMap::set_tileset(const TileSet* new_tileset)
{
  m_tileset = new_tileset;
}

/* EOF */
