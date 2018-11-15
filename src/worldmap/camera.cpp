//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "worldmap/camera.hpp"

#include "supertux/globals.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

namespace {

const float CAMERA_PAN_SPEED = 5.0;

} // namespace

namespace worldmap {

Camera::Camera() :
  m_camera_offset(),
  m_pan_pos(),
  m_panning(false)
{
}

void
Camera::update(float dt_sec)
{
  Vector requested_pos;

  // position "camera"
  if (!m_panning) {
    m_camera_offset = get_camera_pos_for_tux();
  } else {
    Vector delta__ = m_pan_pos - m_camera_offset;
    float mag = delta__.norm();
    if (mag > CAMERA_PAN_SPEED) {
      delta__ *= CAMERA_PAN_SPEED/mag;
    }
    m_camera_offset += delta__;
    if (m_camera_offset == m_pan_pos) {
      m_panning = false;
    }
  }

  requested_pos = m_camera_offset;
  clamp_camera_position(m_camera_offset);

  if (m_panning) {
    if (requested_pos.x != m_camera_offset.x) {
      m_pan_pos.x = m_camera_offset.x;
    }
    if (requested_pos.y != m_camera_offset.y) {
      m_pan_pos.y = m_camera_offset.y;
    }
  }
}

void
Camera::pan()
{
  m_panning = true;
  m_pan_pos = get_camera_pos_for_tux();
  clamp_camera_position(m_pan_pos);
}

Vector
Camera::get_camera_pos_for_tux() const
{
  auto& worldmap = *WorldMap::current();
  auto& tux = worldmap.get_singleton_by_type<Tux>();

  Vector camera_offset_;
  Vector tux_pos = tux.get_pos();
  camera_offset_.x = tux_pos.x - static_cast<float>(SCREEN_WIDTH) / 2.0f;
  camera_offset_.y = tux_pos.y - static_cast<float>(SCREEN_HEIGHT) / 2.0f;
  return camera_offset_;
}

void
Camera::clamp_camera_position(Vector& c) const
{
  auto& worldmap = *WorldMap::current();

  if (c.x < 0) {
    c.x = 0;
  }

  if (c.y < 0) {
    c.y = 0;
  }

  if (c.x > worldmap.get_width() - static_cast<float>(SCREEN_WIDTH)) {
    c.x = worldmap.get_width() - static_cast<float>(SCREEN_WIDTH);
  }

  if (c.y > worldmap.get_height() - static_cast<float>(SCREEN_HEIGHT)) {
    c.y = worldmap.get_height() - static_cast<float>(SCREEN_HEIGHT);
  }

  if (worldmap.get_width() < static_cast<float>(SCREEN_WIDTH)) {
    c.x = (worldmap.get_width() - static_cast<float>(SCREEN_WIDTH)) / 2.0f;
  }

  if (worldmap.get_height() < static_cast<float>(SCREEN_HEIGHT)) {
    c.y = (worldmap.get_height() - static_cast<float>(SCREEN_HEIGHT)) / 2.0f;
  }
}

} // namespace worldmap

/* EOF */
