#include "spike.h"

Spike::Spike(const Vector& pos, Direction dir)
{
  sprite = sprite_manager->create("spike");
  start_position = pos;
  bbox.set_pos(Vector(0, 0));
  bbox.set_size(32, 32);
  set_direction(dir);
}

Spike::Spike(const lisp::Lisp& reader)
{
  sprite = sprite_manager->create("spike");
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(32, 32);
  int idir = 0;
  reader.get("direction", idir);
  set_direction((Direction) idir);
}

void
Spike::set_direction(Direction dir)
{
  spikedir = dir;
  switch(spikedir) {
    case NORTH:
      sprite->set_action("north");
      break;
    case SOUTH:
      sprite->set_action("south");
      break;
    case WEST:
      sprite->set_action("west");
      break;
    case EAST:
      sprite->set_action("east");
      break;
    default:
      break;
  }
}

void
Spike::write(lisp::Writer& writer)
{
  writer.start_list("spike");
  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.write_int("direction", spikedir);
  writer.end_list("spike");
}

void
Spike::kill_fall()
{
  // you can't kill a spike
}

void
Spike::active_action(float )
{
}
