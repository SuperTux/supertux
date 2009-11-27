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

#include "object/player.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "supertux/info_box_line.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"

namespace {
const float SCROLL_DELAY = 0.5;
const float SCROLL_DISTANCE = 16;
const float WIDTH = 400;
const float HEIGHT = 200;
}

InfoBlock::InfoBlock(const Reader& lisp) :
  Block(sprite_manager->create("images/objects/bonus_block/infoblock.sprite")), 
  message(),
  shown_pct(0), 
  dest_pct(0),
  lines(),
  lines_height()
{
  Vector pos;
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  bbox.set_pos(pos);

  if(!lisp.get("message", message)) {
    log_warning << "No message in InfoBlock" << std::endl;
  }
  //stopped = false;
  //ringing = new AmbientSound(get_pos(), 0.5, 300, 1, "sounds/phone.wav");
  //Sector::current()->add_object(ringing);

  // Split text string lines into a vector
  lines = InfoBoxLine::split(message, 400);
  lines_height = 0;
  for(size_t i = 0; i < lines.size(); ++i) lines_height+=lines[i]->get_height();
}

InfoBlock::~InfoBlock()
{
  for(std::vector<InfoBoxLine*>::iterator i = lines.begin(); i != lines.end(); i++) {
    delete *i;
  }
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
    Sector* parent = Sector::current();
    if (!parent) return;
    for (Sector::GameObjects::iterator i = parent->gameobjects.begin(); i != parent->gameobjects.end(); i++) {
      InfoBlock* block = dynamic_cast<InfoBlock*>(*i);
      if (!block) continue;
      if (block != this) block->hide_message();
    }

    // show our message
    show_message();

  } else {
    hide_message();
  }
}

Player*
InfoBlock::get_nearest_player()
{
  // FIXME: does not really return nearest player

  std::vector<Player*> players = Sector::current()->get_players();
  for (std::vector<Player*>::iterator playerIter = players.begin(); playerIter != players.end(); ++playerIter) {
    Player* player = *playerIter;
    return player;
  }

  return 0;
}

void
InfoBlock::update(float delta)
{
  Block::update(delta);

  if (delta == 0) return;

  // hide message if player is too far away or above infoblock
  if (dest_pct > 0) {
    Player* player = get_nearest_player();
    if (player) {
      Vector p1 = this->get_pos() + (this->get_bbox().p2 - this->get_bbox().p1) / 2;
      Vector p2 = player->get_pos() + (player->get_bbox().p2 - player->get_bbox().p1) / 2;
      Vector dist = (p2 - p1);
      float d = dist.norm();
      if (d > 128 || dist.y < 0) dest_pct = 0;
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
  float x1 = (get_bbox().p1.x + get_bbox().p2.x)/2 - width/2;
  float x2 = (get_bbox().p1.x + get_bbox().p2.x)/2 + width/2;
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

    lines[i]->draw(context, Rect(x1, y, x2, y), LAYER_GUI-50+1);
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
