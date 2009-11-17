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
#include "supertux/main.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

TextObject::TextObject(std::string name) :
  font(),
  text(),
  fading(0), 
  fadetime(0), 
  visible(false), 
  centered(),
  anchor(ANCHOR_MIDDLE),
  pos(0, 0)
{
  this->name = name;
  font = normal_font;
  centered = false;
}

TextObject::~TextObject()
{
}

void
TextObject::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  Scripting::expose_object(vm, table_idx, dynamic_cast<Scripting::Text *>(this), name, false);
}

void
TextObject::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  Scripting::unexpose_object(vm, table_idx, name);
}

void
TextObject::set_font(const std::string& name)
{
  if(name == "normal") {
    font = normal_font;
  } else if(name == "big") {
    font = big_font;
  } else if(name == "small") {
    font = small_font;
  } else {
    log_warning << "Unknown font '" << name << "'." << std::endl;
    font = normal_font;
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
TextObject::set_centered(bool centered)
{
  this->centered = centered;
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
  Vector spos = pos + get_anchor_pos(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
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
