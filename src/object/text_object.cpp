//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/text_object.hpp"

#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

TextObject::TextObject(const std::string& name_) :
  ExposedObject<TextObject, scripting::Text>(this),
  font(Resources::normal_font),
  text(),
  fading(0),
  fadetime(0),
  visible(false),
  centered(false),
  anchor(ANCHOR_MIDDLE),
  pos(0, 0)
{
  this->name = name_;
}

TextObject::~TextObject()
{
}

void
TextObject::set_font(const std::string& name_)
{
  if(name_ == "normal") {
    font = Resources::normal_font;
  } else if(name_ == "big") {
    font = Resources::big_font;
  } else if(name_ == "small") {
    font = Resources::small_font;
  } else {
    log_warning << "Unknown font '" << name_ << "'." << std::endl;
    font = Resources::normal_font;
  }
}

void
TextObject::set_text(const std::string& text_)
{
  this->text = text_;
}

void
TextObject::fade_in(float fadetime_)
{
  this->fadetime = fadetime_;
  fading = fadetime_;
}

void
TextObject::fade_out(float fadetime_)
{
  this->fadetime = fadetime_;
  fading = -fadetime_;
}

void
TextObject::set_visible(bool visible_)
{
  this->visible = visible_;
  fading = 0;
}

void
TextObject::set_centered(bool centered_)
{
  this->centered = centered_;
}

void
TextObject::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));
  if(fading > 0) {
    context.set_alpha((fadetime-fading) / fadetime);
  } else if(fading < 0) {
    context.set_alpha(-fading / fadetime);
  } else if(!visible) {
    context.pop_transform();
    return;
  }

  float width  = 500;
  float height = 70;
  Vector spos = pos + get_anchor_pos(Rectf(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
                                     width, height, anchor);

  context.draw_filled_rect(spos, Vector(width, height),
                           Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI-50);
  if (centered) {
    context.draw_center_text(font, text, spos, LAYER_GUI-40, TextObject::default_color);
  } else {
    context.draw_text(font, text, spos + Vector(10, 10), ALIGN_LEFT, LAYER_GUI-40, TextObject::default_color);
  }

  context.pop_transform();
}

void
TextObject::update(float elapsed_time)
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

/* EOF */
