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

#include "object/camera.hpp"

#include <math.h>
#include <physfs.h>

#include "math/util.hpp"
#include "object/player.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

/* this is the fractional distance toward the peek
   position to move each frame; lower is slower,
   0 is never get there, 1 is instant */
static const float PEEK_ARRIVE_RATIO = 0.1f;

class CameraConfig final
{
public:
  // 0 = No, 1 = Fix, 2 = Mario/Yoshi, 3 = Kirby, 4 = Super Metroid-like
  int xmode;
  // as above
  int ymode;
  float kirby_rectsize_x;
  float kirby_rectsize_y;
  // where to fix the player (used for Yoshi and Fix camera)
  float target_x;
  float target_y;
  // maximum scrolling speed in Y direction
  float max_speed_x;
  float max_speed_y;
  // factor to dynamically increase max_speed_x based on player speed
  float dynamic_max_speed_x;

  // time the player has to face into the other direction before we assume a
  // changed direction
  float dirchange_time;
  // edge_x
  float edge_x;
  // when too change from noscroll mode back to lookahead left/right mode
  // set to <= 0 to disable noscroll mode
  float sensitive_x;

  float clamp_x;
  float clamp_y;

  float dynamic_speed_sm;

  CameraConfig() :
    xmode(4),
    ymode(3),
    kirby_rectsize_x(0.2f),
    kirby_rectsize_y(0.34f),
    target_x(.5f),
    target_y(.5f),
    max_speed_x(100),
    max_speed_y(100),
    dynamic_max_speed_x(1.0),
    dirchange_time(0.2f),
    edge_x(0.4f),
    sensitive_x(-1),
    clamp_x(0.1666f),
    clamp_y(0.3f),
    dynamic_speed_sm(0.8f)
  {
  }

  void load(const std::string& filename)
  {
    auto doc = ReaderDocument::from_file(filename);
    auto root = doc.get_root();
    if (root.get_name() == "camera-config")
    {
      throw std::runtime_error("file is not a camera config file.");
    }
    else
    {
      auto camconfig = root.get_mapping();

      camconfig.get("xmode", xmode);
      camconfig.get("ymode", ymode);
      camconfig.get("target-x", target_x);
      camconfig.get("target-y", target_y);
      camconfig.get("max-speed-x", max_speed_x);
      camconfig.get("max-speed-y", max_speed_y);
      camconfig.get("dynamic-max-speed-x", dynamic_max_speed_x);
      camconfig.get("dirchange-time", dirchange_time);
      camconfig.get("clamp-x", clamp_x);
      camconfig.get("clamp-y", clamp_y);
      camconfig.get("kirby-rectsize-x", kirby_rectsize_x);
      camconfig.get("kirby-rectsize-y", kirby_rectsize_y);
      camconfig.get("edge-x", edge_x);
      camconfig.get("sensitive-x", sensitive_x);
      camconfig.get("dynamic-speed-sm", dynamic_speed_sm);
    }
  }
};

Camera::Camera(const std::string& name) :
  GameObject(name),
  ExposedObject<Camera, scripting::Camera>(this),
  m_mode(Mode::NORMAL),
  m_defaultmode(Mode::NORMAL),
  m_screen_size(SCREEN_WIDTH, SCREEN_HEIGHT),
  m_translation(),
  m_lookahead_mode(LookaheadMode::NONE),
  m_changetime(),
  m_lookahead_pos(),
  m_peek_pos(),
  m_cached_translation(),
  m_shaketimer(),
  m_shakespeed(),
  m_shakedepth_x(),
  m_shakedepth_y(),
  m_scroll_from(),
  m_scroll_goal(),
  m_scroll_to_pos(),
  m_scrollspeed(),
  m_config(std::make_unique<CameraConfig>())
{
  reload_config();
}

