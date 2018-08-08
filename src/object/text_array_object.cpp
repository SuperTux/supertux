//  SuperTux
//  Copyright (C) 2018 Nir <goproducti@gmail.com>
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

#include "object/text_array_object.hpp"
#include "control/input_manager.hpp"

TextArrayObject::TextArrayObject(const std::string& name_) :
                                  ExposedObject<TextArrayObject, scripting::TextArray>(this),
                                  m_isDone(false),
                                  m_isAuto(false),
                                  m_keepVisible(false),
                                  m_fadeTransition(true),
                                  m_fadetime(1.0),
                                  m_texts(),
                                  m_curTextIndex(0),
                                  m_lastTextIndex(0),
                                  m_waiting()
{
  name = name_;
}

void TextArrayObject::clear()
{
  this->m_texts.clear();
  this->reset_automation();
}

void TextArrayObject::add_text(const std::string& text_, float duration_)
{
  std::unique_ptr<TextArrayItem> pText(new TextArrayItem());
  assert(pText);

  pText->duration = duration_;
  pText->text_object.set_text(text_);

  this->m_texts.push_back(std::move(pText));
}

void TextArrayObject::set_text_index(ta_index index)
{
  if (index < m_texts.size())
    m_curTextIndex = index;
}

void TextArrayObject::set_fade_time(float fadetime)
{
  this->m_fadetime = fadetime;
}

void TextArrayObject::next_text()
{
  if (m_isDone)
    return;

  if (m_curTextIndex+1 >= m_texts.size()) {
    m_isDone = true;
    return;
  }

  m_lastTextIndex = m_curTextIndex++;

  this->override_properties();
  this->reset_automation();
}

void TextArrayObject::prev_text()
{
  if (m_isDone)
    return;

  if (m_curTextIndex == 0)
    return;

  m_lastTextIndex = m_curTextIndex--;

  this->override_properties();
  this->reset_automation();
}

void TextArrayObject::set_keep_visible(bool keep_visible)
{
  m_keepVisible = keep_visible;
}

void TextArrayObject::set_fade_transition(bool fade_transition)
{
  m_fadeTransition = fade_transition;
}

TextArrayItem* TextArrayObject::get_text_item(ta_index index)
{
  auto vecSize = m_texts.size();

  if (vecSize == 0 || index >= vecSize)
    return nullptr;

  return this->m_texts.at(index).get();
}

TextArrayItem* TextArrayObject::get_current_text_item()
{
  return this->get_text_item(m_curTextIndex);
}

TextArrayItem* TextArrayObject::get_last_text_item()
{
  return this->get_text_item(m_lastTextIndex);
}

void TextArrayObject::set_done(bool done)
{
  this->m_isDone = done;
}

void TextArrayObject::set_auto(bool is_auto)
{
  m_isAuto = is_auto;
  this->reset_automation();
}

void TextArrayObject::update(float elapsed_time)
{
  if (m_isDone)
    return;

  // make sure there's anything to update
  if (m_texts.size() == 0)
    return;

  // detect change request
  this->handle_input_requests();

  // check if if should update auto narration
  if (m_isAuto && m_waiting.check()) {
    next_text();
  }

  // update current
  auto* curTextItem = this->get_current_text_item();
  if (curTextItem)
    curTextItem->text_object.update(elapsed_time);

  // update last (if transition is enabled)

  if (this->should_fade()) {
    auto* lastTextItem = this->get_last_text_item();
    if (lastTextItem)
      lastTextItem->text_object.update(elapsed_time);
  }
}

void TextArrayObject::draw(DrawingContext& context)
{
  if (m_isDone)
    return;

  auto* curTextItem = this->get_current_text_item();
  if (!curTextItem)
    return;

  // draw last if transition enabled
  if (this->should_fade()) {
    auto* lastTextItem = this->get_last_text_item();
    if (lastTextItem)
      lastTextItem->text_object.draw(context);
  }

  // draw current
  curTextItem->text_object.draw(context);
}

void TextArrayObject::override_properties()
{
  if (!(this->should_fade() || m_keepVisible))
    return;

  auto* curTextItem = this->get_current_text_item();
  if (!curTextItem)
    return;

  // apply overrides
  if (this->should_fade()) { // make fade transition
    auto* lastTextItem = this->get_last_text_item();
    if (lastTextItem) {
      lastTextItem->text_object.fade_out(m_fadetime);
      curTextItem->text_object.fade_in(m_fadetime);
    }
  } else if (m_keepVisible) { // keep visible
    curTextItem->text_object.set_visible(true);
  }
}

void TextArrayObject::reset_automation()
{
  m_waiting.stop();

  if (m_isAuto) {
    auto* text = this->get_current_text_item();
    if (text)
      m_waiting.start(text->duration);
  }
}

void TextArrayObject::handle_input_requests()
{
  auto* inputManager = InputManager::current();
  if (!inputManager)
    return;

  auto* inputController = inputManager->get_controller();
  if (!inputController)
    return;

  if (inputController->pressed(Controller::MENU_SELECT)) {
    m_isAuto = false;
    next_text();
  } else if(inputController->pressed(Controller::REMOVE)) {
    m_isAuto = false;
    prev_text();
  }

}

bool TextArrayObject::should_fade()
{
  return m_fadeTransition && (m_curTextIndex != m_lastTextIndex);
}

/* EOF */
