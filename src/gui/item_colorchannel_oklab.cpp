//  SuperTux
//  Copyright (C) 2021
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

#include "gui/item_colorchannel.hpp"

#include <vector>

#include "math/util.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"


// This value affects the gamut clipping in the hue selection.
// A bigger value means more preserving of chroma instead of lightness.
constexpr float HUE_COLORFULNESS = 0.5f;

ItemColorChannelOKLab::ItemColorChannelOKLab(Color* col, int channel,
    Menu* menu) :
  MenuItem(""),
  m_color(col),
  m_col_prev(0, 0, 0),
  m_channel(ChannelType::CHANNEL_L),
  m_menu(menu),
  m_mousedown(false)
{
  if (channel == 1)
    m_channel = ChannelType::CHANNEL_L;
  else if (channel == 2)
    m_channel = ChannelType::CHANNEL_C;
  else
    m_channel = ChannelType::CHANNEL_H;
}

void
ItemColorChannelOKLab::draw(DrawingContext& context, const Vector& pos,
  int menu_width, bool active)
{
  const float lw = static_cast<float>(menu_width - 32);
  ColorOKLCh col_oklch(0, 0, 0);
  if (active) {
    col_oklch = m_col_prev;
  } else {
    col_oklch = ColorOKLCh(*m_color);
  }

  // Draw all possible colour values for the given component
  float chroma_max_any_l = 1.0f;
  if (m_channel == ChannelType::CHANNEL_C)
    chroma_max_any_l = col_oklch.get_maximum_chroma_any_l();
  constexpr int NUM_RECTS = 128;
  std::vector<Color> colors(NUM_RECTS+1);
  for (int i = 0; i < NUM_RECTS+1; ++i) {
    ColorOKLCh col_oklch_current = col_oklch;
    float x = static_cast<float>(i) / NUM_RECTS;
    if (m_channel == ChannelType::CHANNEL_L) {
      col_oklch_current.L = x;
    } else if (m_channel == ChannelType::CHANNEL_C) {
      col_oklch_current.C = x * chroma_max_any_l;
      col_oklch_current.clip_lightness();
    } else {
      col_oklch_current.h = (2.0f * x - 1.0f) * math::PI;
      col_oklch_current.clip_adaptive_L0_L_cusp(HUE_COLORFULNESS);
    }
    colors[i] = col_oklch_current.to_srgb();
  }
  for (int i = 0; i < NUM_RECTS; ++i) {
    float x1 = 16 + static_cast<float>(i) * lw / NUM_RECTS;
    float x2 = x1 + lw / NUM_RECTS;
    context.color().draw_gradient(colors[i], colors[i+1], LAYER_GUI-1,
      GradientDirection::HORIZONTAL,
      Rectf(pos + Vector(x1, -10), pos + Vector(x2, 10)));
  }

  // Draw a marker for the current colour
  float x_marker;
  if (m_channel == ChannelType::CHANNEL_L) {
    x_marker = col_oklch.L;
  } else if (m_channel == ChannelType::CHANNEL_C) {
    x_marker = chroma_max_any_l > 0.0f ? col_oklch.C / chroma_max_any_l : 0.0f;
  } else {
    x_marker = 0.5f * col_oklch.h / math::PI + 0.5f;
  }
  x_marker = pos.x + 16 + x_marker * lw;
  context.color().draw_triangle(Vector(x_marker - 3, pos.y - 11),
    Vector(x_marker + 3, pos.y - 11), Vector(x_marker, pos.y - 4),
    Color::WHITE, LAYER_GUI-1);
  context.color().draw_triangle(Vector(x_marker, pos.y + 4),
    Vector(x_marker - 3, pos.y + 11), Vector(x_marker + 3, pos.y + 11),
    Color::BLACK, LAYER_GUI-1);

  if (m_channel == ChannelType::CHANNEL_C && chroma_max_any_l > 0.0f) {
    // Draw a marker where the lightness clipping starts
    x_marker = col_oklch.get_maximum_chroma() / chroma_max_any_l;
    x_marker = pos.x + 16 + x_marker * lw;
    context.color().draw_triangle(Vector(x_marker - 2, pos.y - 11),
      Vector(x_marker + 2, pos.y - 11), Vector(x_marker, pos.y),
      Color(0.73f, 0.73f, 0.73f), LAYER_GUI-1);
  }
}

