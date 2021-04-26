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

#include "object/infoblock.hpp"

#include <algorithm>

#include "editor/editor.hpp"
#include "object/player.hpp"
#include "supertux/info_box_line.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

InfoBlock::InfoBlock(const ReaderMapping& mapping) :
  Block(mapping, "images/objects/bonus_block/infoblock.sprite"),
  m_message(),
  m_shown_pct(0),
  m_dest_pct(0),
  m_lines(),
  m_lines_height(0)
{
  if (!mapping.get("message", m_message) && !(Editor::is_active())) {
    log_warning << "No message in InfoBlock" << std::endl;
  }
  //stopped = false;
  //ringing = new AmbientSound(get_pos(), 0.5, 300, 1, "sounds/phone.wav");
  //Sector::get().add_object(ringing);

  // Split text string lines into a vector
  m_lines = InfoBoxLine::split(m_message, 400);
  for (const auto& line : m_lines) m_lines_height += line->get_height();
}

InfoBlock::~InfoBlock()
{
}

ObjectSettings
InfoBlock::get_settings()
{
  ObjectSettings result = Block::get_settings();

  result.add_translatable_text(_("Message"), &m_message, "message");

  result.reorder({"message", "x", "y"});

  return result;
}

void
InfoBlock::hit(Player& player)
{
  start_bounce(&player);

  //if (!stopped) {
  //  ringing->remove_me();
  //  stopped = true;
  //}

  if (m_dest_pct != 1) {

    // first hide all other InfoBlocks' messages in same sector
    for (auto& block : Sector::get().get_objects_by_type<InfoBlock>())
    {
      if (&block != this)
      {
        block.hide_message();
      }
    }

    show_message();

  } else {
    hide_message();
  }
}

HitResponse
InfoBlock::collision(GameObject& other, const CollisionHit& hit_)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player)
  {
    if (player->m_does_buttjump)
      InfoBlock::hit(*player);
  }
  return Block::collision(other, hit_);
}

Player*
InfoBlock::get_nearest_player() const
{
  return Sector::get().get_nearest_player (m_col.m_bbox);
}

void
InfoBlock::update(float dt_sec)
{
  Block::update(dt_sec);

  if (dt_sec == 0) return;

  // hide message if player is too far away
  if (m_dest_pct > 0) {
    if (auto* player = get_nearest_player()) {
      Vector p1 = m_col.m_bbox.get_middle();
      Vector p2 = player->get_bbox().get_middle();
      Vector dist = (p2 - p1);
      float d = glm::length(dist);
      if (d > 128) m_dest_pct = 0;
    }
  }

  // handle soft fade-in and fade-out
  if (m_shown_pct != m_dest_pct) {
    if (m_dest_pct > m_shown_pct) m_shown_pct = std::min(m_shown_pct + 2 * dt_sec, m_dest_pct);
    if (m_dest_pct < m_shown_pct) m_shown_pct = std::max(m_shown_pct - 2 * dt_sec, m_dest_pct);
  }
}

void
InfoBlock::draw(DrawingContext& context)
{
  Block::draw(context);

  if (m_shown_pct <= 0) return;

  context.push_transform();
  //context.set_translation(Vector(0, 0));
  context.set_alpha(m_shown_pct);

  //float x1 = SCREEN_WIDTH/2-200;
  //float y1 = SCREEN_HEIGHT/2-200;
  float border = 8;
  float width = 400; // this is the text width only
  float height = m_lines_height; // this is the text height only
  float x1 = (m_col.m_bbox.get_left() + m_col.m_bbox.get_right())/2 - width/2;
  float x2 = (m_col.m_bbox.get_left() + m_col.m_bbox.get_right())/2 + width/2;
  float y1 = m_original_y - height;

  if (x1 < 0) {
    x1 = 0;
    x2 = width;
  }

  if (x2 > Sector::get().get_width()) {
    x2 = Sector::get().get_width();
    x1 = x2 - width;
  }

  // lines_height includes one ITEMS_SPACE too much, so the bottom border is reduced by 4px
  context.color().draw_filled_rect(Rectf(Vector(x1-border, y1-border),
                                         Sizef(width+2*border, height+2*border-4)),
                                   Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI-50);

  float y = y1;
  for (size_t i = 0; i < m_lines.size(); ++i) {
    if (y >= y1 + height) {
      //log_warning << "Too many lines of text in InfoBlock" << std::endl;
      //dest_pct = 0;
      //shown_pct = 0;
      break;
    }

    m_lines[i]->draw(context, Rectf(x1, y, x2, y), LAYER_GUI-50+1);
    y += m_lines[i]->get_height();
  }

  context.pop_transform();
}

void
InfoBlock::show_message()
{
  m_dest_pct = 1;
}

void
InfoBlock::hide_message()
{
  m_dest_pct = 0;
}

/* EOF */
