#include <config.h>

#include "secretarea_trigger.h"
#include "utils/lispwriter.h"
#include "gameloop.h"

SecretAreaTrigger::SecretAreaTrigger(LispReader& reader)
{
  reader.read_float("x", bbox.p1.x);
  reader.read_float("y", bbox.p1.y);
  bbox.set_size(32, 32);

  reader.read_string("message", message);
}

SecretAreaTrigger::SecretAreaTrigger(const Vector& pos,
    const std::string& secretarea)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  triggerevent = EVENT_TOUCH;
  show_message = 0;
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
   if (show_message == 1) {
      context.draw_center_text(gold_text, message, Vector(0, screen->h/2 - gold_text->get_height()/2), LAYER_GUI);
      std::cout<<message<<std::endl;
   }
}

void
SecretAreaTrigger::event(Player& , EventType type)
{
  if(type == triggerevent) {
    show_message = 1;
  }
}