void
ItemColorChannelOKLab::process_action(const MenuAction& action)
{
  if (action == MenuAction::SELECT) {
    m_col_prev = ColorOKLCh(*m_color);
    return;
  }
  float increment;
  if (action == MenuAction::LEFT)
    increment = -0.1f;
  else if (action == MenuAction::RIGHT)
    increment = 0.1f;
  else
    return;

  ColorOKLCh col_oklch = m_col_prev;
  ColorOKLCh col_oklch_clipped(0, 0, 0);
  if (m_channel == ChannelType::CHANNEL_L) {
    col_oklch.L = math::clamp(col_oklch.L + increment, 0.0f, 1.0f);
    col_oklch_clipped = col_oklch;
  } else if (m_channel == ChannelType::CHANNEL_C) {
    float chroma_max = col_oklch.get_maximum_chroma_any_l();
    increment *= chroma_max;
    col_oklch.C = math::clamp(col_oklch.C + increment, 0.0f, chroma_max);
    col_oklch_clipped = col_oklch;
    col_oklch_clipped.clip_lightness();
  } else {
    increment *= 3.0f;
    col_oklch.h = fmodf(col_oklch.h + increment + 3.0f * math::PI,
      2.0f * math::PI) - math::PI;
    col_oklch_clipped = col_oklch;
    col_oklch_clipped.clip_adaptive_L0_L_cusp(HUE_COLORFULNESS);
  }
  set_color(col_oklch_clipped, col_oklch);
}

void
ItemColorChannelOKLab::event(const SDL_Event& ev)
{
  // Determine the new colour with the mouse position if either the mouse
  // is clicked once or clicked and held down
  bool is_mouseclick = ev.type == SDL_MOUSEBUTTONDOWN
    && ev.button.button == SDL_BUTTON_LEFT;
  bool is_hold_mousemove = ev.type == SDL_MOUSEMOTION
    && (ev.motion.state & SDL_BUTTON_LMASK);
  if (is_mouseclick) {
    m_mousedown = true;
  } else if (!is_hold_mousemove || !m_mousedown) {
    m_mousedown = false;
    return;
  }

  Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(
    ev.motion.x, ev.motion.y);

  // Calculate the menu item positions as passed in the draw method
  Vector menu_centre = m_menu->get_center_pos();
  const float menu_width = m_menu->get_width();
  const float menu_height = m_menu->get_height();
  Vector pos(
    menu_centre.x - menu_width / 2.0f,
    menu_centre.y
    + 24.0f * static_cast<float>(m_menu->get_active_item_id())
    - menu_height / 2.0f + 12.0f
  );

  // Calculate the relative horizontal position
  float x1 = pos.x + 16.0f;
  float x2 = pos.x + menu_width - 16.0f;
  float x = (mouse_pos.x - x1) / (x2 - x1);
  if (m_channel != ChannelType::CHANNEL_H) {
    x = math::clamp(x, 0.0f, 1.0f);
  } else {
    // The hue is periodic
    x = fmodf(x + 3.0f, 1.0f);
  }

  // Ignore distant mouse presses
  if (x < -0.5f || x > 1.5f || mouse_pos.y > pos.y + menu_height / 2.0f
      || mouse_pos.y < pos.y - menu_height / 2.0f)
    return;

  ColorOKLCh col_oklch = m_col_prev;
  ColorOKLCh col_oklch_clipped(0, 0, 0);
  if (m_channel == ChannelType::CHANNEL_L) {
    col_oklch.L = x;
    col_oklch_clipped = col_oklch;
  } else if (m_channel == ChannelType::CHANNEL_C) {
    float chroma_max_any_l = col_oklch.get_maximum_chroma_any_l();
    col_oklch.C = x * chroma_max_any_l;
    col_oklch_clipped = col_oklch;
    col_oklch_clipped.clip_lightness();
  } else {
    col_oklch.h = (2.0f * x - 1.0f) * math::PI;
    col_oklch_clipped = col_oklch;
    col_oklch_clipped.clip_adaptive_L0_L_cusp(HUE_COLORFULNESS);
  }
  set_color(col_oklch_clipped, col_oklch);
}

void
ItemColorChannelOKLab::set_color(ColorOKLCh& col_oklch_clipped,
  ColorOKLCh& col_oklch_store)
{
  // Save the current unclipped colour
  m_col_prev = col_oklch_store;
  // Convert the colour back to sRGB and clip if needed; preserve transparency
  float alpha = m_color->alpha;
  *m_color = col_oklch_clipped.to_srgb();
  m_color->alpha = alpha;
}

/* EOF */
