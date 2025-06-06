//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "gui/item_colorchannel_saturation.hpp"

#include <sstream>

#include "math/util.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

namespace {

std::string sat_value_to_string(float v_raw)
{
  float percent = v_raw * 100.0f;
  // Round to nearest integer percentage
  percent = 0.01f * floorf(percent * 100.0f + 0.5f);
  std::ostringstream os;
  os << v_raw << " (" << percent << " %" << ")";
  return os.str();
}

std::string float_to_string(float v)
{
  std::ostringstream os;
  os << v;
  return os.str();
}

} // namespace

ItemColorChannelSaturation::ItemColorChannelSaturation(Color* color, ColorOKLCh* okl, int id) :
  MenuItem(sat_value_to_string(okl->C), id),
  m_color(color),
  m_okl(okl),
  m_sat(&okl->C),
  m_sat_prev(okl->C),
  m_edit_mode(false),
  m_flickw(static_cast<int>(Resources::normal_font->get_text_width("_")))
  //m_channel(channel)
{
}

void
ItemColorChannelSaturation::draw(DrawingContext& context, const Vector& pos,
  int menu_width, bool active)
{
  if (!m_edit_mode && *m_sat != m_sat_prev) {
    set_text(sat_value_to_string(*m_sat));
    m_sat_prev = *m_sat;
  }

  MenuItem::draw(context, pos, menu_width, active);
  *m_okl = ColorOKLCh(*m_color);
  float maxC = m_okl->get_max_chroma();
  float fraction = 0.0f;
  if (maxC > 0.0f) fraction = std::clamp((*m_sat / 1.0f), 0.0f, 1.0f);
  const float lw = float(menu_width - 32) * (fraction);
  context.color().draw_filled_rect(Rectf(pos + Vector(16, -4),
                                         pos + Vector(16 + lw, 4)),
                                   Color::WHITE, 0.0f, LAYER_GUI-1);
}

int
ItemColorChannelSaturation::get_width() const
{
  return static_cast<int>(Resources::normal_font->get_text_width(get_text()) + 16 + static_cast<float>(m_flickw));
}

void
ItemColorChannelSaturation::enable_edit_mode()
{
  if (m_edit_mode)
    // Do nothing if it is already enabled
    return;
  m_edit_mode = true;
  set_text(float_to_string(*m_sat));
}


void
ItemColorChannelSaturation::event(const SDL_Event& ev)
{
  if (ev.type == SDL_TEXTINPUT) {
    std::string txt = ev.text.text;
    for (auto& c : txt) {
      add_char(c);
    }
  }
}

void
ItemColorChannelSaturation::add_char(char c)
{
  enable_edit_mode();
  std::string text = get_text();

  if (c == '.' || c == ',')
  {
    const bool has_comma = (text.find('.') != std::string::npos);
    if (!has_comma)
    {
      if (text.empty()) {
        text = "0.";
      } else {
        text.push_back('.');
      }
    }
  }
  else if (isdigit(c))
  {
    text.push_back(c);
  }
  else
  {
    return;
  }

  float number = std::stof(text);
  if (0.0f <= number && number <= 1.0f) {
    *m_sat = number;
    set_text(text);
  }
}

void
ItemColorChannelSaturation::remove_char()
{
  enable_edit_mode();
  std::string text = get_text();

  if (text.empty())
  {
    *m_sat = 0.0f;
  }
  else
  {
    text.pop_back();

    if (!text.empty()) {
      *m_sat = std::stof(text);
    } else {
      *m_sat = 0.0f;
    }
  }

  set_text(text);
}

void
ItemColorChannelSaturation::process_action(const MenuAction& action)
{
  switch (action)
  {
    case MenuAction::REMOVE:
      remove_char();
      break;

    case MenuAction::LEFT: 
    case MenuAction::RIGHT: {
      float maxC = m_okl->get_max_chroma();
      float delta = (action == MenuAction::LEFT ? -0.01f : +0.01f);

      float newSat = *m_sat + delta;
      newSat = std::clamp(newSat, 0.0f, maxC);
      newSat = roundf(newSat * 100.0f) / 100.0f;
      // Snap to reduce jitter at boundaries
      if(newSat < 0.005f) newSat = 0.0f;
      if(newSat > maxC - 0.005f) newSat = maxC;

      *m_sat = newSat;

      float oldA = m_color->alpha;
      *m_color = Color::from_oklch(*m_okl);
      m_color->alpha = oldA;  // if you want to preserve alpha
      m_edit_mode = false;
      break;
    }

    case MenuAction::UNSELECT:
      m_edit_mode = false;
      break;

    default:
      break;
  }
}

// Color
// ItemColorChannelSaturation::get_color() const
// {
//   return m_channel;
// }

/* EOF */
