#include <config.h>

#include "secretarea_trigger.h"
#include "utils/lispwriter.h"
#include "gameloop.h"

#define MESSAGE_TIME 3.5

//TODO: Count numbers of triggered/total secret areas
SecretAreaTrigger::SecretAreaTrigger(LispReader& reader)
{
  reader.read_float("x", bbox.p1.x);
  reader.read_float("y", bbox.p1.y);
  bbox.set_size(32, 32);

  reader.read_string("message", message);
  message_displayed = false;
}

SecretAreaTrigger::SecretAreaTrigger(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  message = "You found a secret area!";
  message_displayed = false;
}

SecretAreaTrigger::~SecretAreaTrigger()
{
}

void
SecretAreaTrigger::write(LispWriter& writer)
{
  writer.start_list("secretarea");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.write_float("width", bbox.get_width());
  writer.write_float("height", bbox.get_height());
  writer.write_string("message", message);

  writer.end_list("secretarea");
}

void
SecretAreaTrigger::draw(DrawingContext& context)
{
   if (message_timer.started()) {
      context.push_transform();
      context.set_translation(Vector(0, 0));
      Vector pos = Vector(0, screen->h/2 - gold_text->get_height()/2);
      context.draw_center_text(gold_text, message, pos, LAYER_GUI);
      context.pop_transform();
   }
   if (message_timer.check()) {
      remove_me();
   }
}

void
SecretAreaTrigger::event(Player& , EventType type)
{
  if(type == EVENT_TOUCH) {
    if (!message_displayed) {
      message_timer.start(MESSAGE_TIME);
      message_displayed = true;
    }
  }
}
