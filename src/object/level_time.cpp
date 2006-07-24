//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "level_time.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include "main.hpp"
#include "resources.hpp"
#include "sector.hpp"
#include "gettext.hpp"
#include "object_factory.hpp"
#include "object/player.hpp"
#include "video/drawing_context.hpp"
#include "lisp/list_iterator.hpp"
#include "log.hpp"

/** When to alert player they're low on time! */
static const float TIME_WARNING = 20;

LevelTime::LevelTime(const lisp::Lisp& reader)
: running(true), time_left(0)
{
  reader.get("time", time_left);
  if(time_left <= 0) throw std::runtime_error("No or invalid leveltime specified");
}

void
LevelTime::update(float elapsed_time)
{
  if (!running) return;

  time_left = std::max(time_left - elapsed_time, 0.0f);
  if(time_left <= 0) {
    Sector::current()->player->kill(true);
    stop();
  }
}

void
LevelTime::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  if ((time_left > TIME_WARNING) || (int(game_time * 2.5) % 2)) {
    std::stringstream ss;
    ss << int(time_left);
    std::string time = ss.str();

    float caption_width = white_text->get_text_width(_("TIME")) + white_text->get_text_width(" ");
    float all_width = caption_width + white_text->get_text_width(time);

    context.draw_text(white_text, _("TIME"), Vector((SCREEN_WIDTH - all_width)/2, BORDER_Y), LEFT_ALLIGN, LAYER_FOREGROUND1);
    context.draw_text(gold_text, time, Vector((SCREEN_WIDTH - all_width)/2 + caption_width, BORDER_Y), LEFT_ALLIGN, LAYER_FOREGROUND1);
  }

  context.pop_transform();
}

void
LevelTime::stop()
{
  running = false;
}

IMPLEMENT_FACTORY(LevelTime, "leveltime");
