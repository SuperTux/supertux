#include <config.h>

#include "sequence_trigger.h"
#include "utils/lispwriter.h"
#include "gameloop.h"

SequenceTrigger::SequenceTrigger(LispReader& reader)
{
  (void) reader;
  // TODO
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
SequenceTrigger::write(LispWriter& writer)
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
