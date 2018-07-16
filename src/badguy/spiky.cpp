#include "badguy/spiky.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

Spiky::Spiky(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/spiky/spiky.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 600;
}

void
Spiky::grab(MovingObject&, const Vector& pos, Direction dir_)
{
  assert(frozen);
  movement = pos - get_pos();
  dir = dir_;
  sprite->set_action(dir_ == LEFT ? "iced-left" : "iced-right");
  set_colgroup_active(COLGROUP_DISABLED);
  grabbed = true;
}

void
Spiky::ungrab(MovingObject& , Direction dir_)
{
  dir = dir_;
  set_colgroup_active(COLGROUP_MOVING);
  grabbed = false;
}

bool
Spiky::is_freezable() const
{
  return true;
}

bool
Spiky::is_flammable() const
{
  return true;
}

bool
Spiky::is_portable() const
{
  return frozen;
}
