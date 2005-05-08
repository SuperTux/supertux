#include <config.h>

#include "text_object.h"
#include "resources.h"
#include "video/drawing_context.h"

TextObject::TextObject()
  : fading(0), fadetime(0), visible(false)
{
  font = blue_text;
}

TextObject::~TextObject()
{
}

void
TextObject::set_font(const std::string& name)
{
  if(name == "gold") {
    font = gold_text;
  } else if(name == "white") {
    font = white_text;
  } else if(name == "blue") {
    font = blue_text;
  } else if(name == "gray") {
    font = gray_text;
  } else if(name == "white") {
    font = white_text;
  } else if(name == "big") {
    font = white_big_text;
  } else if(name == "small") {
    font = white_small_text;
  } else {
    std::cerr << "Unknown font '" << name << "'.\n";
  }
}

void
TextObject::set_text(const std::string& text)
{
  this->text = text;
}

void
TextObject::fade_in(float fadetime)
{
  this->fadetime = fadetime;
  fading = fadetime;
}

void
TextObject::fade_out(float fadetime)
{
  this->fadetime = fadetime;
  fading = -fadetime;
}

void
TextObject::set_visible(bool visible)
{
  this->visible = visible;
  fading = 0;
}

void
TextObject::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));
  if(fading > 0) {
    context.set_alpha(static_cast<uint8_t> 
        ((fadetime-fading) * 255.0 / fadetime));
  } else if(fading < 0) {
    context.set_alpha(static_cast<uint8_t> (-fading * 255.0 / fadetime));
  } else if(!visible) {
    context.pop_transform();
    return;
  }

  context.draw_filled_rect(Vector(125, 50), Vector(550, 120),
      Color(150, 180, 200, 125), LAYER_GUI-50);
  context.draw_text(font, text, Vector(125+35, 50+35), LEFT_ALLIGN, LAYER_GUI-40);

  context.pop_transform();
}

void
TextObject::action(float elapsed_time)
{
  if(fading > 0) {
    fading -= elapsed_time;
    if(fading <= 0) {
      fading = 0;
      visible = true;
    }
  } else if(fading < 0) {
    fading += elapsed_time;
    if(fading >= 0) {
      fading = 0;
      visible = false;
    }
  }
}

