#include "supertux/counter.hpp"

#include "sprite/sprite_manager.hpp"
#include <algorithm>
#include "math/util.hpp"

static const CounterDef COUNTER_DEFS[COUNTER_COUNT] =
{
  /* COUNTER_STAR */
  {
    14.f,
    16,
    "images/powerups/star/star.sprite",
    "images/powerups/counter/counter.sprite",
    "images/particles/sparkle.sprite"
  }
};

const CounterDef& Counter::get_def(CounterType type)
{
  return COUNTER_DEFS[type];
}

Counter::Counter(CounterType type) :
  m_type(type),
  m_counter_on(false),
  m_total_time(get_def(type).total_time),
  m_remaining_time(m_total_time),
  m_num_parts(get_def(type).num_parts),
  m_counter_idx(0),
  m_effect_angle(-math::PI_2)
{
  
}

void
Counter::update_counter(bool counter, float remaining_time)
{
  m_counter_on = counter;
  m_remaining_time = remaining_time;

  if (!m_counter_on)
  {
    m_remaining_time = 0.f;
    return;
  }

  m_counter_idx = std::clamp(static_cast<int>(m_num_parts - m_remaining_time * m_num_parts / m_total_time),
                            0,
                            static_cast<int>(m_num_parts - 1));

  float step_deg = 360.f / static_cast<float>(m_num_parts);
  m_effect_angle = -math::PI_2 + math::radians(step_deg * static_cast<float>(m_counter_idx));
}

CounterType
Counter::get_type() const
{
  return m_type;
}

bool
Counter::is_counter_on() const
{
  return m_counter_on;
}

int
Counter::counter_stage() const
{
  return m_counter_idx;
}

float
Counter::effect_angle() const
{
  return m_effect_angle;
}
