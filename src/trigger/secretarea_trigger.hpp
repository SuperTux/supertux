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

#ifndef __SECRETAREA_TRIGGER_H__
#define __SECRETAREA_TRIGGER_H__

#include "trigger_base.hpp"

#include "serializable.hpp"
#include "timer.hpp"

namespace lisp {
class Lisp;
class Writer;
}
class DrawingContext;
class Color;

class SecretAreaTrigger : public TriggerBase, public Serializable
{
  static Color text_color;
public:
  SecretAreaTrigger(const lisp::Lisp& reader);
  SecretAreaTrigger(const Rect& area, std::string fade_tilemap = "");
  ~SecretAreaTrigger();

  void write(lisp::Writer& writer);
  void event(Player& player, EventType type);
  void draw(DrawingContext& context);

private:
  Timer message_timer;
  bool message_displayed;
  std::string fade_tilemap; /**< tilemap to fade away when trigger is activated, or empty if you don't care */
};

#endif
