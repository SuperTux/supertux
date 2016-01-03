//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "badguy/walking_candle.hpp"

#include "object/lantern.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"

WalkingCandle::WalkingCandle(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/mr_candle/mr-candle.sprite", "left", "right"),
    lightcolor(1, 1, 1)
{
  walk_speed = 80;
  max_drop_height = 64;

  std::vector<float> vColor;
  if (reader.get("color", vColor)) {
    lightcolor = Color(vColor);
  }
  sprite->set_color(lightcolor);
  lightsprite->set_color(lightcolor);

  countMe = false;
  glowing = true;
}

bool
WalkingCandle::is_freezable() const
{
  return true;
}

bool
WalkingCandle::is_flammable() const
{
  return frozen;
}

void
WalkingCandle::freeze() {
  BadGuy::freeze();
  glowing = false;
}

void
WalkingCandle::unfreeze() {
  BadGuy::unfreeze();
  glowing = true;
}

HitResponse
WalkingCandle::collision(GameObject& other, const CollisionHit& hit) {
  Lantern* l = dynamic_cast<Lantern*>(&other);
  if (l && !frozen) if (l->get_bbox().p2.y < bbox.p1.y) {
    l->add_color(lightcolor);
    run_dead_script();
    remove_me();
    return FORCE_MOVE;
  }
  return WalkingBadguy::collision(other, hit);
}

ObjectSettings
MrCandle::get_settings() {
  ObjectSettings result(_("Mr. Candle"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  result.options.push_back( ObjectOption(MN_COLOR, _("Colour"), &lightcolor));
  return result;
}

void
MrCandle::after_editor_set() {
  sprite->set_color(lightcolor);
  candle_light->set_color(lightcolor);
}

void
MrCandle::save(Writer& writer) {
  BadGuy::save(writer);
  writer.write("color", lightcolor.toVector(false));
}

/* EOF */