Camera::Camera(const ReaderMapping& reader) :
  GameObject(reader),
  ExposedObject<Camera, scripting::Camera>(this),
  m_mode(Mode::NORMAL),
  m_defaultmode(Mode::NORMAL),
  m_screen_size(SCREEN_WIDTH, SCREEN_HEIGHT),
  m_translation(),
  m_lookahead_mode(LookaheadMode::NONE),
  m_changetime(),
  m_lookahead_pos(),
  m_peek_pos(),
  m_cached_translation(),
  m_shaketimer(),
  m_shakespeed(),
  m_shakedepth_x(),
  m_shakedepth_y(),
  m_scroll_from(),
  m_scroll_goal(),
  m_scroll_to_pos(),
  m_scrollspeed(),
  m_config(std::make_unique<CameraConfig>())
{
  std::string modename;

  reader.get("mode", modename);
  if (modename == "normal")
  {
    m_mode = Mode::NORMAL;
  }
  else if (modename == "autoscroll")
  {
    m_mode = Mode::AUTOSCROLL;

    init_path(reader, true);
  }
  else if (modename == "manual")
  {
    m_mode = Mode::MANUAL;
  }
  else
  {
    m_mode = Mode::NORMAL;
    log_warning << "invalid camera mode '" << modename << "'found in worldfile." << std::endl;
  }
  m_defaultmode = m_mode;

  if (m_name.empty()) {
    m_name = "Camera";
  }

  reload_config();
}

Camera::~Camera()
{
}

ObjectSettings
Camera::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  result.add_enum(_("Mode"), reinterpret_cast<int*>(&m_defaultmode),
                  {_("normal"), _("manual"), _("autoscroll")},
                  {"normal", "manual", "autoscroll"},
                  {}, "mode");

  result.add_path_ref(_("Path"), get_path_ref(), "path-ref");

  if (get_walker() && get_path()->is_valid()) {
    result.add_walk_mode(_("Path Mode"), &get_path()->m_mode, {}, {});
  }

  return result;
}

void
Camera::after_editor_set()
{
  if (get_walker() && get_path()->is_valid()) {
    if (m_defaultmode != Mode::AUTOSCROLL) {
      get_path()->m_nodes.clear();
    }
  } else {
    if (m_defaultmode == Mode::AUTOSCROLL) {
      init_path_pos(Vector(0,0));
    }
  }
}

const Vector&
Camera::get_translation() const
{
  return m_translation;
}

void
Camera::reset(const Vector& tuxpos)
{
  m_translation.x = tuxpos.x - static_cast<float>(m_screen_size.width) / 2.0f;
  m_translation.y = tuxpos.y - static_cast<float>(m_screen_size.height) / 2.0f;

  m_shakespeed = 0;
  m_shaketimer.stop();
  keep_in_bounds(m_translation);

  m_cached_translation = m_translation;
}

void
Camera::shake(float duration, float x, float y)
{
  m_shaketimer.start(duration);
  m_shakedepth_x = x;
  m_shakedepth_y = y;
  m_shakespeed = math::PI_2 / duration;
}

void
Camera::scroll_to(const Vector& goal, float scrolltime)
{
  if(scrolltime == 0.0f)
  {
    m_translation.x = goal.x;
    m_translation.y = goal.y;
    m_mode = Mode::MANUAL;
    return;
  }

  m_scroll_from = m_translation;
  m_scroll_goal = goal;
  keep_in_bounds(m_scroll_goal);

  m_scroll_to_pos = 0;
  m_scrollspeed = 1.f / scrolltime;
  m_mode = Mode::SCROLLTO;
}

static const float CAMERA_EPSILON = .00001f;

void
Camera::draw(DrawingContext& context)
{
  m_screen_size = Size(context.get_width(),
                       context.get_height());
}

void
Camera::update(float dt_sec)
{
  switch (m_mode) {
    case Mode::NORMAL:
      update_scroll_normal(dt_sec);
      break;
    case Mode::AUTOSCROLL:
      update_scroll_autoscroll(dt_sec);
      break;
    case Mode::SCROLLTO:
      update_scroll_to(dt_sec);
      break;
    default:
      break;
  }
  shake();
}

