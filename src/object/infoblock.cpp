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

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/info_box_line.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

InfoBlock::InfoBlock(const ReaderMapping& lisp) :
  Block(lisp, "images/objects/bonus_block/infoblock.sprite"),
  message(),
  shown_pct(0),
  dest_pct(0),
  lines(),
  lines_height(0)
{
  if(!lisp.get("message", message)) {
    log_warning << "No message in InfoBlock" << std::endl;
  }
  //stopped = false;
  //ringing = new AmbientSound(get_pos(), 0.5, 300, 1, "sounds/phone.wav");
  //Sector::current()->add_object(ringing);

  // Split text string lines into a vector
  lines = InfoBoxLine::split(message, 400);
  for(const auto& line : lines) lines_height += line->get_height();
}

InfoBlock::~InfoBlock()
{
}

ObjectSettings
InfoBlock::get_settings() {
  ObjectSettings result = Block::get_settings();
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Message"), &message,
                                         "message"));

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

  if (dest_pct != 1) {

    // first hide all other InfoBlocks' messages in same sector
    auto parent = Sector::current();
    if (!parent) return;
    for (const auto& object : parent->gameobjects) {
      auto block = dynamic_cast<InfoBlock*>(object.get());
      if (!block) continue;
      if (block != this) block->hide_message();
    }

    // show our message
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
    if (player->does_buttjump)
      InfoBlock::hit(*player);
  }
  return Block::collision(other, hit_);
}

Player*
InfoBlock::get_nearest_player() const
{
  return Sector::current()->get_nearest_player (bbox);
}

void
InfoBlock::update(float delta)
{
  Block::update(delta);

  if (delta == 0) return;

  // hide message if player is too far away
  if (dest_pct > 0) {
    auto player = get_nearest_player();
    if (player) {
      Vector p1 = bbox.get_middle();
      Vector p2 = player->get_bbox().get_middle();
      Vector dist = (p2 - p1);
      float d = dist.norm();
      if (d > 128) dest_pct = 0;
    }
  }

  // handle soft fade-in and fade-out
  if (shown_pct != dest_pct) {
    if (dest_pct > shown_pct) shown_pct = std::min(shown_pct + 2*delta, dest_pct);
    if (dest_pct < shown_pct) shown_pct = std::max(shown_pct - 2*delta, dest_pct);
  }
}

void
InfoBlock::draw(DrawingContext& context)
{
  Block::draw(context);

  if (shown_pct <= 0) return;

  context.push_transform();
  //context.set_translation(Vector(0, 0));
  context.set_alpha(shown_pct);

  //float x1 = SCREEN_WIDTH/2-200;
  //float y1 = SCREEN_HEIGHT/2-200;
  float border = 8;
  float width = 400; // this is the text width only
  float height = lines_height; // this is the text height only
  float x1 = (bbox.p1.x + bbox.p2.x)/2 - width/2;
  float x2 = (bbox.p1.x + bbox.p2.x)/2 + width/2;
  float y1 = original_y - height;

  if(x1 < 0) {
    x1 = 0;
    x2 = width;
  }

  if(x2 > Sector::current()->get_width()) {
    x2 = Sector::current()->get_width();
    x1 = x2 - width;
  }

  // lines_height includes one ITEMS_SPACE too much, so the bottom border is reduced by 4px
  context.draw_filled_rect(Vector(x1-border, y1-border), Vector(width+2*border, height+2*border-4), Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI-50);

  float y = y1;
  for(size_t i = 0; i < lines.size(); ++i) {
    if(y >= y1 + height) {
      //log_warning << "Too many lines of text in InfoBlock" << std::endl;
      //dest_pct = 0;
      //shown_pct = 0;
      break;
    }

    lines[i]->draw(context, Rectf(x1, y, x2, y), LAYER_GUI-50+1);
    y += lines[i]->get_height();
  }

  context.pop_transform();
}

void
InfoBlock::show_message()
{
  dest_pct = 1;
}

void
InfoBlock::hide_message()
{
  dest_pct = 0;
}

/* EOF */
