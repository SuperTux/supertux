//  SuperTux - Climbable area
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "trigger/climbable.hpp"

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "supertux/debug.hpp"
#include "supertux/resources.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

namespace {
const float GRACE_DX = 8; // how far off may the player's bounding-box be x-wise
const float GRACE_DY = 8; // how far off may the player's bounding-box be y-wise
const float ACTIVATE_TRY_FOR = 1; // how long to try correcting mis-alignment of player and climbable before giving up
const float POSITION_FIX_AX = 30; // x-wise acceleration applied to player when trying to align player and Climbable
const float POSITION_FIX_AY = 50; // y-wise acceleration applied to player when trying to align player and Climbable
}

Climbable::Climbable(const ReaderMapping& reader) :
  climbed_by(),
  activate_try_timer(),
  message(),
  new_size(0.0f, 0.0f)
{
  reader.get("x", m_col.m_bbox.get_left());
  reader.get("y", m_col.m_bbox.get_top());
  float w = 32, h = 32;
  reader.get("width", w);
  reader.get("height", h);
  m_col.m_bbox.set_size(w, h);
  new_size.x = w;
  new_size.y = h;
  reader.get("message", message);
}

Climbable::Climbable(const Rectf& area) :
  climbed_by(),
  activate_try_timer(),
  message(),
  new_size(0.0f, 0.0f)
{
  m_col.m_bbox = area;
}

Climbable::~Climbable()
{
  for (auto* player : climbed_by)
    player->stop_climbing(*this);

  climbed_by.clear();
}

ObjectSettings
Climbable::get_settings()
{
  new_size.x = m_col.m_bbox.get_width();
  new_size.y = m_col.m_bbox.get_height();

  ObjectSettings result = TriggerBase::get_settings();

  // result.add_float(_("Width"), &new_size.x, "width");
  // result.add_float(_("Height"), &new_size.y, "height");

  result.add_translatable_text(_("Message"), &message, "message");

  result.reorder({"message", "region", "x", "y"});

  return result;
}

void
Climbable::after_editor_set() {
  m_col.m_bbox.set_size(new_size.x, new_size.y);
}

void
Climbable::update(float /*dt_sec*/)
{
  for (auto* player : climbed_by)
    if (!may_climb(*player))
      player->stop_climbing(*this);
}

void
Climbable::draw(DrawingContext& context)
{
  if (!climbed_by.empty() && !message.empty()) {
    context.push_transform();
    context.set_translation(Vector(0, 0));
    Vector pos = Vector(0, static_cast<float>(SCREEN_HEIGHT) / 2.0f - Resources::normal_font->get_height() / 2.0f);
    context.color().draw_center_text(Resources::normal_font, _(message), pos, LAYER_HUD, Climbable::text_color);
    context.pop_transform();
  }
  if (Editor::is_active() || g_debug.show_collision_rects) {
    context.color().draw_filled_rect(m_col.m_bbox, Color(1.0f, 1.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

void
Climbable::event(Player& player, EventType type)
{
  if ((type == EVENT_ACTIVATE) || (activate_try_timer.started())) {
    if (player.get_grabbed_object() == nullptr){
      if (may_climb(player)) {
        climbed_by.push_back(&player);
        player.start_climbing(*this);
        activate_try_timer.stop();
      } else {
        if (type == EVENT_ACTIVATE) activate_try_timer.start(ACTIVATE_TRY_FOR);
        // the "-13" to y velocity prevents Tux from walking in place on the ground for horizonal adjustments
        if (player.get_bbox().get_left() < m_col.m_bbox.get_left() - GRACE_DX) player.add_velocity(Vector(POSITION_FIX_AX,-13));
        if (player.get_bbox().get_right() > m_col.m_bbox.get_right() + GRACE_DX) player.add_velocity(Vector(-POSITION_FIX_AX,-13));
        if (player.get_bbox().get_top() < m_col.m_bbox.get_top() - GRACE_DY) player.add_velocity(Vector(0,POSITION_FIX_AY));
        if (player.get_bbox().get_bottom() > m_col.m_bbox.get_bottom() + GRACE_DY) player.add_velocity(Vector(0,-POSITION_FIX_AY));
      }
    }
  }

  if (type == EVENT_LOSETOUCH) {
    player.stop_climbing(*this);
    auto it = climbed_by.begin();
    while (it != climbed_by.end())
    {
      if (*it == &player)
        it = climbed_by.erase(it);
      else
        it++;
    }
  }
}

bool
Climbable::may_climb(Player& player) const
{
  if (player.get_bbox().get_left() < m_col.m_bbox.get_left() - GRACE_DX) return false;
  if (player.get_bbox().get_right() > m_col.m_bbox.get_right() + GRACE_DX) return false;
  if (player.get_bbox().get_top() < m_col.m_bbox.get_top() - GRACE_DY) return false;
  if (player.get_bbox().get_bottom() > m_col.m_bbox.get_bottom() + GRACE_DY) return false;
  return true;
}

/* EOF */
