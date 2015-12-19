//  SuperTux - PneumaticPlatform
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

#include "object/pneumatic_platform.hpp"

#include "object/player.hpp"
#include "object/portable.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"

PneumaticPlatform::PneumaticPlatform(const Reader& reader) :
  MovingSprite(reader, LAYER_OBJECTS, COLGROUP_STATIC),
  master(0),
  slave(0),
  start_y(0),
  offset_y(0),
  speed_y(0),
  contacts()
{
  start_y = get_pos().y;
}

PneumaticPlatform::PneumaticPlatform(PneumaticPlatform* master_) :
  MovingSprite(*master_),
  master(master_),
  slave(this),
  start_y(master_->start_y),
  offset_y(-master_->offset_y),
  speed_y(0),
  contacts()
{
  set_pos(get_pos() + Vector(master->get_bbox().get_width(), 0));
  master->master = master;
  master->slave = this;
}

PneumaticPlatform::~PneumaticPlatform()
{
  if ((this == master) && (master)) {
    slave->master = 0;
    slave->slave = 0;
  }
  if ((master) && (this == slave)) {
    master->master = 0;
    master->slave = 0;
  }
  master = 0;
  slave = 0;
}

HitResponse
PneumaticPlatform::collision(GameObject& other, const CollisionHit& )
{

  // somehow the hit parameter does not get filled in, so to determine (hit.top == true) we do this:
  MovingObject* mo = dynamic_cast<MovingObject*>(&other);
  if (!mo) return FORCE_MOVE;
  if ((mo->get_bbox().p2.y) > (bbox.p1.y + 2)) return FORCE_MOVE;

  Player* pl = dynamic_cast<Player*>(mo);
  if (pl) {
    if (pl->is_big()) contacts.insert(0);
    Portable* po = pl->get_grabbed_object();
    MovingObject* pomo = dynamic_cast<MovingObject*>(po);
    if (pomo) contacts.insert(pomo);
  }

  contacts.insert(&other);
  return FORCE_MOVE;
}

void
PneumaticPlatform::update(float elapsed_time)
{
  if (!slave) {
    Sector::current()->add_object(std::make_shared<PneumaticPlatform>(this));
    return;
  }
  if (!master) {
    return;
  }
  if (this == slave) {
    offset_y = -master->offset_y;
    movement = Vector(0, (start_y + offset_y) - get_pos().y);
  }
  if (this == master) {
    int contact_diff = contacts.size() - slave->contacts.size();
    contacts.clear();
    slave->contacts.clear();

    speed_y += ((float)contact_diff * elapsed_time) * 12.8f;
    speed_y -= (offset_y * elapsed_time * 0.05f);
    speed_y *= 1 - elapsed_time;
    offset_y += speed_y * elapsed_time * Sector::current()->get_gravity();
    if (offset_y < -256) { offset_y = -256; speed_y = 0; }
    if (offset_y > 256) { offset_y = 256; speed_y = -0; }
    movement = Vector(0, (start_y + offset_y) - get_pos().y);
  }
}


ObjectSettings
PneumaticPlatform::get_settings() {
  ObjectSettings result(_("Pneumatic platform"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));

  return result;
}

/* EOF */