void
Camera::reload_config()
{
  if (PHYSFS_exists("camera.cfg")) {
    try {
      m_config->load("camera.cfg");
      log_info << "Loaded camera.cfg." << std::endl;
    } catch(std::exception &e) {
      log_debug << "Couldn't load camera.cfg, using defaults ("
                << e.what() << ")" << std::endl;
    }
  }
}

void
Camera::keep_in_bounds(Vector& translation_)
{
  float width = d_sector->get_width();
  float height = d_sector->get_height();

  // don't scroll before the start or after the level's end
  translation_.x = math::clamp(translation_.x, 0.0f, width - static_cast<float>(m_screen_size.width));
  translation_.y = math::clamp(translation_.y, 0.0f, height - static_cast<float>(m_screen_size.height));

  if (height < static_cast<float>(m_screen_size.height))
    translation_.y = height / 2.0f - static_cast<float>(m_screen_size.height) / 2.0f;
  if (width < static_cast<float>(m_screen_size.width))
    translation_.x = width / 2.0f - static_cast<float>(m_screen_size.width) / 2.0f;
}

void
Camera::shake()
{
  if (m_shaketimer.started()) {
    m_translation.x -= sinf(m_shaketimer.get_timegone() * m_shakespeed) * m_shakedepth_x;
    m_translation.y -= sinf(m_shaketimer.get_timegone() * m_shakespeed) * m_shakedepth_y;
  }
}

