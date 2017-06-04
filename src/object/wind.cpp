//  SuperTux - Wind
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "object/wind.hpp"

#include "editor/editor.hpp"
#include "math/random_generator.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

Wind::Wind(const ReaderMapping& reader) :
  ExposedObject<Wind, scripting::Wind>(this),
  blowing(),
  speed(),
  acceleration(),
  new_size(),
  elapsed_time(0)
{
  float w,h;
  reader.get("name", name,"");
  reader.get("x", bbox.p1.x,0);
  reader.get("y", bbox.p1.y,0);
  reader.get("width", w,32);
  reader.get("height", h,32);
  bbox.set_size(w, h);

  reader.get("blowing", blowing,true);

  reader.get("speed-x", speed.x,0);
  reader.get("speed-y", speed.y,0);

  reader.get("acceleration", acceleration,100);

  set_group(COLGROUP_TOUCHABLE);
}

ObjectSettings
Wind::get_settings() {
  new_size.x = bbox.get_width();
  new_size.y = bbox.get_height();
  ObjectSettings result = MovingObject::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, "width", &new_size.x,
                                         "width", false));
  result.options.push_back( ObjectOption(MN_NUMFIELD, "height", &new_size.y,
                                         "height", false));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed X"), &speed.x,
                                         "speed-x"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed Y"), &speed.y,
                                         "speed-y"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Acceleration"), &acceleration,
                                         "acceleration"));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Blowing"), &blowing,
                                         "blowing"));

  return result;
}

void
Wind::update(float elapsed_time_)
{
  this->elapsed_time = elapsed_time_;

  if (!blowing) return;
  if (bbox.get_width() <= 16 || bbox.get_height() <= 16) return;

  // TODO: nicer, configurable particles for wind?
  if (graphicsRandom.rand(0, 100) < 20) {
    // emit a particle
    Vector ppos = Vector(graphicsRandom.randf(bbox.p1.x+8, bbox.p2.x-8), graphicsRandom.randf(bbox.p1.y+8, bbox.p2.y-8));
    Vector pspeed = Vector(speed.x, speed.y);
    Sector::current()->add_object(std::make_shared<Particles>(ppos, 44, 46, pspeed, Vector(0,0), 1, Color(.4f, .4f, .4f), 3, .1f,
                                                LAYER_BACKGROUNDTILES+1));
  }
}

void
Wind::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    context.draw_filled_rect(bbox, Color(0.0f, 1.0f, 1.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

HitResponse
Wind::collision(GameObject& other, const CollisionHit& )
{
  if (!blowing) return ABORT_MOVE;

  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    if (!player->on_ground()) {
      player->add_velocity(speed * acceleration * elapsed_time, speed);
    }
  }

  return ABORT_MOVE;
}

void
Wind::start()
{
  blowing = true;
}

void
Wind::stop()
{
  blowing = false;
}

/* EOF */
