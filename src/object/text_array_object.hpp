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

#ifndef HEADER_SUPERTUX_OBJECT_TEXT_ARRAY_OBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_TEXT_ARRAY_OBJECT_HPP

#include <memory>

#include "scripting/exposed_object.hpp"
#include "scripting/text_array.hpp"

#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"

#include "object/text_object.hpp"
#include "object/text_array_item.hpp"

typedef size_t ta_index;

/**
 * A text array object intended for narration
 */
class TextArrayObject : public GameObject,
                        public ExposedObject<TextArrayObject, scripting::TextArray>
{
public:

  /**
   * TextArrayObject constructor
   *
   * @param: name_  name of GameObject.
   */
  TextArrayObject(const std::string& name_ = std::string());

  /**
   * TextArrayObject destructor (default)
   */
  ~TextArrayObject() = default;

  /////////// TextArray api related ///////////

  /**
   * Empties the text array.
   */
  void clear();

  /**
   * Adds a text with duration.
   *
   * @param: text_      the text itself (can be multiline & formatted).
   * @param: duration_  (optional) the text display time in seconds, defaults to 3.
   */
  void add_text(const std::string& text_, float duration_  = 3.0f);

  /**
   * Sets the current text index.
   *
   * @param: index_ the index to set to.
   */
  void set_text_index(ta_index index_);

  /**
   * Sets the keep visible flag.
   * This flag overrides all texts to be visible.
   *
   * @note: fade_transition overrides this
   *
   * @param: keep_visible  true to enable keep_visible; false to disable the flag.
   */
  void set_keep_visible(bool keep_visible);

  /**
   * Sets the fade transition flag.
   * This flag overrides all texts to be visible and fading.
   *
   * @note: overrides keep_visible flag
   */
  void set_fade_transition(bool fade_transition);

  /**
   * Sets fadetime for fade_transition.
   *
   * @param: fadetime  the fade time.
   *
   * @note: does NOT override the TextArray::fade_in() method.
   */
  void set_fade_time(float fadetime);

  /**
   * Sets the done flag as on. This disables the text array.
   *
   * @note: the text array is not cleared.
   *
   * @param: done  true for on; false for off.
   */
  void set_done(bool done);

  /**
   * Sets the auto flag on & starts the auto narration.
   *
   * @note: this starts the auto narration immediately!
   *        this is disabled once the user inputs a skip!
   */
  void set_auto(bool is_auto);

  /**
   * Sets the current text to the next one.
   *
   * @note: if the text is the last on the array,
   *        the done flag is set, and the text array is disabled.
   */
  void next_text();

  /**
   * Sets the current text to the previous.
   *
   * @note: if the current text is the first on the array,
   *        it stays that way.
   */
  void prev_text();

  /////////// TextArrayObject access ///////////

  /**
   * Gets the text item at a certain index.
   *
   * @param: index  the index of the text item to get.
   *
   * @return: pointer to the text array item; or nullptr if fails.
   */
  TextArrayItem* get_text_item(ta_index index);

  /**
   * Gets the current text item.
   *
   * @return: pointer the current text array item; or nullptr if fails.
   */
  TextArrayItem* get_current_text_item();

  /**
   * Gets the last text item.
   *
   * @return: pointer to the last text item; or nullptr if fails.
   */
  TextArrayItem* get_last_text_item();

  /////////// GameObject api related ///////////

  /**
   * Overrides draw for GameObject.
   * Draws current text / transitions between last and current.
   *
   * @see: GameObject, TextObject
   *
   * @param: context  the drawing context.
   */
  void draw(DrawingContext& context);

  /**
   * Updates the text array.
   *
   * @see: GameObject, TextObject
   *
   * @param: elapsed_time  the elapsed time.
   */
  void update(float elapsed_time);

  /////////// internals  ///////////

private:
  /**
   * Overrides the properties of the text objects, according to the flags.
   */
  void override_properties();

  /**
   * Resets the auto narration state and updates it if necessary.
   */
  void reset_automation();

  /**
   * Handles user input requests (skipping, rewinding)
   *
   * @note: might change to manual mode (disables auto flag)
   */
  void handle_input_requests();

  /**
   * Should fade transition logic apply
   *
   * @return: true if fadeTransition flag is on & the transition is valid;
   *          false otherwise.
   */
  bool should_fade();

private:
  /** flags */
  bool m_isDone;
  bool m_isAuto;
  bool m_keepVisible;
  bool m_fadeTransition;

  /** fade transition related */
  float m_fadetime;

  /** text items related */
  std::vector<std::unique_ptr<TextArrayItem> > m_texts;
  ta_index m_curTextIndex, m_lastTextIndex;

  /** timer for auto narration */
  Timer m_waiting;
};

#endif // HEADER_SUPERTUX_OBJECT_TEXT_ARRAY_OBJECT_HPP

/* EOF */
