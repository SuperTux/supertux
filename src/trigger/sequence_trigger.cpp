#include <config.h>

#include "sequence_trigger.h"
#include "gameloop.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "object_factory.h"

SequenceTrigger::SequenceTrigger(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w, h;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);
  reader.get("sequence", sequence_name);
}

SequenceTrigger::SequenceTrigger(const Vector& pos, const std::string& sequence)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sequence_name = sequence;
  triggerevent = EVENT_TOUCH;
}

SequenceTrigger::~SequenceTrigger()
{
}

void
SequenceTrigger::write(lisp::Writer& writer)
{
  writer.start_list("sequencetrigger");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.write_float("width", bbox.get_width());
  writer.write_float("height", bbox.get_height());
  writer.write_string("sequence", sequence_name);

  writer.end_list("sequencetrigger");
}

void
SequenceTrigger::event(Player& , EventType type)
{
  if(type == triggerevent) {
    GameSession::current()->start_sequence(sequence_name);
  }
}

IMPLEMENT_FACTORY(SequenceTrigger, "sequencetrigger")