void
Camera::update_scroll_normal(float dt_sec)
{
  const auto& config_ = *(m_config);
  Player& player = d_sector->get_player();
  // TODO: co-op mode needs a good camera
  Vector player_pos(player.get_bbox().get_left(),
                                    player.get_bbox().get_bottom());
  static Vector last_player_pos = player_pos;
  Vector player_delta = player_pos - last_player_pos;
  last_player_pos = player_pos;

  // check that we don't have division by zero later
  if (dt_sec < CAMERA_EPSILON)
    return;

  /****** Vertical Scrolling part ******/
  int ymode = config_.ymode;

  if (player.is_dying() || d_sector->get_height() == 19*32) {
    ymode = 0;
  }
  if (ymode == 1) {
    m_cached_translation.y = player_pos.y - static_cast<float>(m_screen_size.height) * config_.target_y;
  }
  if (ymode == 2) {
    // target_y is the height we target our scrolling at. This is not always the
    // height of the player: while jumping upwards, we should use the
    // position where they last touched the ground. (this probably needs
    // exceptions for trampolines and similar things in the future)
    float target_y;
    if (player.m_fall_mode == Player::JUMPING)
      target_y = player.m_last_ground_y + player.get_bbox().get_height();
    else
      target_y = player.get_bbox().get_bottom();
    target_y -= static_cast<float>(static_cast<float>(m_screen_size.height)) * config_.target_y;

    // delta_y is the distance we'd have to travel to directly reach target_y
    float delta_y = m_cached_translation.y - target_y;
    // speed is the speed the camera would need to reach target_y in this frame
    float speed_y = delta_y / dt_sec;

    // limit the camera speed when jumping upwards
    if (player.m_fall_mode != Player::FALLING
       && player.m_fall_mode != Player::TRAMPOLINE_JUMP) {
      speed_y = math::clamp(speed_y, -config_.max_speed_y, config_.max_speed_y);
    }

    // scroll with calculated speed
    m_cached_translation.y -= speed_y * dt_sec;
  }
  if (ymode == 3) {
    float halfsize = config_.kirby_rectsize_y * 0.5f;
    m_cached_translation.y = math::clamp(m_cached_translation.y,
                                 player_pos.y - static_cast<float>(m_screen_size.height) * (0.5f + halfsize),
                                 player_pos.y - static_cast<float>(m_screen_size.height) * (0.5f - halfsize));
  }
  if (ymode == 4) {
    float upperend = static_cast<float>(m_screen_size.height) * config_.edge_x;
    float lowerend = static_cast<float>(m_screen_size.height) * (1 - config_.edge_x);

    if (player_delta.y < -CAMERA_EPSILON) {
      // walking left
      m_lookahead_pos.y -= player_delta.y * config_.dynamic_speed_sm;

      if (m_lookahead_pos.y > lowerend) {
        m_lookahead_pos.y = lowerend;
      }
    } else if (player_delta.y > CAMERA_EPSILON) {
      // walking right
      m_lookahead_pos.y -= player_delta.y * config_.dynamic_speed_sm;
      if (m_lookahead_pos.y < upperend) {
        m_lookahead_pos.y = upperend;
      }
    }

    // adjust for level ends
    if (player_pos.y < upperend) {
      m_lookahead_pos.y = upperend;
    }
    if (player_pos.y > d_sector->get_width() - upperend) {
      m_lookahead_pos.y = lowerend;
    }

    m_cached_translation.y = player_pos.y - m_lookahead_pos.y;
  }

  m_translation.y = m_cached_translation.y;

  if (ymode != 0) {
    float top_edge, bottom_edge;
    if (config_.clamp_y <= 0) {
      top_edge = 0;
      bottom_edge = static_cast<float>(m_screen_size.height);
    } else {
      top_edge = static_cast<float>(m_screen_size.height) * config_.clamp_y;
      bottom_edge = static_cast<float>(m_screen_size.height) * (1.0f - config_.clamp_y);
    }

    float peek_to = 0;
    float translation_compensation = player_pos.y - m_translation.y;

    if (player.peeking_direction_y() == Direction::UP) {
      peek_to = bottom_edge - translation_compensation;
    } else if (player.peeking_direction_y() == Direction::DOWN) {
      peek_to = top_edge - translation_compensation;
    }

    float peek_move = (peek_to - m_peek_pos.y) * PEEK_ARRIVE_RATIO;
    if (fabsf(peek_move) < 1.0f) {
      peek_move = 0.0;
    }

    m_peek_pos.y += peek_move;

    m_translation.y -= m_peek_pos.y;

    if (config_.clamp_y > 0.0f) {
      m_translation.y = math::clamp(m_translation.y,
                            player_pos.y - static_cast<float>(m_screen_size.height) * (1.0f - config_.clamp_y),
                            player_pos.y - static_cast<float>(m_screen_size.height) * config_.clamp_y);
      m_cached_translation.y = math::clamp(m_cached_translation.y,
                                   player_pos.y - static_cast<float>(m_screen_size.height) * (1.0f - config_.clamp_y),
                                   player_pos.y - static_cast<float>(m_screen_size.height) * config_.clamp_y);
    }
  }

  /****** Horizontal scrolling part *******/
  int xmode = config_.xmode;

  if (player.is_dying())
    xmode = 0;

  if (xmode == 1) {
    m_cached_translation.x = player_pos.x - static_cast<float>(m_screen_size.width) * config_.target_x;
  }
  if (xmode == 2) {
    // our camera is either in leftscrolling, rightscrolling or
    // nonscrollingmode.
    //
    // when suddenly changing directions while scrolling into the other
    // direction abort scrolling, since tux might be going left/right at a
    // relatively small part of the map (like when jumping upwards)

    // Find out direction in which the player moves
    LookaheadMode walkDirection;
    if (player_delta.x < -CAMERA_EPSILON) walkDirection = LookaheadMode::LEFT;
    else if (player_delta.x > CAMERA_EPSILON) walkDirection = LookaheadMode::RIGHT;
    else if (player.m_dir == Direction::LEFT) walkDirection = LookaheadMode::LEFT;
    else walkDirection = LookaheadMode::RIGHT;

    float LEFTEND, RIGHTEND;
    if (config_.sensitive_x > 0) {
      LEFTEND = static_cast<float>(m_screen_size.width) * config_.sensitive_x;
      RIGHTEND = static_cast<float>(m_screen_size.width) * (1-config_.sensitive_x);
    } else {
      LEFTEND = static_cast<float>(m_screen_size.width);
      RIGHTEND = 0.0f;
    }

    if (m_lookahead_mode == LookaheadMode::NONE) {
      /* if we're undecided then look if we crossed the left or right
       * "sensitive" area */
      if (player_pos.x < m_cached_translation.x + LEFTEND) {
        m_lookahead_mode = LookaheadMode::LEFT;
      } else if (player_pos.x > m_cached_translation.x + RIGHTEND) {
        m_lookahead_mode = LookaheadMode::RIGHT;
      }
      /* at the ends of a level it's obvious which way we will go */
      if (player_pos.x < static_cast<float>(m_screen_size.width) * 0.5f) {
        m_lookahead_mode = LookaheadMode::RIGHT;
      } else if (player_pos.x >= d_sector->get_width() - static_cast<float>(m_screen_size.width) * 0.5f) {
        m_lookahead_mode = LookaheadMode::LEFT;
      }

      m_changetime = -1;
    } else if (m_lookahead_mode != walkDirection) {
      /* Tux changed direction while camera was scrolling...
       * he has to do this for a certain time to add robustness against
       * sudden changes */
      if (m_changetime < 0) {
        m_changetime = g_game_time;
      } else if (g_game_time - m_changetime > config_.dirchange_time) {
        if (m_lookahead_mode == LookaheadMode::LEFT &&
           player_pos.x > m_cached_translation.x + RIGHTEND) {
          m_lookahead_mode = LookaheadMode::RIGHT;
        } else if (m_lookahead_mode == LookaheadMode::RIGHT &&
                  player_pos.x < m_cached_translation.x + LEFTEND) {
          m_lookahead_mode = LookaheadMode::LEFT;
        } else {
          m_lookahead_mode = LookaheadMode::NONE;
        }
      }
    } else {
      m_changetime = -1;
    }

    LEFTEND = static_cast<float>(m_screen_size.width) * config_.edge_x;
    RIGHTEND = static_cast<float>(m_screen_size.width) * (1-config_.edge_x);

    // calculate our scroll target depending on scroll mode
    float target_x;
    if (m_lookahead_mode == LookaheadMode::LEFT)
      target_x = player_pos.x - RIGHTEND;
    else if (m_lookahead_mode == LookaheadMode::RIGHT)
      target_x = player_pos.x - LEFTEND;
    else
      target_x = m_cached_translation.x;

    // that's the distance we would have to travel to reach target_x
    float delta_x = m_cached_translation.x - target_x;
    // the speed we'd need to travel to reach target_x in this frame
    float speed_x = delta_x / dt_sec;

    // limit our speed
    float player_speed_x = player_delta.x / dt_sec;
    float maxv = config_.max_speed_x + (fabsf(player_speed_x * config_.dynamic_max_speed_x));
    speed_x = math::clamp(speed_x, -maxv, maxv);

    // apply scrolling
    m_cached_translation.x -= speed_x * dt_sec;
  }
  if (xmode == 3) {
    float halfsize = config_.kirby_rectsize_x * 0.5f;
    m_cached_translation.x = math::clamp(m_cached_translation.x,
                                 player_pos.x - static_cast<float>(m_screen_size.width) * (0.5f + halfsize),
                                 player_pos.x - static_cast<float>(m_screen_size.width) * (0.5f - halfsize));
  }
  if (xmode == 4) {
    float LEFTEND = static_cast<float>(m_screen_size.width) * config_.edge_x;
    float RIGHTEND = static_cast<float>(m_screen_size.width) * (1 - config_.edge_x);

    if (player_delta.x < -CAMERA_EPSILON) {
      // walking left
      m_lookahead_pos.x -= player_delta.x * config_.dynamic_speed_sm;
      if (m_lookahead_pos.x > RIGHTEND) {
        m_lookahead_pos.x = RIGHTEND;
      }

    } else if (player_delta.x > CAMERA_EPSILON) {
      // walking right
      m_lookahead_pos.x -= player_delta.x * config_.dynamic_speed_sm;
      if (m_lookahead_pos.x < LEFTEND) {
        m_lookahead_pos.x = LEFTEND;
      }
    }

    // adjust for level ends
    if (player_pos.x < LEFTEND) {
      m_lookahead_pos.x = LEFTEND;
    }
    if (player_pos.x > d_sector->get_width() - LEFTEND) {
      m_lookahead_pos.x = RIGHTEND;
    }

    m_cached_translation.x = player_pos.x - m_lookahead_pos.x;
  }

  m_translation.x = m_cached_translation.x;

  if (xmode != 0) {
    float left_edge, right_edge;
    if (config_.clamp_x <= 0) {
      left_edge = 0;
      right_edge = static_cast<float>(m_screen_size.width);
    } else {
      left_edge = static_cast<float>(m_screen_size.width) * config_.clamp_x;
      right_edge = static_cast<float>(m_screen_size.width) * (1.0f - config_.clamp_x);
    }

    float peek_to = 0;
    float translation_compensation = player_pos.x - m_translation.x;

    if (player.peeking_direction_x() == ::Direction::LEFT) {
      peek_to = right_edge - translation_compensation;
    } else if (player.peeking_direction_x() == Direction::RIGHT) {
      peek_to = left_edge - translation_compensation;
    }

    float peek_move = (peek_to - m_peek_pos.x) * PEEK_ARRIVE_RATIO;
    if (fabsf(peek_move) < 1.0f) {
      peek_move = 0.0f;
    }

    m_peek_pos.x += peek_move;

    m_translation.x -= m_peek_pos.x;

    if (config_.clamp_x > 0.0f) {
      m_translation.x = math::clamp(m_translation.x,
                            player_pos.x - static_cast<float>(m_screen_size.width) * (1-config_.clamp_x),
                            player_pos.x - static_cast<float>(m_screen_size.width) * config_.clamp_x);

      m_cached_translation.x = math::clamp(m_cached_translation.x,
                                   player_pos.x - static_cast<float>(m_screen_size.width) * (1-config_.clamp_x),
                                   player_pos.x - static_cast<float>(m_screen_size.width) * config_.clamp_x);
    }
  }

  keep_in_bounds(m_translation);
  keep_in_bounds(m_cached_translation);
}

void
Camera::update_scroll_autoscroll(float dt_sec)
{
  Player& player = d_sector->get_player();
  if (player.is_dying())
    return;

  get_walker()->update(dt_sec);
  m_translation = get_walker()->get_pos();

  keep_in_bounds(m_translation);
}

void
Camera::update_scroll_to(float dt_sec)
{
  m_scroll_to_pos += dt_sec * m_scrollspeed;
  if (m_scroll_to_pos >= 1.0f) {
    m_mode = Mode::MANUAL;
    m_translation = m_scroll_goal;
    return;
  }

  m_translation = m_scroll_from + (m_scroll_goal - m_scroll_from) * m_scroll_to_pos;
}

Vector
Camera::get_center() const
{
  return m_translation + Vector(static_cast<float>(m_screen_size.width) / 2.0f,
                              static_cast<float>(m_screen_size.height) / 2.0f);
}

void
Camera::move(const int dx, const int dy)
{
  m_translation.x += static_cast<float>(dx);
  m_translation.y += static_cast<float>(dy);
}

bool
Camera::is_saveable() const
{
  return !(Level::current() &&
           Level::current()->is_worldmap());
}
/* EOF */
