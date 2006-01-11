#include <config.h>

#include "level_time.hpp"

#include <stdexcept>
#include <iostream>
#include "main.hpp"
#include "resources.hpp"
#include "sector.hpp"
#include "gettext.hpp"
#include "object_factory.hpp"
#include "object/player.hpp"
#include "video/drawing_context.hpp"
#include "lisp/list_iterator.hpp"

/** When to alert player they're low on time! */
static const float TIME_WARNING = 20;

LevelTime::LevelTime(const lisp::Lisp& reader)
{
    float time = -1;
    lisp::ListIterator iter(&reader);
    while(iter.next()) {
        if(iter.item() == "time") {
            iter.value()->get(time);
            break;
        } else {
            std::cerr << "Unknown token '" << iter.item() 
                      << "' in LevelTime object.\n";
        }
    }
    if(time < 0)
      throw std::runtime_error("Invalid leveltime specified");
    time_left.start(time);
}

LevelTime::~LevelTime()
{}

void
LevelTime::update(float )
{
  if(time_left.check()) {
    Sector::current()->player->kill(Player::KILL);
  }
}

void
LevelTime::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  char str[60];
    
  if(time_left.get_timeleft() < 0) {
    context.draw_text(white_text, _("TIME's UP"), Vector(SCREEN_WIDTH/2, BORDER_Y),
        CENTER_ALLIGN, LAYER_FOREGROUND1);
  } else if (time_left.get_timeleft() > TIME_WARNING
      || int(game_time * 2.5) % 2) {
    snprintf(str, sizeof(str), " %d", int(time_left.get_timeleft()));
    context.draw_text(white_text, _("TIME"),
        Vector(SCREEN_WIDTH/2, BORDER_Y), CENTER_ALLIGN, LAYER_FOREGROUND1);
    context.draw_text(gold_text, str, Vector(SCREEN_WIDTH/2 + 4*16, BORDER_Y),
                      CENTER_ALLIGN, LAYER_FOREGROUND1);
  }

  context.pop_transform();
}

float
LevelTime::get_level_time()
{
  return time_left.get_period();
}

float
LevelTime::get_remaining_time()
{
  return time_left.get_timeleft();
}

IMPLEMENT_FACTORY(LevelTime, "leveltime");
